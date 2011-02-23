#!/usr/bin/node
;

var FSEvents = require('./fsevents.js');

global.fse = new FSEvents(__dirname);
global.fse.on("change",function(path, flags, evtid) {
	console.log(evtid+": "+path+" ("+flags+")");
});

