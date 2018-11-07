/*
** © 2018 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "runner.h"
#define NAPI_EXPERIMENTAL
#include <node_api.h>
#include <stdio.h>

static void Callback(napi_env env, napi_value js_cb, void* context, void* data) {
  fse_event_t *event = (fse_event_t*)data;
  napi_value argv[3];
  napi_value this, result;
  assert(napi_get_null(env, &this) == napi_ok);
  assert(napi_create_string_utf8(env, event->path, event->pathlen, &argv[0]) == napi_ok);
  napi_create_int32(env, event->flags, &argv[1]);
  napi_create_int64(env, event->id, &argv[2]);
  assert(napi_call_function(env, this, js_cb, 3, argv, &result) == napi_ok);
  free(event);
}

static void cleanup(napi_env env, void* data, void* ignored) {
  fse_t *instance = (fse_t*)data;
  stop(instance);
  free(instance);
}

static napi_value FSEStart(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2], result;
  char path[PATH_MAX];
  napi_threadsafe_function cb;

  assert(napi_get_cb_info(env, info, &argc, argv,  NULL, NULL) == napi_ok);
  assert(napi_get_value_string_utf8(env, argv[0], path, PATH_MAX, &argc) == 0);
  assert(napi_create_threadsafe_function(env, argv[1], NULL, argv[0], 0, 1, NULL, NULL, NULL, Callback, &cb) == napi_ok);

  void *instance = start(path, cb);
  assert(napi_create_external(env, (void*) instance, cleanup, NULL, &result) == napi_ok);
  return result;
}
static napi_value FSEStop(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value external, result;
  fse_t *instance;
  assert(napi_get_cb_info(env, info, &argc, &external,  NULL, NULL) == napi_ok);
  assert(napi_get_value_external(env, external, (void**)&instance) == napi_ok);
  stop(instance);
  assert(napi_get_undefined(env, &result) == napi_ok);
  return result;
}

/*napi_value*/ NAPI_MODULE_INIT(/*napi_env env, napi_value exports*/) {
  napi_value constants;
  napi_value value;
  assert(napi_create_object(env, &constants) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagNone, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagNone", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagMustScanSubDirs, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagMustScanSubDirs", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagUserDropped, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagUserDropped", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagKernelDropped, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagKernelDropped", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagEventIdsWrapped, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagEventIdsWrapped", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagHistoryDone, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagHistoryDone", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagRootChanged, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagRootChanged", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagMount, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagMount", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagUnmount, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagUnmount", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemCreated, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemCreated", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemRemoved, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemRemoved", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemInodeMetaMod, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemInodeMetaMod", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemRenamed, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemRenamed", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemModified, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemModified", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemFinderInfoMod, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemFinderInfoMod", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemChangeOwner, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemChangeOwner", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemXattrMod, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemXattrMod", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemIsFile, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemIsFile", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemIsDir, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemIsDir", value) == napi_ok);

  napi_create_int32(env, kFSEventStreamEventFlagItemIsSymlink, &value);
  assert(napi_set_named_property(env, constants, "kFSEventStreamEventFlagItemIsSymlink", value) == napi_ok);

  napi_property_descriptor descriptors[] = {
    { "Constants", NULL,  NULL,     NULL, NULL,  constants, napi_default, NULL },
    { "start",     NULL,  FSEStart, NULL, NULL,  NULL,      napi_default, NULL },
    { "stop",      NULL,  FSEStop, NULL, NULL,  NULL,      napi_default, NULL }
  };
  assert(napi_define_properties(env, exports, 3, descriptors) == napi_ok);

  return exports;
}