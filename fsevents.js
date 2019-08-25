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
const events = Object.keys(Native.constants).reduce((obj, name) => {
  obj[name.replace('kFSEventStreamEventFlag', '')] = Native.constants[name];
  return obj;
}, {});

function watch(path, handler) {
  if ('string' !== typeof path) throw new TypeError(`fsevents argument 1 must be a string and not a ${typeof path}`);
  if ('function' !== typeof handler) throw new TypeError(`fsevents argument 2 must be a function and not a ${typeof handler}`);

  let instance = Native.start(path, handler);
  if (!instance) throw new Error(`could not watch: ${path}`);
  return () => {
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
  if (events.ItemIsFile & flags) return 'file';
  if (events.ItemIsDir & flags) return 'directory';
  if (events.ItemIsSymlink & flags) return 'symlink';
}
function getEventType(flags) {
  if (events.ItemRemoved & flags) return 'deleted';
  if (events.ItemRenamed & flags) return 'moved';
  if (events.ItemCreated & flags) return 'created';
  if (events.ItemModified & flags) return 'modified';
  if (events.RootChanged & flags) return 'root-changed';

  return 'unknown';
}
function getFileChanges(flags) {
  return {
    inode: !!(events.ItemInodeMetaMod & flags),
    finder: !!(events.ItemFinderInfoMod & flags),
    access: !!(events.ItemChangeOwner & flags),
    xattrs: !!(events.ItemXattrMod & flags)
  };
}

exports.watch = watch;
exports.getInfo = getInfo;
exports.constants = events;
