/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "fse_thread.h"

using namespace fse;

#ifndef kFSEventStreamCreateFlagFileEvents
#define kFSEventStreamCreateFlagFileEvents 0x00000010
#endif

struct fse_event {
  UInt64 id;
  UInt32 flags;
  CFStringRef path;
};
typedef  struct fse_event fse_event;

const void *FSEventRetain(CFAllocatorRef allocator, const void *ptr) {
  fse_event *orig = (fse_event *)ptr;
  fse_event *copy = (fse_event *)CFAllocatorAllocate(allocator, sizeof(fse_event), 0);
  copy->id = orig->id;
  copy->flags = orig->flags;
  copy->path = orig->path;
  CFRetain(copy->path);
  return copy;
}
void FSEventRelease(CFAllocatorRef allocator, const void *ptr) {
  fse_event *evt = (fse_event *)ptr;
  CFRelease(evt->path);
  CFAllocatorDeallocate(allocator, evt);
}
const CFArrayCallBacks FSEventArrayCallBacks = {0, FSEventRetain, FSEventRelease, 0, 0};

Thread::Thread(const char* path, void *ctx, notify_callback handler): ctx(ctx), handler(handler) {
  CFStringRef dirs[] = { CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8) };
  paths = CFArrayCreate(NULL, (const void **)&dirs, 1, NULL);
  
  events = CFArrayCreateMutable(NULL, 0,  &FSEventArrayCallBacks);
  pthread_mutex_init(&mutex, NULL);
  pthread_create(&thread, NULL, &Thread::thread_run, this);
}

void Thread::getevents(void *ctx, event_callback handler) {
  CFIndex idx;
  fse_event *event;
  char pathbuf[1024];
  const char *pathptr = NULL;
  pthread_mutex_lock(&mutex);
  for (idx=0; idx<CFArrayGetCount(events); idx++) {
    event = (fse_event *)CFArrayGetValueAtIndex(events, idx);
    pathptr = CFStringGetCStringPtr(event->path, kCFStringEncodingUTF8);
    if (!pathptr) CFStringGetCString(event->path, pathbuf, 1024, kCFStringEncodingUTF8);
    handler(ctx, pathptr ? pathptr : pathbuf, event->flags, event->id);
  }
  CFArrayRemoveAllValues(events);
  pthread_mutex_unlock(&mutex);
}

Thread::~Thread() {
  CFRunLoopStop(loop);
  pthread_join(thread, NULL);
  pthread_mutex_destroy(&mutex);
  CFRelease(paths);
  CFRelease(events);
  paths = NULL;
  events = NULL;
}

void *Thread::thread_run(void *ctx) {
  Thread *thread = (Thread*)ctx;
  FSEventStreamContext context = { 0, ctx, NULL, NULL, NULL };
  FSEventStreamRef stream = FSEventStreamCreate(NULL, &Thread::stream_evt, &context, thread->paths, kFSEventStreamEventIdSinceNow, (CFAbsoluteTime) 0.1, kFSEventStreamCreateFlagNone | kFSEventStreamCreateFlagWatchRoot | kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagUseCFTypes);
  thread->loop = CFRunLoopGetCurrent();
  FSEventStreamScheduleWithRunLoop(stream, thread->loop, kCFRunLoopDefaultMode);
  FSEventStreamStart(stream);
  CFRunLoopRun();
  FSEventStreamStop(stream);
  FSEventStreamUnscheduleFromRunLoop(stream, thread->loop, kCFRunLoopDefaultMode);
  FSEventStreamInvalidate(stream);
  FSEventStreamRelease(stream);
  return NULL;
}

void Thread::stream_evt(ConstFSEventStreamRef stream, void *ctx, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[], const FSEventStreamEventId eventIds[]) {
  Thread *thread = (Thread*)ctx;
  size_t idx;
  pthread_mutex_lock(&thread->mutex);
  fse_event event;
  for (idx=0; idx < numEvents; idx++) {
    event.path = (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)eventPaths, idx);
    event.flags = eventFlags[idx];
    event.id = eventIds[idx];
    CFArrayAppendValue(thread->events, &event);
  }
  pthread_mutex_unlock(&thread->mutex);
  thread->handler(thread->ctx);
}
