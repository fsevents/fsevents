/*
** © 2020 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

#include <assert.h>

#define NAPI_VERSION 4
#include <node_api.h>

#include "rawfsevents.h"
#include "constants.h"

#ifndef CHECK
#ifdef NDEBUG
#define CHECK(x) \
  do             \
  {              \
    if (!(x))    \
      abort();   \
  } while (0)
#else
#define CHECK assert
#endif
#endif

typedef struct
{
  size_t count;
  fse_event_t *events;
} fse_js_event;

void fse_propagate_event(void *callback, size_t numevents, fse_event_t *events)
{
  fse_js_event *event = malloc(sizeof(*event));
  CHECK(event);
  event->count = numevents;
  event->events = events;
  CHECK(napi_call_threadsafe_function((napi_threadsafe_function)callback, event, napi_tsfn_blocking) == napi_ok);
}

void fse_dispatch_events(napi_env env, napi_value callback, void *context, void *data)
{
  fse_js_event *event = data;
  napi_value recv, args[3];
  size_t idx;
  CHECK(napi_get_null(env, &recv) == napi_ok);

  for (idx = 0; idx < event->count; idx++)
  {
    CHECK(napi_create_string_utf8(env, event->events[idx].path, NAPI_AUTO_LENGTH, &args[0]) == napi_ok);
    CHECK(napi_create_uint32(env, event->events[idx].flags, &args[1]) == napi_ok);
    CHECK(napi_create_int64(env, event->events[idx].id, &args[2]) == napi_ok);
    CHECK(napi_call_function(env, recv, callback, 3, args, &recv) == napi_ok);
  }

  free(event->events);
  free(event);
}

void fse_free_watcher(napi_env env, void *watcher, void *callback)
{
  fse_free(watcher);
}

void fse_watcher_started(void *context)
{
  if (context == NULL)
  {
    return;
  }
  napi_threadsafe_function callback = (napi_threadsafe_function)context;
  CHECK(napi_acquire_threadsafe_function(callback) == napi_ok);
}
void fse_watcher_ended(void *context)
{
  if (context == NULL)
  {
    return;
  }
  napi_threadsafe_function callback = (napi_threadsafe_function)context;
  CHECK(napi_release_threadsafe_function(callback, napi_tsfn_abort) == napi_ok);
}

static napi_value FSEStart(napi_env env, napi_callback_info info)
{
  size_t argc = 4;
  napi_value argv[argc];
  char path[PATH_MAX];
  napi_threadsafe_function callback = NULL;
  napi_value asyncResource, asyncName;
  int64_t since;

  CHECK(napi_get_cb_info(env, info, &argc, argv, NULL, NULL) == napi_ok);
  fse_loop_t *global = NULL;
  CHECK(napi_get_value_external(env, argv[0], (void **)&global) == napi_ok);
  CHECK(napi_get_value_string_utf8(env, argv[1], path, PATH_MAX, &argc) == napi_ok);
  CHECK(napi_get_value_int64(env, argv[2], &since) == napi_ok);

  CHECK(napi_create_object(env, &asyncResource) == napi_ok);
  CHECK(napi_create_string_utf8(env, "fsevents", NAPI_AUTO_LENGTH, &asyncName) == napi_ok);
  CHECK(napi_create_threadsafe_function(env, argv[3], asyncResource, asyncName, 0, 2, NULL, NULL, global, fse_dispatch_events, &callback) == napi_ok);
  CHECK(napi_ref_threadsafe_function(env, callback) == napi_ok);

  napi_value result;
  if (!callback)
  {
    CHECK(napi_get_undefined(env, &result) == napi_ok);
    return result;
  }
  fse_watcher_t watcher = fse_alloc();
  CHECK(watcher);
  watcher->global = global;
  watcher->since = since;
  fse_watch(path, fse_propagate_event, callback, fse_watcher_started, fse_watcher_ended, watcher);
  CHECK(napi_create_external(env, watcher, fse_free_watcher, callback, &result) == napi_ok);
  return result;
}
static napi_value FSEStop(napi_env env, napi_callback_info info)
{
  size_t argc = 2;
  napi_value argv[argc];
  fse_watcher_t watcher;
  fse_loop_t global;
  CHECK(napi_get_cb_info(env, info, &argc, argv, NULL, NULL) == napi_ok);
  CHECK(napi_get_value_external(env, argv[0], (void **)&global) == napi_ok);
  CHECK(napi_get_value_external(env, argv[1], (void **)&watcher) == napi_ok);
  napi_threadsafe_function callback = (napi_threadsafe_function)fse_context_of(watcher);
  if (callback)
  {
    CHECK(napi_unref_threadsafe_function(env, callback) == napi_ok);
  }
  fse_unwatch(watcher);
  napi_value result;
  CHECK(napi_get_undefined(env, &result) == napi_ok);
  return result;
}

void fse_free_global(napi_env env, void *global, void *hint)
{
  fse_destroy((fse_loop_t *)global);
  free(global);
}

#define CONSTANT(name)                                                               \
  do                                                                                 \
  {                                                                                  \
    CHECK(napi_create_int32(env, kFSEventStreamEventFlag##name, &value) == napi_ok); \
    CHECK(napi_set_named_property(env, constants, #name, value) == napi_ok);         \
  } while (0)

napi_value Init(napi_env env, napi_value exports)
{
  fse_loop_t *global = malloc(sizeof(fse_loop_t));
  fse_init(global);

  napi_value napiglobal;
  CHECK(napi_create_external(env, global, fse_free_global, NULL, &napiglobal) == napi_ok);

  napi_value value, constants;

  CHECK(napi_create_object(env, &constants) == napi_ok);
  CHECK(napi_create_int64(env, kFSEventStreamEventIdSinceNow, &value) == napi_ok);
  napi_property_descriptor descriptors[] = {
      {"global", NULL, NULL, NULL, NULL, napiglobal, napi_default, NULL},
      {"start", NULL, FSEStart, NULL, NULL, NULL, napi_default, NULL},
      {"stop", NULL, FSEStop, NULL, NULL, NULL, napi_default, NULL},
      {"constants", NULL, NULL, NULL, NULL, constants, napi_default, NULL},
      {"kFSEventStreamEventIdSinceNow", NULL, NULL, NULL, NULL, value, napi_default, NULL}};

  CHECK(napi_define_properties(env, exports, 5, descriptors) == napi_ok);

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

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
