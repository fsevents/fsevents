/*
 ** © 2014 by Philipp Dunkel <pip@pipobscure.com>
 ** Licensed under MIT License.
 */

/* jshint node:true */
'use strict';

var Native = require('./build/Release/fse').FSEvents;
var Constants = require('./build/Release/fseconst');

var EventEmitter = require('events').EventEmitter;
var util = require('util');
var getInfo = require('./fsinfo.js');
var fs = require('fs');

module.exports = FSEvents;
module.exports.getInfo = getInfo;
module.exports.Native = Native;
module.exports.Contants = Constants;

function FSEvents(path) {
  if (!(this instanceof FSEvents)) return new FSEvents(path);
  EventEmitter.call(this);
  Object.defineProperty(this, 'native', {
    value: new Native(String(path || process.cwd()), nativeListener.bind(null, this))
  });
}
util.inherits(FSEvents, EventEmitter);

Object.keys(Constants).forEach(function(key) {
  FSEvents.prototype[key] = Constants[key];
});

function nativeListener(obj, path, flags, id) {
  obj.emit('fsevent', path, flags, id);
  var info = getInfo(path, flags);
  info.id = id;

  if (info.event == 'moved') {
    fs.stat(info.path, function(err, stat) {
      info.event = (err || !stat) ? 'moved-out' : 'moved-in';
      obj.emit('change', path, info);
      obj.emit(info.event, path, info);
    });
  } else {
    obj.emit('change', path, info);
    if (info.event !== 'unknown') obj.emit(info.event, path, info);
  }
}