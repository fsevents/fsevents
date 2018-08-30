/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "napi.h"
#include "uv.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include <iostream>
#include <vector>

#include "src/storage.cc"
namespace fse {
 class FSEvents : public Napi::ObjectWrap<FSEvents> {
  public:
    // constructor - exposed
    explicit FSEvents(const Napi::CallbackInfo& info);
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
    Napi::FunctionReference handler;
    Napi::Env _env;
    void emitEvent(const char *path, UInt32 flags, UInt64 id);

    // Common
    CFArrayRef paths;
    std::vector<fse_event*> events;
    static Napi::Object Init(Napi::Env env, Napi::Object exports);

    // methods.cc - exposed
    Napi::Value Stop(const Napi::CallbackInfo& info);
    Napi::Value Start(const Napi::CallbackInfo& info);

  private:
    static Napi::FunctionReference constructor;
  };
}

using namespace fse;

Napi::FunctionReference FSEvents::constructor;

Napi::Object FSEvents::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "FSEvents", {
    InstanceMethod("start", &FSEvents::Start),
    InstanceMethod("stop", &FSEvents::Stop)
  });

  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("FSEvents", func);

  return exports;
}

FSEvents::FSEvents(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<FSEvents>(info),
    _env(info.Env())  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  Napi::String path = info[0].ToString().As<Napi::String>();

  if (!info[1].IsFunction()) {
    Napi::TypeError::New(env, "Expected a function as handler").ThrowAsJavaScriptException();
    return;
  }
  handler = Persistent(info[1].As<Napi::Function>());

  CFStringRef dirs[] = { CFStringCreateWithCString(NULL, path.Utf8Value().c_str(), kCFStringEncodingUTF8) };
  paths = CFArrayCreate(NULL, (const void **)&dirs, 1, NULL);
  threadloop = NULL;
  if (uv_mutex_init(&mutex)) abort();
}
FSEvents::~FSEvents() {
  CFRelease(paths);
  uv_mutex_destroy(&mutex);
  handler.Unref();
}

#ifndef kFSEventStreamEventFlagItemCreated
#define kFSEventStreamEventFlagItemCreated 0x00000010
#endif

#include "src/async.cc"
#include "src/thread.cc"
#include "src/constants.cc"
#include "src/methods.cc"

Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
  exports = FSEvents::Init(env, exports);

  exports.Set(Napi::String::New(env, "Constants"), Constants(env));

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Initialize)
