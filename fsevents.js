var util = require('util');
var events = require('events');
var binding = require('./build/Release/bindings');

module.exports = binding.FSEvents;
util.inherits(module.exports, events.EventEmitter);
