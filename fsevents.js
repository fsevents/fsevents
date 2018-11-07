/*
 ** © 2014-2018 by Philipp Dunkel <pip@pipobscure.com>
 ** Licensed under MIT License.
 */

/* jshint node:true */
'use strict';

if (process.platform !== 'darwin') {
  throw new Error(`Module 'fsevents' is not compatible with platform '${process.platform}'`);
}

var Native = require('./fsevents.node');

var EventEmitter = require('events').EventEmitter;
var fs = require('fs');
var inherits = require('util').inherits;

Native.FSEvents = function FSEvents(path, handler) {
  if ('string' !== typeof path) throw new TypeError('path must be a string');
  if ('function' !== typeof handler) throw new TypeError('handler must be a function');

  let native;
  this.start = () => {
    if (native) return this;
    native = Native.start(path, handler);
    return this;
  };
  this.stop = () => {
    if (!native) return this;
    native = Native.stop(native);
    return this;
  };
  this[Symbol.toStringTag] = 'FSEvents';
};

const native = Symbol('native');
function FSEvents(path, handler) {
  EventEmitter.call(this);
  this[native] = new Native.FSEvents(String(path || ''), handler);
}

inherits(FSEvents, EventEmitter);
FSEvents.prototype.start = function () { this[native].start(); return this; };
FSEvents.prototype.stop = function () { this[native].stop(); return this; };

module.exports = watch;
module.exports.getInfo = getInfo;
module.exports.FSEvents = Native.FSEvents;
module.exports.Constants = Native.Constants;

var defer = global.setImmediate || process.nextTick;

function watch(path) {
  var fse = new FSEvents(String(path || ''), handler);
  EventEmitter.call(fse);
  return fse;

  function handler(path, flags, id) {
    defer(function () {
      fse.emit('fsevent', path, flags, id);
      var info = getInfo(path, flags);
      info.id = id;
      if (info.event === 'moved') {
        fs.stat(info.path, function (err, stat) {
          info.event = (err || !stat) ? 'moved-out' : 'moved-in';
          fse.emit('change', path, info);
          fse.emit(info.event, path, info);
        });
      } else {
        fse.emit('change', path, info);
        fse.emit(info.event, path, info);
      }
    });
  }
}

function getFileType(flags) {
  if (Native.Constants.kFSEventStreamEventFlagItemIsFile & flags) return 'file';
  if (Native.Constants.kFSEventStreamEventFlagItemIsDir & flags) return 'directory';
  if (Native.Constants.kFSEventStreamEventFlagItemIsSymlink & flags) return 'symlink';
}

function getEventType(flags) {
  if (Native.Constants.kFSEventStreamEventFlagItemRemoved & flags) return 'deleted';
  if (Native.Constants.kFSEventStreamEventFlagItemRenamed & flags) return 'moved';
  if (Native.Constants.kFSEventStreamEventFlagItemCreated & flags) return 'created';
  if (Native.Constants.kFSEventStreamEventFlagItemModified & flags) return 'modified';
  if (Native.Constants.kFSEventStreamEventFlagRootChanged & flags) return 'root-changed';

  return 'unknown';
}

function getFileChanges(flags) {
  return {
    inode: !!(Native.Constants.kFSEventStreamEventFlagItemInodeMetaMod & flags),
    finder: !!(Native.Constants.kFSEventStreamEventFlagItemFinderInfoMod & flags),
    access: !!(Native.Constants.kFSEventStreamEventFlagItemChangeOwner & flags),
    xattrs: !!(Native.Constants.kFSEventStreamEventFlagItemXattrMod & flags)
  };
}

function getInfo(path, flags) {
  return {
    path: path,
    event: getEventType(flags),
    type: getFileType(flags),
    changes: getFileChanges(flags),
    flags: flags
  };
}
