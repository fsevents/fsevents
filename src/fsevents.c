/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

#include "runner.h"
#include <node_api.h>

#define CONSTANT(name) do {\
  CHECK(napi_create_int32(env, name, &value) == napi_ok);\
  CHECK(napi_set_named_property(env, constants, #name, value) == napi_ok);\
} while (0)

// constants from https://developer.apple.com/library/mac/documentation/Darwin/Reference/FSEvents_Ref/index.html#//apple_ref/doc/constant_group/FSEventStreamEventFlags
#ifndef kFSEventStreamEventFlagNone
#define kFSEventStreamEventFlagNone 0x00000000
#endif

#ifndef kFSEventStreamEventFlagMustScanSubDirs
#define kFSEventStreamEventFlagMustScanSubDirs 0x00000001
#endif

#ifndef kFSEventStreamEventFlagUserDropped
#define kFSEventStreamEventFlagUserDropped 0x00000002
#endif

#ifndef kFSEventStreamEventFlagKernelDropped
#define kFSEventStreamEventFlagKernelDropped 0x00000004
#endif

#ifndef kFSEventStreamEventFlagEventIdsWrapped
#define kFSEventStreamEventFlagEventIdsWrapped 0x00000008
#endif

#ifndef kFSEventStreamEventFlagHistoryDone
#define kFSEventStreamEventFlagHistoryDone 0x00000010
#endif

#ifndef kFSEventStreamEventFlagRootChanged
#define kFSEventStreamEventFlagRootChanged 0x00000020
#endif

#ifndef kFSEventStreamEventFlagMount
#define kFSEventStreamEventFlagMount 0x00000040
#endif

#ifndef kFSEventStreamEventFlagUnmount
#define kFSEventStreamEventFlagUnmount 0x00000080
#endif

#ifndef kFSEventStreamEventFlagItemCreated
#define kFSEventStreamEventFlagItemCreated 0x00000100
#endif

#ifndef kFSEventStreamEventFlagItemRemoved
#define kFSEventStreamEventFlagItemRemoved 0x00000200
#endif

#ifndef kFSEventStreamEventFlagItemInodeMetaMod
#define kFSEventStreamEventFlagItemInodeMetaMod 0x00000400
#endif

#ifndef kFSEventStreamEventFlagItemRenamed
#define kFSEventStreamEventFlagItemRenamed 0x00000800
#endif

#ifndef kFSEventStreamEventFlagItemModified
#define kFSEventStreamEventFlagItemModified 0x00001000
#endif

#ifndef kFSEventStreamEventFlagItemFinderInfoMod
#define kFSEventStreamEventFlagItemFinderInfoMod 0x00002000
#endif

#ifndef kFSEventStreamEventFlagItemChangeOwner
#define kFSEventStreamEventFlagItemChangeOwner 0x00004000
#endif

#ifndef kFSEventStreamEventFlagItemXattrMod
#define kFSEventStreamEventFlagItemXattrMod 0x00008000
#endif

#ifndef kFSEventStreamEventFlagItemIsFile
#define kFSEventStreamEventFlagItemIsFile 0x00010000
#endif

#ifndef kFSEventStreamEventFlagItemIsDir
#define kFSEventStreamEventFlagItemIsDir 0x00020000
#endif

#ifndef kFSEventStreamEventFlagItemIsSymlink
#define kFSEventStreamEventFlagItemIsSymlink 0x00040000
#endif

// constants from https://developer.apple.com/library/mac/documentation/Darwin/Reference/FSEvents_Ref/index.html#//apple_ref/doc/constant_group/FSEventStreamCreateFlags
#ifndef kFSEventStreamCreateFlagNone
#define kFSEventStreamCreateFlagNone 0x00000000
#endif

#ifndef kFSEventStreamCreateFlagUseCFTypes
#define kFSEventStreamCreateFlagUseCFTypes 0x00000001
#endif

#ifndef kFSEventStreamCreateFlagNoDefer
#define kFSEventStreamCreateFlagNoDefer 0x00000002
#endif

#ifndef kFSEventStreamCreateFlagWatchRoot
#define kFSEventStreamCreateFlagWatchRoot 0x00000004
#endif

#ifndef kFSEventStreamCreateFlagIgnoreSelf
#define kFSEventStreamCreateFlagIgnoreSelf 0x00000008
#endif

#ifndef kFSEventStreamCreateFlagFileEvents
#define kFSEventStreamCreateFlagFileEvents 0x00000010
#endif

static napi_value FSEStart(napi_env env, napi_callback_info info) {
  size_t argc = 2;
  napi_value argv[2];
  char path[PATH_MAX];

  CHECK(napi_get_cb_info(env, info, &argc, argv,  NULL, NULL) == napi_ok);
  CHECK(napi_get_value_string_utf8(env, argv[0], path, PATH_MAX, &argc) == 0);
  return start(env, &path, argv[1]);
}
static napi_value FSEStop(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value external, result;
  fse_t *instance;
  CHECK(napi_get_cb_info(env, info, &argc, &external,  NULL, NULL) == napi_ok);
  CHECK(napi_get_value_external(env, external, (void**)&instance) == napi_ok);
  stop(instance);
  CHECK(napi_get_undefined(env, &result) == napi_ok);
  return result;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_value constants;
  napi_value value;
  CHECK(napi_create_object(env, &constants) == napi_ok);

  CONSTANT(kFSEventStreamEventFlagNone);
  CONSTANT(kFSEventStreamEventFlagMustScanSubDirs);
  CONSTANT(kFSEventStreamEventFlagUserDropped);
  CONSTANT(kFSEventStreamEventFlagKernelDropped);
  CONSTANT(kFSEventStreamEventFlagEventIdsWrapped);
  CONSTANT(kFSEventStreamEventFlagHistoryDone);
  CONSTANT(kFSEventStreamEventFlagRootChanged);
  CONSTANT(kFSEventStreamEventFlagMount);
  CONSTANT(kFSEventStreamEventFlagUnmount);
  CONSTANT(kFSEventStreamEventFlagItemCreated);
  CONSTANT(kFSEventStreamEventFlagItemRemoved);
  CONSTANT(kFSEventStreamEventFlagItemInodeMetaMod);
  CONSTANT(kFSEventStreamEventFlagItemRenamed);
  CONSTANT(kFSEventStreamEventFlagItemModified);
  CONSTANT(kFSEventStreamEventFlagItemFinderInfoMod);
  CONSTANT(kFSEventStreamEventFlagItemChangeOwner);
  CONSTANT(kFSEventStreamEventFlagItemXattrMod);
  CONSTANT(kFSEventStreamEventFlagItemIsFile);
  CONSTANT(kFSEventStreamEventFlagItemIsDir);
  CONSTANT(kFSEventStreamEventFlagItemIsSymlink);

  napi_property_descriptor descriptors[] = {
    { "Constants", NULL,  NULL,     NULL, NULL,  constants, napi_default, NULL },
    { "start",     NULL,  FSEStart, NULL, NULL,  NULL,      napi_default, NULL },
    { "stop",      NULL,  FSEStop, NULL, NULL,  NULL,      napi_default, NULL }
  };
  CHECK(napi_define_properties(env, exports, 3, descriptors) == napi_ok);

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
