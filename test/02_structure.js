/*
 ** © 2020 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
 ** Licensed under MIT License.
 */

/* jshint node:true */
'use strict';

const assert = require('assert');
const fsevents = require('../fsevents');
const { run } = require('./utils/misc.js');

run(async () => {
  assert.equal('object', typeof fsevents);
  assert.equal('function', typeof fsevents.watch);
  assert.equal('function', typeof fsevents.getInfo);
  assert.equal('object', typeof fsevents.constants);

  assert.equal('file', fsevents.getInfo('path', fsevents.constants.ItemIsFile).type);
  assert.equal('directory', fsevents.getInfo('path', fsevents.constants.ItemIsDir).type);
  assert.equal('symlink', fsevents.getInfo('path', fsevents.constants.ItemIsSymlink).type);

  assert.equal('deleted', fsevents.getInfo('path', fsevents.constants.ItemRemoved).event);
  assert.equal('moved', fsevents.getInfo('path', fsevents.constants.ItemRenamed).event);
  assert.equal('created', fsevents.getInfo('path', fsevents.constants.ItemCreated).event);
  assert.equal('modified', fsevents.getInfo('path', fsevents.constants.ItemModified).event);
  assert.equal('root-changed', fsevents.getInfo('path', fsevents.constants.RootChanged).event);
});
