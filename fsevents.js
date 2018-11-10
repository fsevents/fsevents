/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

/* jshint node:true */
'use strict';

if (process.platform !== 'darwin') {
  throw new Error(`Module 'fsevents' is not compatible with platform '${process.platform}'`);
}

const { stat } = require('fs');
const Native = require('./fsevents.node');
const { EventEmitter } = require('events');

const native = new WeakMap();
class FSEvents {
  constructor(path, handler) {
    if ('string' !== typeof path) throw new TypeError('path must be a string');
    if ('function' !== typeof handler) throw new TypeError('function must be a function');
    Object.defineProperties(this, {
      path: { value: path },
      handler: { value: handler }
    });
  }
  start() {
    if (native.has(this)) return;
    const instance = Native.start(this.path, this.handler);
    native.set(this, instance);
    return this;
  }
  stop() {
    const instance = native.get(this);
    if (!instance) return;
    Native.stop(instance);
    native.delete(this);
    return this;
  }
}
FSEvents.prototype[Symbol.toStringTag] = 'FSEvents';

const fse = Symbol('fsevents');
class Emitter extends EventEmitter {
  constructor(path) {
    super();
    this[fse] = new FSEvents(path, (...args) => this.pushEvent(...args));
  }
  start() {
    this[fse].start();
    return this;
  }
  stop() {
    this[fse].stop();
    return this;
  }
  pushEvent(path, flags, id) {
    this.emit('fsevent', path, flags, id);
    const info = getInfo(path, flags, id);
    if (info.event === 'moved') {
      stat(info.path, (err, stat) => {
        info.event = (err || !stat) ? 'moved-out' : 'moved-in';
        this.emit('change', path, info);
        this.emit(info.event, path, info);
      });
    } else {
      Promise.resolve().then(() => {
        this.emit('change', path, info);
        this.emit(info.event, path, info);
      });
    }
  }
}
Emitter.prototype[Symbol.toStringTag] = 'FSEventsEmitter';

module.exports = function (path) { return new Emitter(path); }
module.exports.getInfo = getInfo;
module.exports.FSEvents = FSEvents;
module.exports.Constants = Native.Constants;

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

function getInfo(path, flags, id) {
  return {
    path, flags, id,
    event: getEventType(flags),
    type: getFileType(flags),
    changes: getFileChanges(flags)
  };
}
