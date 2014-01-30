/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "fse_native.h"

using namespace fse;
using namespace v8;

Native::Native(const char *path, NanCallback *handler): handler(handler) {
  async = new Async(this, &Native::async_callback);
  thread = new Thread(path, this, &Native::thread_callback);
}

void Native::wrap(v8::Handle<v8::Object> obj) {
  Wrap(obj);
}

void Native::stop() {
  if (thread) {
    delete thread;
    thread = NULL;
  }

  if (handler) {
    delete handler;
    handler = NULL;  
  }

  if (async) {
    async->destroy();
    async = NULL;
  }
}

Native::~Native() {
  stop();
}

void Native::thread_callback(void *data) {
  Native *native = (Native*)data;
  if (!native->async) return;
  native->async->trigger();
}

void Native::async_callback(void *data) {
  Native *native = (Native*)data;
  if (!native->thread) return;
  native->thread->getevents(native, &Native::event_callback);
}

void Native::event_callback(void *ctx, const char *path, UInt32 flags, UInt64 id) {
  Native *native = (Native*)ctx;
  native->send_event(path, flags, id);
}

void Native::send_event(const char* path, UInt64 flags, UInt32 id) {
  if (!handler) return;
  NanScope();
  Handle<Value> argv[] = {
    String::New(path),
    Number::New(flags),
    Number::New(id)
  };
  handler->Call(3, argv);
}
