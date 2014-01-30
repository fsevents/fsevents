/*
 ** © 2014 by Philipp Dunkel <pip@pipobscure.com>
 ** Licensed under MIT License.
 */

/* jshint node:true */
'use strict';

module.exports = getInfo;

var Constants = require('./build/Release/fseconst');

function getFileType(flags) {
  if (Constants.kFSEventStreamEventFlagItemIsFile & flags) return 'file';
  if (Constants.kFSEventStreamEventFlagItemIsDir & flags) return 'directory';
  if (Constants.kFSEventStreamEventFlagItemIsSymlink & flags) return 'symlink';
}

function getEventType(flags) {
  if (Constants.kFSEventStreamEventFlagItemRemoved & flags) return 'deleted';
  if (Constants.kFSEventStreamEventFlagItemCreated & flags) return 'created';
  if (Constants.kFSEventStreamEventFlagItemRenamed & flags) return 'moved';
  if (Constants.kFSEventStreamEventFlagItemModified & flags) return 'modified';
  return 'unknown';
}

function getFileChanges(flags) {
  return {
    inode: !! (Constants.kFSEventStreamEventFlagItemInodeMetaMod & flags),
    finder: !! (Constants.kFSEventStreamEventFlagItemFinderInfoMod & flags),
    access: !! (Constants.kFSEventStreamEventFlagItemChangeOwner & flags),
    xattrs: !! (Constants.kFSEventStreamEventFlagItemXattrMod & flags)
  };
}

function getInfo(path, flags) {
  return {
    path: path,
    event: getEventType(flags),
    type: getFileType(flags),
    changes: getFileChanges(flags),
  };
}