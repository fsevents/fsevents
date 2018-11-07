/*
** © 2018 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "runner.h"
#include <pthread.h>
#include <limits.h>
#include <stdlib.h>
#include <strings.h>
#include "CoreFoundation/CoreFoundation.h"
#define NAPI_EXPERIMENTAL
#include <node_api.h>

void notify(
  ConstFSEventStreamRef stream,
  void *data,
  size_t numEvents,
  void *eventPaths,
  const FSEventStreamEventFlags eventFlags[],
  const FSEventStreamEventId eventIds[]
) {
  napi_status status;
  size_t idx;
  fse_t *instance = (fse_t *)data;

  for (idx=0; idx < numEvents; idx++) {
    fse_event_t *event = (fse_event_t*)malloc(sizeof(fse_event_t));
    CFStringRef path = (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)eventPaths, idx);
    event->pathlen = CFStringGetLength(path);
    strncpy(event->path, CFStringGetCStringPtr(path, kCFStringEncodingUTF8), PATH_MAX);
    event->id = eventIds[idx];
    event->flags = eventFlags[idx];
    status = napi_call_threadsafe_function(instance->callback, event, napi_tsfn_nonblocking);
    // ignore status.
  }
}

void *execute(void *data) {
  fse_t *instance = (fse_t*)data;

  instance->loop = CFRunLoopGetCurrent();

  FSEventStreamContext context = { 0, data, NULL, NULL, NULL };
  CFStringRef dirs[] = { CFStringCreateWithCString(NULL, instance->path, kCFStringEncodingUTF8) };
  FSEventStreamRef stream = FSEventStreamCreate(NULL, &notify, &context, CFArrayCreate(NULL, (const void **)&dirs, 1, NULL), kFSEventStreamEventIdSinceNow, (CFAbsoluteTime) 0.1, kFSEventStreamCreateFlagNone | kFSEventStreamCreateFlagWatchRoot | kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagUseCFTypes);
  FSEventStreamScheduleWithRunLoop(stream, instance->loop, kCFRunLoopDefaultMode);
  FSEventStreamStart(stream);

  CFRunLoopRun();

  FSEventStreamStop(stream);
  FSEventStreamUnscheduleFromRunLoop(stream, instance->loop, kCFRunLoopDefaultMode);
  FSEventStreamInvalidate(stream);
  FSEventStreamRelease(stream);

  instance->loop = NULL;
  return instance;
}

void stop(fse_t *instance) {
  if (!instance->loop) return;
  CFRunLoopStop(instance->loop);
  pthread_join(instance->thread, NULL);
  napi_release_threadsafe_function(instance->callback, napi_tsfn_release);
}

fse_t *start(const char path[PATH_MAX], napi_threadsafe_function callback) {
  pthread_attr_t attr;
  fse_t *instance = (fse_t*)malloc(sizeof(fse_t));
  memcpy(instance->path, path, PATH_MAX);
  instance->callback = callback;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&instance->thread, &attr, execute, (void *)instance);

  return instance;
}
