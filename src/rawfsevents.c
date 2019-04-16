#include "rawfsevents.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"
#include <pthread.h>
#include <assert.h>

#ifndef CHECK
#ifdef NDEBUG
#define CHECK(x) do { if (!(x)) abort(); } while (0)
#else
#define CHECK assert
#endif
#endif

typedef struct {
  pthread_t thread;
  CFRunLoopRef loop;
  pthread_mutex_t lock;
  pthread_cond_t init;
} fse_loop_t;

struct fse_watcher_s {
  char path[PATH_MAX];
  FSEventStreamRef stream;
  fse_event_handler_t handler;
  fse_thread_hook_t hookend;
  void *context;
};

static fse_loop_t fsevents;

void fse_init() {
  fsevents.thread = NULL;
  fsevents.loop = NULL;
  pthread_mutex_init(&fsevents.lock, NULL);
  pthread_cond_init(&fsevents.init, NULL);
}

void *fse_run_loop(void *data) {

  fsevents.loop = CFRunLoopGetCurrent();
  pthread_mutex_unlock(&fsevents.lock);

  pthread_cond_signal(&fsevents.init);
  pthread_mutex_lock(&fsevents.lock);
  CFRunLoopPerformBlock(fsevents.loop, kCFRunLoopDefaultMode, ^(void){
    pthread_mutex_unlock(&fsevents.lock);
  });
  CFRunLoopRun();

  pthread_mutex_lock(&fsevents.lock);
  fsevents.loop = NULL;
  pthread_mutex_unlock(&fsevents.lock);

  return NULL;
}

void fse_handle_events(
  ConstFSEventStreamRef stream,
  void *data,
  size_t numEvents,
  void *eventPaths,
  const FSEventStreamEventFlags eventFlags[],
  const FSEventStreamEventId eventIds[]
) {
  fse_watcher_t watcher = data;
  if (!watcher->handler) return;
  fse_event_t *events = malloc(sizeof(*events) * numEvents);
  CHECK(events);
  size_t idx;
  for (idx=0; idx < numEvents; idx++) {
    fse_event_t *event = &events[idx];
    CFStringRef path = (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)eventPaths, idx);
    strncpy(event->path, CFStringGetCStringPtr(path, kCFStringEncodingUTF8), sizeof(event->path));
    event->id = eventIds[idx];
    event->flags = eventFlags[idx];
  }
  if (!watcher->handler) {
    free(events);
  } else {
    watcher->handler(watcher->context, numEvents, events);
  }
}

void fse_clear(fse_watcher_t watcher) {
  watcher->path[0] = 0;
  watcher->handler = NULL;
  watcher->stream = NULL;
  watcher->context = NULL;
  watcher->hookend = NULL;
}

fse_watcher_t fse_alloc() {
  fse_watcher_t watcher = malloc(sizeof(*watcher));
  CHECK(watcher);
  fse_clear(watcher);
  return watcher;
}

void fse_free(fse_watcher_t watcher) {
  fse_unwatch(watcher);
  free(watcher);
}

void fse_watch(const char *path, fse_event_handler_t handler, void *context, fse_thread_hook_t hookstart, fse_thread_hook_t hookend, fse_watcher_t watcher) {
  pthread_mutex_lock(&fsevents.lock);
  if (!fsevents.loop) {
    pthread_create(&fsevents.thread, NULL, fse_run_loop, NULL);
    pthread_mutex_unlock(&fsevents.lock);
    pthread_cond_wait(&fsevents.init, &fsevents.lock);
  }

  strncpy(watcher->path, path, PATH_MAX);
  watcher->handler = handler;
  watcher->context = context;
  CFRunLoopPerformBlock(fsevents.loop, kCFRunLoopDefaultMode, ^(void){
    if (hookstart) hookstart(watcher->context);
    FSEventStreamContext streamcontext = { 0, watcher, NULL, NULL, NULL };
    CFStringRef dirs[] = { CFStringCreateWithCString(NULL, watcher->path, kCFStringEncodingUTF8) };
    watcher->stream = FSEventStreamCreate(NULL, &fse_handle_events, &streamcontext, CFArrayCreate(NULL, (const void **)&dirs, 1, NULL), kFSEventStreamEventIdSinceNow, (CFAbsoluteTime) 0.1, kFSEventStreamCreateFlagNone | kFSEventStreamCreateFlagWatchRoot | kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagUseCFTypes);
    FSEventStreamScheduleWithRunLoop(watcher->stream, fsevents.loop, kCFRunLoopDefaultMode);
    FSEventStreamStart(watcher->stream);
  });
  CFRunLoopWakeUp(fsevents.loop);
  pthread_mutex_unlock(&fsevents.lock);
}

void fse_unwatch(fse_watcher_t watcher) {
  FSEventStreamRef stream = watcher->stream;
  fse_thread_hook_t hookend = watcher->hookend;
  void *context = watcher->context;
  fse_clear(watcher);

  pthread_mutex_lock(&fsevents.lock);
  if (fsevents.loop) {
    CFRunLoopPerformBlock(fsevents.loop, kCFRunLoopDefaultMode, ^(void){
      if (stream) {
        FSEventStreamStop(stream);
        FSEventStreamUnscheduleFromRunLoop(stream, fsevents.loop, kCFRunLoopDefaultMode);
        FSEventStreamInvalidate(stream);
        FSEventStreamRelease(stream);
      }
      if (hookend) hookend(context);
    });
  }
  pthread_mutex_unlock(&fsevents.lock);
}

void *fse_context_of(fse_watcher_t watcher) {
  return watcher->context;
}
