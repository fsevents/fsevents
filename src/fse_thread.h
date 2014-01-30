/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#ifndef fse_thread_h
#define fse_thread_h

#include "fse_types.h"
#include <pthread.h>
#include <CoreServices/CoreServices.h>

namespace fse {
  class Thread {
    public:
      Thread(const char *path, void *ctx, notify_callback handler);
      void getevents(void *ctx, event_callback handler);
      ~Thread();
    protected:
      CFArrayRef paths;
      CFRunLoopRef loop;
      CFMutableArrayRef events;
      void *ctx;
      notify_callback handler;
      pthread_t thread;
      pthread_mutex_t mutex;

      static void *thread_run(void *ctx);
      static void stream_evt(ConstFSEventStreamRef stream, void *ctx, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]);
  };  
}

#endif // fse_thread_h
