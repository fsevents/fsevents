// Copyright (c) 2015, StrongLoop, Inc. <callback@strongloop.com>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

'use strict';

module.exports = watch;
module.exports.getEventType = getEventType;
module.exports.getFileChanges = getFileChanges;
module.exports.getFileType = getFileType;
module.exports.getInfo = getInfo;

var EventEmitter = require('events').EventEmitter;
var fs = require('fs');

try {
  var binding = require('./build/Release/binding');
} catch (e) {
  var binding = require('./build/Debug/binding');
}

var DEFAULT_FLAGS = binding.kFSEventStreamCreateFlagWatchRoot |
                    binding.kFSEventStreamCreateFlagFileEvents;

var DEFAULT_LATENCY = 0;

function watch(path) {
  return new FSEvents(path);
}

function FSEvents(path) {
  EventEmitter.call(this);
  Object.defineProperty(this, 'path_', { value: String(path || '') });
  Object.defineProperty(this, 'handler_', { value: null, writable: true });
}

FSEvents.prototype = Object.create(EventEmitter.prototype);

FSEvents.prototype.start = function() {
  if (this.handler_ === null) {
    this.handler_ = handler;
    this.handler_.data =
        binding.start(this.path_, DEFAULT_FLAGS, DEFAULT_LATENCY, handler);
    this.handler_.target = this;
  }
  return this;

  function handler(events) {
    for (var i = 0, n = events.length; i < n; i += 3) {
      var path = events[i + 0];
      var flags = events[i + 1];
      var ident = events[i + 2];

      if (handler.target === null) continue;  // Watcher was stopped.
      handler.target.emit('fsevent', path, flags, ident);

      var info = getInfo(path, flags);
      info.id = ident;

      if (info.event !== 'moved') {
        handler.target.emit('change', path, info);
        handler.target.emit(info.event, path, info);
        continue;
      }

      fs.stat(path, function(handler, path, info, err) {
        if (handler.target === null) return;  // Watcher was stopped.
        info.event = err ? 'moved-out' : 'moved-in';
        handler.target.emit('change', path, info);
        handler.target.emit(info.event, path, info);
      }.bind(null, handler, path, info));
    }
  }
};

FSEvents.prototype.stop = function() {
  if (this.handler_ !== null) {
    binding.stop(this.handler_.data);
    this.handler_.target = null;
    this.handler_.data = null;
    this.handler_ = null;
  }
  return this;
};

function getFileType(flags) {
  if (flags & binding.kFSEventStreamEventFlagItemIsFile) return 'file';
  if (flags & binding.kFSEventStreamEventFlagItemIsDir) return 'directory';
  if (flags & binding.kFSEventStreamEventFlagItemIsSymlink) return 'symlink';
}

function getEventType(flags) {
  if (flags & binding.kFSEventStreamEventFlagItemRemoved) return 'deleted';
  if (flags & binding.kFSEventStreamEventFlagItemRenamed) return 'moved';
  if (flags & binding.kFSEventStreamEventFlagItemCreated) return 'created';
  if (flags & binding.kFSEventStreamEventFlagItemModified) return 'modified';
  if (flags & binding.kFSEventStreamEventFlagRootChanged) return 'root-changed';
  return 'unknown';
}

function getFileChanges(flags) {
  return {
    access: !!(flags & binding.kFSEventStreamEventFlagItemChangeOwner),
    finder: !!(flags & binding.kFSEventStreamEventFlagItemFinderInfoMod),
    inode:  !!(flags & binding.kFSEventStreamEventFlagItemInodeMetaMod),
    xattrs: !!(flags & binding.kFSEventStreamEventFlagItemXattrMod),
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
