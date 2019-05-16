/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

/* jshint node:true */
'use strict';

const assert = require('assert');
const fsevents = require('../fsevents');
const { run } = require('./utils/misc.js');

run(async ()=>{
  assert.equal('object', typeof fsevents);
  assert.equal('function', typeof fsevents.watch);
  assert.equal('function', typeof fsevents.getInfo);
  assert.equal('object', typeof fsevents.constants);

  assert.equal('file', fsevents.getInfo('path', fsevents.constants.kFSEventStreamEventFlagItemIsFile).type);
  assert.equal('directory', fsevents.getInfo('path', fsevents.constants.kFSEventStreamEventFlagItemIsDir).type);
  assert.equal('symlink', fsevents.getInfo('path', fsevents.constants.kFSEventStreamEventFlagItemIsSymlink).type);

  assert.equal('deleted', fsevents.getInfo('path', fsevents.constants.kFSEventStreamEventFlagItemRemoved).event);
  assert.equal('moved', fsevents.getInfo('path', fsevents.constants.kFSEventStreamEventFlagItemRenamed).event);
  assert.equal('created', fsevents.getInfo('path', fsevents.constants.kFSEventStreamEventFlagItemCreated).event);
  assert.equal('modified', fsevents.getInfo('path', fsevents.constants.kFSEventStreamEventFlagItemModified).event);
  assert.equal('root-changed', fsevents.getInfo('path', fsevents.constants.kFSEventStreamEventFlagRootChanged).event);
});
