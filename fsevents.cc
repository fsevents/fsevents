/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "napi.h"
#include "uv.h"
#include "uv.h"
#include "v8.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include <iostream>
#include <vector>

#include "src/storage.cc"
namespace fse {
  class FSEvents : public Napi::ObjectWrap<FSEvents> {
  public:
    explicit FSEvents(const char *path);
    ~FSEvents();

    uv_mutex_t mutex;

    // async.cc
    uv_async_t async;
    void asyncStart();
    void asyncTrigger();
    void asyncStop();

    // thread.cc
    uv_thread_t thread;
    CFRunLoopRef threadloop;
    void threadStart();
    static void threadRun(void *ctx);
    void threadStop();

    // methods.cc - internal
    Napi::AsyncResource async_resource;
    void emitEvent(const char *path, UInt32 flags, UInt64 id);

    // Common
    CFArrayRef paths;
    std::vector<fse_event*> events;
    static void Initialize(v8::Handle<v8::Object> exports);

    // methods.cc - exposed
    static Napi::Value New(const Napi::CallbackInfo& info);
    static Napi::Value Stop(const Napi::CallbackInfo& info);
    static Napi::Value Start(const Napi::CallbackInfo& info);

  };
}

using namespace fse;

FSEvents::FSEvents(const char *path)
   : async_resource("fsevents:FSEvents") {
  CFStringRef dirs[] = { CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8) };
  paths = CFArrayCreate(NULL, (const void **)&dirs, 1, NULL);
  threadloop = NULL;
  if (uv_mutex_init(&mutex)) abort();
}
FSEvents::~FSEvents() {
  CFRelease(paths);
  uv_mutex_destroy(&mutex);
}

#ifndef kFSEventStreamEventFlagItemCreated
#define kFSEventStreamEventFlagItemCreated 0x00000010
#endif

#include "src/async.cc"
#include "src/thread.cc"
#include "src/constants.cc"
#include "src/methods.cc"

void FSEvents::Initialize(v8::Handle<v8::Object> exports) {
  Napi::FunctionReference tpl = Napi::Function::New(env, FSEvents::New);
  tpl->SetClassName(Napi::String::New(env, "FSEvents"));

  tpl->PrototypeTemplate().Set(
           Napi::String::New(env, "start"),
           Napi::Function::New(env, FSEvents::Start));
  tpl->PrototypeTemplate().Set(
           Napi::String::New(env, "stop"),
           Napi::Function::New(env, FSEvents::Stop));
  exports.Set(Napi::String::New(env, "Constants"), Constants());
  exports.Set(Napi::String::New(env, "FSEvents"),
               tpl->GetFunction());
}

NODE_API_MODULE(fse, FSEvents::Initialize)
