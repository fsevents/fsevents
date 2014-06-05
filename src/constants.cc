/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

using v8::String;
using v8::Integer;

static v8::Local<v8::Object> Constants() {
  NanEscapableScope();
  v8::Local<v8::Object> object = NanNew<v8::Object>();
  object->Set(NanNew<String>("kFSEventStreamEventFlagNone"), NanNew<Integer>(kFSEventStreamEventFlagNone));
  object->Set(NanNew<String>("kFSEventStreamEventFlagMustScanSubDirs"), NanNew<Integer>(kFSEventStreamEventFlagMustScanSubDirs));
  object->Set(NanNew<String>("kFSEventStreamEventFlagUserDropped"), NanNew<Integer>(kFSEventStreamEventFlagUserDropped));
  object->Set(NanNew<String>("kFSEventStreamEventFlagKernelDropped"), NanNew<Integer>(kFSEventStreamEventFlagKernelDropped));
  object->Set(NanNew<String>("kFSEventStreamEventFlagEventIdsWrapped"), NanNew<Integer>(kFSEventStreamEventFlagEventIdsWrapped));
  object->Set(NanNew<String>("kFSEventStreamEventFlagHistoryDone"), NanNew<Integer>(kFSEventStreamEventFlagHistoryDone));
  object->Set(NanNew<String>("kFSEventStreamEventFlagRootChanged"), NanNew<Integer>(kFSEventStreamEventFlagRootChanged));
  object->Set(NanNew<String>("kFSEventStreamEventFlagMount"), NanNew<Integer>(kFSEventStreamEventFlagMount));
  object->Set(NanNew<String>("kFSEventStreamEventFlagUnmount"), NanNew<Integer>(kFSEventStreamEventFlagUnmount));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemCreated"), NanNew<Integer>(kFSEventStreamEventFlagItemCreated));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemRemoved"), NanNew<Integer>(kFSEventStreamEventFlagItemRemoved));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemInodeMetaMod"), NanNew<Integer>(kFSEventStreamEventFlagItemInodeMetaMod));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemRenamed"), NanNew<Integer>(kFSEventStreamEventFlagItemRenamed));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemModified"), NanNew<Integer>(kFSEventStreamEventFlagItemModified));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemFinderInfoMod"), NanNew<Integer>(kFSEventStreamEventFlagItemFinderInfoMod));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemChangeOwner"), NanNew<Integer>(kFSEventStreamEventFlagItemChangeOwner));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemXattrMod"), NanNew<Integer>(kFSEventStreamEventFlagItemXattrMod));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemIsFile"), NanNew<Integer>(kFSEventStreamEventFlagItemIsFile));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemIsDir"), NanNew<Integer>(kFSEventStreamEventFlagItemIsDir));
  object->Set(NanNew<String>("kFSEventStreamEventFlagItemIsSymlink"), NanNew<Integer>(kFSEventStreamEventFlagItemIsSymlink));
  return NanEscapeScope(object);
}
