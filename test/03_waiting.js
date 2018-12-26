/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

/* jshint node:true */
'use strict';

const path = require('path');
const assert = require('assert');
const fsevents = require('../fsevents');
const { run, sleep } = require('./utils/misc.js');
const { touch } = require('./utils/fs.js');

const DIR = process.argv[2];

run(async ()=>{
  const events = [];
  const stopWatching = fsevents.watch(DIR, (...args)=>events.push(args));
  await sleep(500);
  for (let idx = 0; idx < 10; idx++) {
    await touch(path.join(DIR, `${idx + 1}.touch`));
    await sleep(250);
  }
  await sleep(500);
  assert.equal(events.length, 10)
  await stopWatching();
});
