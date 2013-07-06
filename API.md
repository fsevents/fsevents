Native Access to Mac OS-X FSEvents - API
========================================

Example
-------

    var FSEvents = require("fsevents");
    var fso = FSEvents("/path/do/folder");
    fso.on("fsevent",function(path, flags, evtid) {
        console.log(evtid+": "+path+" ("+flags+")");
    });
