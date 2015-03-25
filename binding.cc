// Copyright (c) 2015, StrongLoop, Inc. <callback@strongloop.com>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <string>
#include <vector>

#include <pthread.h>  // pthread_setname_np()

#include "compat.h"
#include "compat-inl.h"
#include "node.h"
#include "node_version.h"
#include "uv.h"

#include <CoreFoundation/CFRunLoop.h>
#include <CoreServices/CoreServices.h>

#ifdef NDEBUG
#define CHECK(expr) do { if (!(expr)) abort(); } while (false)
#else
#define CHECK(expr) assert(expr)
#endif

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_NE(a, b) CHECK((a) != (b))

#if __cplusplus >= 201103L
#define DISALLOW_COPY_AND_ASSIGN(type) \
  type(type&&) = delete; \
  type(const type&) = delete; \
  void operator=(type&&) = delete; \
  void operator=(const type&) = delete
#else
#define DISALLOW_COPY_AND_ASSIGN(type) \
  type(const type&); \
  void operator=(const type&)
#endif

#define FSEVENTS_STREAM_CREATE_FLAGS_MAP(V) \
  V(kFSEventStreamCreateFlagNone) \
  V(kFSEventStreamCreateFlagUseCFTypes) \
  V(kFSEventStreamCreateFlagNoDefer) \
  V(kFSEventStreamCreateFlagWatchRoot) \
  V(kFSEventStreamCreateFlagIgnoreSelf) \
  V(kFSEventStreamCreateFlagFileEvents) \

#define FSEVENTS_STREAM_EVENT_FLAGS_MAP(V) \
  V(kFSEventStreamEventFlagNone) \
  V(kFSEventStreamEventFlagMustScanSubDirs) \
  V(kFSEventStreamEventFlagUserDropped) \
  V(kFSEventStreamEventFlagKernelDropped) \
  V(kFSEventStreamEventFlagEventIdsWrapped) \
  V(kFSEventStreamEventFlagHistoryDone) \
  V(kFSEventStreamEventFlagRootChanged) \
  V(kFSEventStreamEventFlagMount) \
  V(kFSEventStreamEventFlagUnmount) \
  V(kFSEventStreamEventFlagItemCreated) \
  V(kFSEventStreamEventFlagItemRemoved) \
  V(kFSEventStreamEventFlagItemInodeMetaMod) \
  V(kFSEventStreamEventFlagItemRenamed) \
  V(kFSEventStreamEventFlagItemModified) \
  V(kFSEventStreamEventFlagItemFinderInfoMod) \
  V(kFSEventStreamEventFlagItemChangeOwner) \
  V(kFSEventStreamEventFlagItemXattrMod) \
  V(kFSEventStreamEventFlagItemIsFile) \
  V(kFSEventStreamEventFlagItemIsDir) \
  V(kFSEventStreamEventFlagItemIsSymlink) \

namespace {

namespace C = compat;

template <typename T> void SILENCE_UNUSED_VARIABLE_WARNING(const T&) {}

struct Action {
  virtual ~Action() {}
  virtual void Execute() = 0;
};

struct Event {
  virtual ~Event() {}
  virtual void Execute() = 0;
};

typedef std::vector<Action*> ActionList;
typedef std::vector<Event*> EventList;

CFRunLoopRef run_loop_ref;
uv_thread_t watcher_thread;
uv_async_t async_handle;
uv_mutex_t action_mutex;
uv_mutex_t event_mutex;
ActionList actions;  // Main thread -> watcher thread, hold action_mutex.
EventList events;  // Watcher thread -> main thread, hold event_mutex.
uint32_t num_active;

class ScopedLock {
 public:
  explicit ScopedLock(uv_mutex_t* mutex) : mutex_(mutex) {
    uv_mutex_lock(mutex_);
  }

  ~ScopedLock() {
    uv_mutex_unlock(mutex_);
  }

 private:
  uv_mutex_t* mutex_;
  DISALLOW_COPY_AND_ASSIGN(ScopedLock);
};

struct Context {
  Context(v8::Isolate* isolate, v8::Local<v8::Function> callback)
      : isolate_(isolate), stream_ref_(NULL) {
    callback_.Reset(isolate, callback);
  }

  v8::Isolate* const isolate_;
  C::Persistent<v8::Function> callback_;
  FSEventStreamRef stream_ref_;
};

struct FileEvent : public Event {
  std::vector<std::string> event_paths_;
  std::vector<FSEventStreamEventFlags> event_flags_;
  std::vector<FSEventStreamEventId> event_ids_;
  Context* const context_;

