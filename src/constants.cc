/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

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

static Napi::Object Constants() {
  Napi::Object object = Napi::Object::New(env);
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagNone"), Napi::Number::New(env, kFSEventStreamEventFlagNone));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagMustScanSubDirs"), Napi::Number::New(env, kFSEventStreamEventFlagMustScanSubDirs));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagUserDropped"), Napi::Number::New(env, kFSEventStreamEventFlagUserDropped));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagKernelDropped"), Napi::Number::New(env, kFSEventStreamEventFlagKernelDropped));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagEventIdsWrapped"), Napi::Number::New(env, kFSEventStreamEventFlagEventIdsWrapped));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagHistoryDone"), Napi::Number::New(env, kFSEventStreamEventFlagHistoryDone));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagRootChanged"), Napi::Number::New(env, kFSEventStreamEventFlagRootChanged));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagMount"), Napi::Number::New(env, kFSEventStreamEventFlagMount));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagUnmount"), Napi::Number::New(env, kFSEventStreamEventFlagUnmount));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemCreated"), Napi::Number::New(env, kFSEventStreamEventFlagItemCreated));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemRemoved"), Napi::Number::New(env, kFSEventStreamEventFlagItemRemoved));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemInodeMetaMod"), Napi::Number::New(env, kFSEventStreamEventFlagItemInodeMetaMod));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemRenamed"), Napi::Number::New(env, kFSEventStreamEventFlagItemRenamed));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemModified"), Napi::Number::New(env, kFSEventStreamEventFlagItemModified));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemFinderInfoMod"), Napi::Number::New(env, kFSEventStreamEventFlagItemFinderInfoMod));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemChangeOwner"), Napi::Number::New(env, kFSEventStreamEventFlagItemChangeOwner));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemXattrMod"), Napi::Number::New(env, kFSEventStreamEventFlagItemXattrMod));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemIsFile"), Napi::Number::New(env, kFSEventStreamEventFlagItemIsFile));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemIsDir"), Napi::Number::New(env, kFSEventStreamEventFlagItemIsDir));
  object.Set(Napi::String::New(env, "kFSEventStreamEventFlagItemIsSymlink"), Napi::Number::New(env, kFSEventStreamEventFlagItemIsSymlink));
  return object;
}
