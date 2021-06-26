/*
** © 2020 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

#define NAPI_VERSION 4
#include <node_api.h>

#ifndef CHECK
#ifdef NDEBUG
#define CHECK(x) \
  do             \
  {              \
    if (!(x))    \
      abort();   \
  } while (0)
#else
#include <assert.h>
#define CHECK assert
#endif
#endif

#include "constants.h"
#define CONSTANT(name)                                                               \
  do                                                                                 \
  {                                                                                  \
    CHECK(napi_create_int32(env, kFSEventStreamEventFlag##name, &value) == napi_ok); \
    CHECK(napi_set_named_property(env, constants, #name, value) == napi_ok);         \
  } while (0)

#include <pthread.h>
#include <stdlib.h>
#include <limits.h>

//#include <stdio.h>
//#define DEBUG(str, idx) printf("%s(%i)\n", str, idx);

#ifdef AVAILABLE_MAC_OS_X_VERSION_10_13_AND_LATER
  bool fse_extended_data_supported = true;
#else
  bool fse_extended_data_supported = false;
#endif

void RunLoopSourceScheduleRoutine(void *info, CFRunLoopRef loop, CFStringRef mode) {}
void RunLoopSourcePerformRoutine(void *info) {}
void RunLoopSourceCancelRoutine(void *info, CFRunLoopRef loop, CFStringRef mode) {}

typedef struct
{
  pthread_t thread;
  pthread_mutex_t lock;
  CFRunLoopRef loop;
} fse_environment_t;
void fse_environment_destroy(napi_env env, void *voidenv, void *hint)
{
  fse_environment_t *fseenv = voidenv;
  CFRunLoopStop(fseenv->loop);
  pthread_join(fseenv->thread, NULL);
  pthread_mutex_destroy(&fseenv->lock);
  fseenv->thread = NULL;
  fseenv->loop = NULL;
  free(fseenv);
}
void *fse_run_loop(void *voidenv)
{
  fse_environment_t *fseenv = voidenv;
  fseenv->loop = CFRunLoopGetCurrent();
  CFRunLoopSourceContext context = {0, NULL, NULL, NULL, NULL, NULL, NULL, &RunLoopSourceScheduleRoutine, RunLoopSourceCancelRoutine, RunLoopSourcePerformRoutine};
  CFRunLoopSourceRef source = CFRunLoopSourceCreate(NULL, 0, &context);
  CFRunLoopAddSource(fseenv->loop, source, kCFRunLoopDefaultMode);
  pthread_mutex_unlock(&fseenv->lock);
  CFRunLoopRun();
  pthread_mutex_lock(&fseenv->lock);
  fseenv->loop = NULL;
  pthread_mutex_unlock(&fseenv->lock);
  return NULL;
}
napi_value fse_environment_create(napi_env env)
{
  fse_environment_t *fseenv = malloc(sizeof(fse_environment_t));
  fseenv->loop = NULL;

  pthread_mutex_init(&fseenv->lock, NULL);
  pthread_mutex_lock(&fseenv->lock);

  fseenv->thread = NULL;
  pthread_create(&fseenv->thread, NULL, fse_run_loop, (void *)fseenv);

  pthread_mutex_lock(&fseenv->lock);
  pthread_mutex_unlock(&fseenv->lock);

  napi_value result;
  CHECK(napi_create_external(env, fseenv, fse_environment_destroy, NULL, &result) == napi_ok);
  return result;
}

typedef struct
{
  char path[PATH_MAX];
  fse_environment_t *fseenv;
  FSEventStreamRef stream;
  napi_threadsafe_function callback;
} fse_instance_t;
typedef struct
{
  unsigned long long id;
  char path[PATH_MAX];
  unsigned int flags;
  uint64_t inode;
} fse_event_t;
typedef struct
{
  int length;
  fse_event_t *events;
} fse_events_t;
void fse_instance_destroy(napi_env env, void *voidinst, void *hint)
{
  fse_instance_t *instance = voidinst;

  if (instance->stream)
  {
    FSEventStreamStop(instance->stream);
    FSEventStreamUnscheduleFromRunLoop(instance->stream, instance->fseenv->loop, kCFRunLoopDefaultMode);
    FSEventStreamInvalidate(instance->stream);
    FSEventStreamRelease(instance->stream);
    instance->stream = NULL;
  }

  if (instance->callback)
  {
    CHECK(napi_unref_threadsafe_function(env, instance->callback) == napi_ok);
    CHECK(napi_release_threadsafe_function(instance->callback, napi_tsfn_abort) == napi_ok);
    instance->callback = NULL;
  }

  if (instance != hint)
  {
    free(instance);
  }
}
void fse_dispatch_event(napi_env env, napi_value callback, void *context, void *data)
{
  if (!env)
  {
    return;
  }

  fse_events_t *events = data;
  int argc = 3, idx = 0;
  if (fse_extended_data_supported)
  {
    argc = 4;
  }
  napi_value args[argc];
  for (idx = 0; idx < events->length; idx++)
  {
    CHECK(napi_create_string_utf8(env, events->events[idx].path, NAPI_AUTO_LENGTH, &args[0]) == napi_ok);
    CHECK(napi_create_uint32(env, events->events[idx].flags, &args[1]) == napi_ok);
    CHECK(napi_create_int64(env, events->events[idx].id, &args[2]) == napi_ok);
    if (fse_extended_data_supported)
    {
      CHECK(napi_create_int64(env, events->events[idx].inode, &args[3]) == napi_ok);
    }
    napi_value recv;
    CHECK(napi_get_null(env, &recv) == napi_ok);
    CHECK(napi_call_function(env, recv, callback, argc, args, &recv) == napi_ok);
  }
  free(events->events);
  free(events);
}
void fse_handle_events(
    ConstFSEventStreamRef stream,
    void *data,
    size_t numEvents,
    void *eventPaths,
    const FSEventStreamEventFlags eventFlags[],
    const FSEventStreamEventId eventIds[])
{
  fse_instance_t *instance = data;
  if (!instance->callback)
  {
    return;
  }

  fse_events_t *events = malloc(sizeof(fse_events_t));
  events->length = numEvents;
  events->events = malloc(numEvents * sizeof(fse_event_t));
  size_t idx;
  for (idx = 0; idx < numEvents; idx++)
  {
    CFDictionaryRef path_info_dict;
    CFStringRef path;
    CFNumberRef cf_inode;
    if (fse_extended_data_supported)
    {
      path_info_dict = CFArrayGetValueAtIndex((CFArrayRef)eventPaths, idx);
      path = CFDictionaryGetValue(path_info_dict, kFSEventStreamEventExtendedDataPathKey);
      cf_inode = CFDictionaryGetValue(path_info_dict, kFSEventStreamEventExtendedFileIDKey);
    }
    else
    {
      path = (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)eventPaths, idx);
    }
    if (!CFStringGetCString(path, events->events[idx].path, PATH_MAX, kCFStringEncodingUTF8))
    {
      events->events[idx].path[0] = 0;
    }
    events->events[idx].id = eventIds[idx];
    events->events[idx].flags = eventFlags[idx];
    if (fse_extended_data_supported)
    {
      CFNumberGetValue(cf_inode, kCFNumberSInt64Type, &(events->events[idx].inode));
    }
  }
  CHECK(napi_call_threadsafe_function(instance->callback, events, napi_tsfn_blocking) == napi_ok);
}
napi_value FSEStart(napi_env env, napi_callback_info info)
{
  size_t argc = 4;
  napi_value argv[argc];
  CHECK(napi_get_cb_info(env, info, &argc, argv, NULL, NULL) == napi_ok);

  fse_instance_t *instance = malloc(sizeof(fse_instance_t));

  size_t pathlen = PATH_MAX;
  char path[PATH_MAX];
  int64_t since;

  CHECK(napi_get_value_external(env, argv[0], (void **)&instance->fseenv) == napi_ok);
  CHECK(napi_get_value_string_utf8(env, argv[1], path, pathlen, &pathlen) == napi_ok);
  CHECK(napi_get_value_int64(env, argv[2], &since) == napi_ok);
  instance->stream = NULL;
  strncpy(instance->path, path, PATH_MAX);
  napi_value asyncResource, asyncName;

  CHECK(napi_create_object(env, &asyncResource) == napi_ok);
  CHECK(napi_create_string_utf8(env, "fsevents", NAPI_AUTO_LENGTH, &asyncName) == napi_ok);
  CHECK(napi_create_threadsafe_function(env, argv[3], asyncResource, asyncName, 0, 2, NULL, NULL, NULL, fse_dispatch_event, &instance->callback) == napi_ok);
  CHECK(napi_ref_threadsafe_function(env, instance->callback) == napi_ok);

  FSEventStreamContext streamcontext = {0, instance, NULL, NULL, NULL};
  CFStringRef dirs[] = {CFStringCreateWithCString(NULL, instance->path, kCFStringEncodingUTF8)};
  uint32_t event_stream_create_flags = kFSEventStreamCreateFlagNone | kFSEventStreamCreateFlagWatchRoot | kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagUseCFTypes;
  if (fse_extended_data_supported)
  {
    event_stream_create_flags |= kFSEventStreamCreateFlagUseExtendedData;
  }
  instance->stream = FSEventStreamCreate(NULL, &fse_handle_events, &streamcontext, CFArrayCreate(NULL, (const void **)&dirs, 1, NULL), since, (CFAbsoluteTime)0.1, event_stream_create_flags);
  FSEventStreamScheduleWithRunLoop(instance->stream, instance->fseenv->loop, kCFRunLoopDefaultMode);
  FSEventStreamStart(instance->stream);

  napi_value result;
  CHECK(napi_create_external(env, instance, fse_instance_destroy, NULL, &result) == napi_ok);
  return result;
}

napi_value FSEStop(napi_env env, napi_callback_info info)
{
  size_t argc = 1;
  napi_value argv[argc];
  CHECK(napi_get_cb_info(env, info, &argc, argv, NULL, NULL) == napi_ok);
  fse_instance_t *instance = NULL;
  CHECK(napi_get_value_external(env, argv[0], (void **)&instance) == napi_ok);

  fse_instance_destroy(env, instance, instance);

  CHECK(napi_get_undefined(env, &argv[0]) == napi_ok);
  return argv[0];
}

napi_value Init_Constants(napi_env env)
{
  napi_value constants, value;
  CHECK(napi_create_object(env, &constants) == napi_ok);
  CONSTANT(None);
  CONSTANT(MustScanSubDirs);
  CONSTANT(UserDropped);
  CONSTANT(KernelDropped);
  CONSTANT(EventIdsWrapped);
  CONSTANT(HistoryDone);
  CONSTANT(RootChanged);
  CONSTANT(Mount);
  CONSTANT(Unmount);
  CONSTANT(ItemCreated);
  CONSTANT(ItemRemoved);
  CONSTANT(ItemInodeMetaMod);
  CONSTANT(ItemRenamed);
  CONSTANT(ItemModified);
  CONSTANT(ItemFinderInfoMod);
  CONSTANT(ItemChangeOwner);
  CONSTANT(ItemXattrMod);
  CONSTANT(ItemIsFile);
  CONSTANT(ItemIsDir);
  CONSTANT(ItemIsSymlink);
  CONSTANT(ItemIsHardlink);
  CONSTANT(ItemIsLastHardlink);
  CONSTANT(OwnEvent);
  CONSTANT(ItemCloned);
  return constants;
}
napi_value Init_Flags(napi_env env)
{
  napi_value flags, value;
  CHECK(napi_create_object(env, &flags) == napi_ok);
  CHECK(napi_create_int64(env, kFSEventStreamEventIdSinceNow, &value) == napi_ok);
  CHECK(napi_set_named_property(env, flags, "SinceNow", value) == napi_ok);
  return flags;
}
napi_value Init(napi_env env, napi_value exports)
{
  napi_value global = fse_environment_create(env);
  napi_value constants = Init_Constants(env);
  napi_value flags = Init_Flags(env);

  napi_property_descriptor descriptors[] = {
      {"global", NULL, NULL, NULL, NULL, global, napi_default, NULL},
      {"start", NULL, FSEStart, NULL, NULL, NULL, napi_default, NULL},
      {"stop", NULL, FSEStop, NULL, NULL, NULL, napi_default, NULL},
      {"constants", NULL, NULL, NULL, NULL, constants, napi_default, NULL},
      {"flags", NULL, NULL, NULL, NULL, flags, napi_default, NULL}};

  CHECK(napi_define_properties(env, exports, 5, descriptors) == napi_ok);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