  FileEvent(size_t num_events, const char** event_paths,
            const FSEventStreamEventFlags* event_flags,
            const FSEventStreamEventId* event_ids,
            Context* context) : context_(context) {
    event_paths_.reserve(num_events);
    event_flags_.reserve(num_events);
    event_ids_.reserve(num_events);
    for (size_t i = 0; i < num_events; i += 1) {
      if (event_paths[i][0] == '\0') continue;
      event_paths_.push_back(event_paths[i]);
      event_flags_.push_back(event_flags[i]);
      event_ids_.push_back(event_ids[i]);
    }
  }

  virtual void Execute() {
    v8::Isolate* isolate = context_->isolate_;
    C::HandleScope handle_scope(isolate);
    if (context_->callback_.IsEmpty()) return;  // Stale event.
    const size_t size = event_paths_.size();
    v8::Local<v8::Array> array = C::Array::New(isolate, 3 * size);
    for (size_t i = 0; i < size; i += 1) {
      const std::string& event_path = event_paths_[i];
      v8::Local<v8::String> path =
          C::String::NewFromUtf8(isolate, event_path.data(),
                                 C::String::kNormalString, event_path.size());
      v8::Local<v8::Number> flags = C::Number::New(isolate, event_flags_[i]);
      v8::Local<v8::Number> ident = C::Number::New(isolate, event_ids_[i]);
      array->Set(0 + 3 * i, path);
      array->Set(1 + 3 * i, flags);
      array->Set(2 + 3 * i, ident);
    }
    v8::Local<v8::Function> callback = context_->callback_.ToLocal(isolate);
    v8::Local<v8::Value> array_v = array;
#if NODE_MODULE_VERSION <= 11
    // node.js v0.10
    node::MakeCallback(callback, callback, 1, &array_v);
#else
    // node.js v0.12 and io.js
    node::MakeCallback(isolate, callback, callback, 1, &array_v);
#endif
  }
};

void OnFSEvent(ConstFSEventStreamRef fs_event_stream_ref, void *info,
               size_t num_events, void *event_paths_v,
               const FSEventStreamEventFlags *event_flags,
               const FSEventStreamEventId *event_ids) {
  Context* context =  static_cast<Context*>(info);
  const char** event_paths = static_cast<const char**>(event_paths_v);
  FileEvent* event =
      new FileEvent(num_events, event_paths, event_flags, event_ids, context);
  ScopedLock scoped_lock(&event_mutex);
  events.push_back(event);
  uv_async_send(&async_handle);
}

struct StartAction : public Action {
  const std::string path_;
  const uint32_t flags_;
  const uint32_t latency_;
  Context* const context_;

  StartAction(const char* path, uint32_t flags, uint32_t latency,
              Context* context)
      : path_(path), flags_(flags), latency_(latency), context_(context) {}

  virtual void Execute() {
    CFAllocatorRef allocator = kCFAllocatorDefault;
    const int num_paths_to_watch = 1;
    CFMutableArrayRef paths_to_watch =
        CFArrayCreateMutable(allocator, num_paths_to_watch, NULL);
    const uint8_t* data = reinterpret_cast<const uint8_t*>(path_.data());
    CFStringRef path_to_watch =
        CFStringCreateWithBytes(allocator, data, path_.size(),
                                kCFStringEncodingUTF8, TRUE);
    CFArrayAppendValue(paths_to_watch, path_to_watch);
    FSEventStreamContext stream_context = { 0, context_, NULL, NULL, NULL };
    FSEventStreamRef stream_ref =
        FSEventStreamCreate(allocator, OnFSEvent, &stream_context,
                            paths_to_watch, kFSEventStreamEventIdSinceNow,
                            latency_, flags_);
    FSEventStreamScheduleWithRunLoop(stream_ref, run_loop_ref,
                                     kCFRunLoopDefaultMode);
    FSEventStreamStart(stream_ref);
    // Should match mutex in StopAction::Execute().
    ScopedLock scoped_lock(&event_mutex);
    context_->stream_ref_ = stream_ref;
  }
};

struct StopEvent : public Event {
  Context* const context_;

  StopEvent(Context* context) : context_(context) {}

  virtual void Execute() {
    CHECK_NE(num_active, 0);

    num_active -= 1;
    if (num_active == 0)
      uv_unref(reinterpret_cast<uv_handle_t*>(&async_handle));

    delete context_;
  }
};

struct StopAction : public Action {
  Context* const context_;

  StopAction(Context* context) : context_(context) {}

