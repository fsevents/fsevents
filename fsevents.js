var util = require('util');
var events = require('events');
var binding = require('./bindings');

module.exports = binding.FSEvents;
util.inherits(module.exports, events.EventEmitter);
