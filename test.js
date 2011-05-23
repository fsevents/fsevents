#!/usr/bin/node
;
var FS = require("fs");
var FSEvents = require('./fsevents.js');

global.fse = new FSEvents(__dirname);
global.fse.on("change",function(path, flags, evtid) {
	console.log("CHG("+evtid+"): "+path);
	if (FS.statSync("stop")) {
		console.log("Stopping");
		FS.unlinkSync("stop");
		global.fse.stop();
	}
	
});

