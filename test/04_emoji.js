/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

/* jshint node:true */
'use strict';

const { rm, touch, rename } = require('./utils/fs.js');
const { sleep, capture, run } = require('./utils/misc.js');

const path = require('path');
const assert = require('assert');
const fsevents = require('../fsevents');

const DIR = process.argv[2];

run(async ()=>{
  const events = capture();
  const stop = fsevents.watch(DIR, events.callback);

  await touch(path.join(DIR, '\uD83D\uDE00'));
  await sleep(250);
  await rm(path.join(DIR, '\uD83D\uDE00'));
  
  await sleep(1500);

  await stop();

  const expected = [
    {
      path: path.join(DIR, '\uD83D\uDE00'),
      event: 'created',
      type: 'file'
    },
    {
      path: path.join(DIR, '\uD83D\uDE00'),
      event: 'deleted',
      type: 'file'
    }
  ];

  const parsed = events.map((args) => fsevents.getInfo(...args));
  parsed.forEach((actual, idx)=>{
    const expect = expected[idx];
    assert.equal(actual.path, expect.path);
    assert.equal(actual.type, expect.type);
    assert.equal(actual.event, expect.event);
  })
});