  virtual void Execute() {
    FSEventStreamRef stream_ref;
    {
      // Should match mutex in StartAction::Execute().
      ScopedLock scoped_lock(&event_mutex);
      stream_ref = context_->stream_ref_;
      context_->stream_ref_ = NULL;
    }

    CHECK_NE(stream_ref, NULL);
    CHECK_NE(run_loop_ref, NULL);
    FSEventStreamUnscheduleFromRunLoop(stream_ref, run_loop_ref,
                                       kCFRunLoopDefaultMode);

    StopEvent* event = new StopEvent(context_);
    ScopedLock scoped_lock(&event_mutex);
    events.push_back(event);
    uv_async_send(&async_handle);
  }
};

void OnAsyncEvent(uv_async_t*) {
  EventList local_events;
  {
    ScopedLock scoped_lock(&event_mutex);
    local_events.swap(events);
  }
  for (EventList::iterator it = local_events.begin(),
       end = local_events.end(); it != end; ++it) {
    (*it)->Execute();
    delete *it;
  }
}

void ThreadMain(void*) {
  pthread_setname_np("fsevents watcher thread");
  {
    // Signal the main thread that we're live.
    ScopedLock scoped_lock(&action_mutex);
    run_loop_ref = CFRunLoopGetCurrent();
  }
  for (;;) {
    ActionList local_actions;
    {
      ScopedLock scoped_lock(&action_mutex);
      if (run_loop_ref == NULL) return;  // Signal from main thread.
      local_actions.swap(actions);
    }
    for (ActionList::iterator it = local_actions.begin(),
         end = local_actions.end(); it != end; ++it) {
      (*it)->Execute();
      delete *it;
    }
    CFRunLoopRun();
  }
}

C::ReturnType Start(const C::ArgumentType& args) {
  v8::Isolate* isolate = args.GetIsolate();
  C::ReturnableHandleScope handle_scope(args);
  CHECK(args[0]->IsString());
  CHECK(args[1]->IsUint32());
  CHECK(args[2]->IsUint32());
  CHECK(args[3]->IsFunction());
  v8::String::Utf8Value path(args[0]);
  const uint32_t flags = args[1]->Uint32Value();
  const uint32_t latency = args[2]->Uint32Value();
  v8::Local<v8::Function> callback = args[3].As<v8::Function>();
  Context* context = new Context(isolate, callback);
  StartAction* action = new StartAction(*path, flags, latency, context);
  if (run_loop_ref == NULL) {
    actions.push_back(action);
    uv_loop_t* const event_loop = uv_default_loop();
    CHECK_EQ(0, uv_async_init(event_loop, &async_handle,
                              reinterpret_cast<uv_async_cb>(OnAsyncEvent)));
    CHECK_EQ(0, uv_mutex_init(&action_mutex));
    CHECK_EQ(0, uv_mutex_init(&event_mutex));
    CHECK_EQ(0, uv_thread_create(&watcher_thread, ThreadMain, NULL));
    // OS X doesn't have pthread barriers so we busy loop instead until we see
    // that the CF event loop is active.  Unelegant but ultimately not that big
    // a deal because the new thread starts up in microseconds.
    for (;;) {
      ScopedLock scoped_lock(&action_mutex);
      if (run_loop_ref != NULL) break;
    }
  } else {
    ScopedLock scoped_lock(&action_mutex);
    actions.push_back(action);
    CFRunLoopStop(run_loop_ref);
  }

  num_active += 1;
  if (num_active == 1)
    uv_ref(reinterpret_cast<uv_handle_t*>(&async_handle));

#if NODE_MODULE_VERSION <= 11
  v8::Local<v8::External> data = v8::External::New(context);
#else
  v8::Local<v8::External> data = v8::External::New(isolate, context);
#endif
  return handle_scope.Return(data);
}

C::ReturnType Stop(const C::ArgumentType& args) {
  C::ReturnableHandleScope handle_scope(args);
  CHECK(args[0]->IsExternal());
  Context* context =
      static_cast<Context*>(args[0].As<v8::External>()->Value());
  context->callback_.Reset();
  StopAction* action = new StopAction(context);
  ScopedLock scoped_lock(&action_mutex);
  actions.push_back(action);
  CFRunLoopStop(run_loop_ref);
  return handle_scope.Return();
}

void Initialize(v8::Local<v8::Object> binding) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  binding->Set(C::String::NewFromUtf8(isolate, "start"),
               C::FunctionTemplate::New(isolate, Start)->GetFunction());
  binding->Set(C::String::NewFromUtf8(isolate, "stop"),
               C::FunctionTemplate::New(isolate, Stop)->GetFunction());
#define V(name) \
  binding->Set(C::String::NewFromUtf8(isolate, #name), \
               C::Number::New(isolate, name));
  FSEVENTS_STREAM_CREATE_FLAGS_MAP(V)
  FSEVENTS_STREAM_EVENT_FLAGS_MAP(V)
#undef V
}

}  // namespace anonymous

NODE_MODULE(binding, Initialize)
