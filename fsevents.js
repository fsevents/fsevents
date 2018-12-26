/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

/* jshint node:true */
'use strict';

if (process.platform !== 'darwin') {
  throw new Error(`Module 'fsevents' is not compatible with platform '${process.platform}'`);
}

const Native = require('./fsevents.node');

function watch(path, handler) {
  if ('string' !== typeof path) throw new TypeError(`argument 1 must be a string and not a ${typeof path}`);
  if ('function' !== typeof handler) throw new TypeError(`argument 2 must be a function and not a ${typeof handler}`);


  let instance = Native.start(path, handler);
  return ()=>{
    const result = instance ? Promise.resolve(instance).then(Native.stop) : null;
    instance = null;
    return result;
  };
}
function getInfo(path, flags) {
  return {
    path, flags,
    event: getEventType(flags),
    type: getFileType(flags),
    changes: getFileChanges(flags)
  };
}
function getFileType(flags) {
  if (Native.constants.kFSEventStreamEventFlagItemIsFile & flags) return 'file';
  if (Native.constants.kFSEventStreamEventFlagItemIsDir & flags) return 'directory';
  if (Native.constants.kFSEventStreamEventFlagItemIsSymlink & flags) return 'symlink';
}
function getEventType(flags) {
  if (Native.constants.kFSEventStreamEventFlagItemRemoved & flags) return 'deleted';
  if (Native.constants.kFSEventStreamEventFlagItemRenamed & flags) return 'moved';
  if (Native.constants.kFSEventStreamEventFlagItemCreated & flags) return 'created';
  if (Native.constants.kFSEventStreamEventFlagItemModified & flags) return 'modified';
  if (Native.constants.kFSEventStreamEventFlagRootChanged & flags) return 'root-changed';

  return 'unknown';
}
function getFileChanges(flags) {
  return {
    inode: !!(Native.constants.kFSEventStreamEventFlagItemInodeMetaMod & flags),
    finder: !!(Native.constants.kFSEventStreamEventFlagItemFinderInfoMod & flags),
    access: !!(Native.constants.kFSEventStreamEventFlagItemChangeOwner & flags),
    xattrs: !!(Native.constants.kFSEventStreamEventFlagItemXattrMod & flags)
  };
}

exports.watch = watch;
exports.getInfo = getInfo;
exports.constants = Native.constants;
