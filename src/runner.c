/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
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

typedef struct fse_event {
  char path[PATH_MAX];
  size_t pathlen;
  UInt64 id;
  UInt32 flags;
  struct fse_event *next;
} fse_event_t;

void handleEvents(
  ConstFSEventStreamRef stream,
  void *data,
  size_t numEvents,
  void *eventPaths,
  const FSEventStreamEventFlags eventFlags[],
  const FSEventStreamEventId eventIds[]
) {
  size_t idx;
  fse_t *instance = (fse_t *)data;

  uv_mutex_lock(&instance->mutex);
  for (idx=0; idx < numEvents; idx++) {
    fse_event_t *event = (fse_event_t*)malloc(sizeof(fse_event_t));
    CFStringRef path = (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)eventPaths, idx);
    event->pathlen = CFStringGetLength(path);
    strncpy(event->path, CFStringGetCStringPtr(path, kCFStringEncodingUTF8), PATH_MAX);
    event->id = eventIds[idx];
    event->flags = eventFlags[idx];
    event->next = NULL;
    if (instance->head) {
      ((fse_event_t*)instance->head)->next = event;
      instance->head = event;
    } else {
      instance->head = event;
      instance->tail = event;
    }
  }
  uv_async_send(&instance->async);
  uv_mutex_unlock(&instance->mutex);
}

void eventCallback(uv_async_t *async) {
  fse_t *instance = (fse_t *) async->data;
  napi_handle_scope scope;
  napi_value this, callback, argv[3], result;
  fse_event_t *event;
  uv_mutex_lock(&instance->mutex);
  assert(napi_open_handle_scope(instance->env, &scope) == napi_ok);
  assert(napi_get_null(instance->env, &this) == napi_ok);
  assert(napi_get_reference_value(instance->env, instance->callback, &callback) == napi_ok);
  instance->head = NULL;
  while (instance->tail) {
    event = (fse_event_t*)instance->tail;
    instance->tail = event->next;
    assert(napi_create_string_utf8(instance->env, event->path, event->pathlen, &argv[0]) == napi_ok);
    assert(napi_create_uint32(instance->env, event->flags, &argv[1]) == napi_ok);
    assert(napi_create_int64(instance->env, event->id, &argv[2]) == napi_ok);
    napi_call_function(instance->env, this, callback, 3, argv, &result);
    free(event);
  }
  assert(napi_close_handle_scope(instance->env, scope) == napi_ok);
  uv_mutex_unlock(&instance->mutex);
}

void execute(void *data) {
  fse_t *instance = (fse_t*)data;

  instance->loop = CFRunLoopGetCurrent();

  FSEventStreamContext context = { 0, data, NULL, NULL, NULL };
  CFStringRef dirs[] = { CFStringCreateWithCString(NULL, instance->path, kCFStringEncodingUTF8) };
  FSEventStreamRef stream = FSEventStreamCreate(NULL, &handleEvents, &context, CFArrayCreate(NULL, (const void **)&dirs, 1, NULL), kFSEventStreamEventIdSinceNow, (CFAbsoluteTime) 0.1, kFSEventStreamCreateFlagNone | kFSEventStreamCreateFlagWatchRoot | kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagUseCFTypes);
  FSEventStreamScheduleWithRunLoop(stream, instance->loop, kCFRunLoopDefaultMode);
  FSEventStreamStart(stream);

  CFRunLoopRun();

  FSEventStreamStop(stream);
  FSEventStreamUnscheduleFromRunLoop(stream, instance->loop, kCFRunLoopDefaultMode);
  FSEventStreamInvalidate(stream);
  FSEventStreamRelease(stream);

  instance->loop = NULL;
}

void cleanup(napi_env env, void* data, void* ignored) {
  fse_t *instance = (fse_t*)data;
  stop(instance);
  assert(napi_delete_reference(instance->env, instance->callback) == napi_ok);
  free(instance);
}

void stop(fse_t *instance) {
  if (!instance->loop) return;
  unsigned int count = 0;
  CFRunLoopStop(instance->loop);
  uv_thread_join(&instance->thread);
  instance->async.data = NULL;
  uv_close((uv_handle_t *) &instance->async, NULL);
  uv_mutex_destroy(&instance->mutex);
  assert(napi_reference_unref(instance->env, instance->callback, &count) == napi_ok);
}

napi_value start(napi_env env, const char path[PATH_MAX], napi_value callback) {
  napi_value result;
  fse_t *instance = (fse_t*)malloc(sizeof(fse_t));
  memcpy(instance->path, path, PATH_MAX);

  instance->env = env;
  instance->head = NULL;
  instance->tail = NULL;

  assert(napi_create_reference(env, callback, 1, &instance->callback) == napi_ok);
  uv_async_init(uv_default_loop(), &instance->async, (uv_async_cb) eventCallback);
  uv_mutex_init(&instance->mutex);
  instance->async.data = instance;
  uv_thread_create(&instance->thread, execute, (void*)instance);

  assert(napi_create_external(env, (void*) instance, cleanup, NULL, &result) == napi_ok);
  return result;
}
