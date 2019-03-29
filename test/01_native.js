const native = require('../fsevents.node');
const { rm, touch, rename } = require('./utils/fs.js');
const { run, sleep } = require('./utils/misc.js');
const path = require('path');
const assert = require('assert');

const DIR = process.argv[2];

run(async () => {
  const events = [];
  const listener = native.start(DIR, (...args) => events.push(args));

  await touch(path.join(DIR, 'created'));
  await sleep(250);
  await rename(path.join(DIR, 'created'), path.join(DIR, 'renamed'));
  await sleep(250);
  await rm(path.join(DIR, 'renamed'));
  await sleep(500);

  native.stop(listener);

  const expected = [
    [path.join(DIR, 'created'), 66816, 80865],
    [path.join(DIR, 'created'), 68864, 80887],
    [path.join(DIR, 'renamed'), 67584, 80888],
    [path.join(DIR, 'renamed'), 68096, 80910]
  ];
  assert.equal(events.length, expected.length);
  for (let idx = 0; idx < events.length; idx++) {
    assert.equal(events[idx].length, expected[idx].length);
    assert.equal(events[idx][0], expected[idx][0]);
    assert.equal(events[idx][1], expected[idx][1]);
  }
});
