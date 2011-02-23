Native Access to Mac OS-X FSEvents - API
========================================

** Example: **
		var FSEvents = require("fsevents");
		var fso = new FSEvents("/path/do/folder");
		fso.on("change",function(path, flags, evtid) {
			console.log(evtid+": "+path+" ("+flags+")");
		});

** Object Constructor: **
		FSEvents(<path-name>[, <event-id>])
