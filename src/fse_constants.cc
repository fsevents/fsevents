/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#include "nan.h"
#include <CoreServices/CoreServices.h>

static void Init (v8::Handle<v8::Object> exports) {
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagNone"), v8::Integer::New(0x00000000));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagMustScanSubDirs"), v8::Integer::New(0x00000001));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagUserDropped"), v8::Integer::New(0x00000002));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagKernelDropped"), v8::Integer::New(0x00000004));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagEventIdsWrapped"), v8::Integer::New(0x00000008));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagHistoryDone"), v8::Integer::New(0x00000010));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagRootChanged"), v8::Integer::New(0x00000020));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagMount"), v8::Integer::New(0x00000040));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagUnmount"), v8::Integer::New(0x00000080));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemCreated"), v8::Integer::New(0x00000100));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemRemoved"), v8::Integer::New(0x00000200));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemInodeMetaMod"), v8::Integer::New(0x00000400));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemRenamed"), v8::Integer::New(0x00000800));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemModified"), v8::Integer::New(0x00001000));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemFinderInfoMod"), v8::Integer::New(0x00002000));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemChangeOwner"), v8::Integer::New(0x00004000));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemXattrMod"), v8::Integer::New(0x00008000));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemIsFile"), v8::Integer::New(0x00010000));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemIsDir"), v8::Integer::New(0x00020000));
  exports->Set(v8::String::NewSymbol("kFSEventStreamEventFlagItemIsSymlink"), v8::Integer::New(0x00040000));
}
  
NODE_MODULE(fseconst, Init)
