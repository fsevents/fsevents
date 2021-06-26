#ifndef __constants_h
#define __constants_h

#include "Availability.h"
#include "CoreFoundation/CoreFoundation.h"
#include "CoreServices/CoreServices.h"

// constants from https://developer.apple.com/documentation/coreservices/1455361-fseventstreameventflags?language=objc
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

#ifndef kFSEventStreamEventFlagItemIsHardlink
#define kFSEventStreamEventFlagItemIsHardlink 0x00100000
#endif

#ifndef kFSEventStreamEventFlagItemIsLastHardlink
#define kFSEventStreamEventFlagItemIsLastHardlink 0x00200000
#endif

#ifndef kFSEventStreamEventFlagOwnEvent
#define kFSEventStreamEventFlagOwnEvent 0x00080000
#endif

#ifndef kFSEventStreamEventFlagItemCloned
#define kFSEventStreamEventFlagItemCloned 0x00400000
#endif

// constants from https://developer.apple.com/documentation/coreservices/1455376-fseventstreamcreateflags?language=objc
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

#ifndef kFSEventStreamCreateFlagUseExtendedData
#define kFSEventStreamCreateFlagUseExtendedData 0x00000040
#endif

#endif
