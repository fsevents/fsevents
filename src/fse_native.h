/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#ifndef fse_native_h
#define fse_native_h

#include "nan.h"
#include "fse_async.h"
#include "fse_thread.h"

namespace fse {
  class Native : public node::ObjectWrap {
    public:
      Native(const char *path, NanCallback *handler);
      void wrap(v8::Handle<v8::Object> obj);
      void stop();
      ~Native();
    protected:
      static void thread_callback(void *data);
      static void async_callback(void *data);
      static void event_callback(void *ctx, const char *path, UInt32 flags, UInt64 id);
      void send_event(const char* path, UInt64 flags, UInt32 id);
      
      Async *async;
      Thread *thread;
      NanCallback *handler;
  };
}

#endif // fse_native_h
