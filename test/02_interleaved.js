const native = require('../fsevents.node');
const { mkdir, rm, touch, rename } = require('./utils/fs.js');
const { run, sleep } = require('./utils/misc.js');
const path = require('path');
const assert = require('assert');

const DIR = process.argv[2];

run(async () => {
  await mkdir(`${DIR}/A`);
  await mkdir(`${DIR}/B`);
  await sleep(100);
  const events = [];

  const listenerA = native.start(`${DIR}/A`, (...args) => events.push(args));

  await touch(path.join(`${DIR}/A`, 'created'));
  await sleep(500);
  const listenerB = native.start(`${DIR}/B`, (...args) => events.push(args));
  await sleep(500);
  native.stop(listenerA);
  await rename(path.join(`${DIR}/A`, 'created'), path.join(`${DIR}/B`, 'renamed'));
  await sleep(500);
  await rm(path.join(`${DIR}/B`, 'renamed'));
  await sleep(500);

  native.stop(listenerB);

  const expected = [
    [path.join(`${DIR}/A`, 'created'), 66816, 80865],
    [path.join(`${DIR}/B`, 'renamed'), 67584, 80888],
    [path.join(`${DIR}/B`, 'renamed'), 68096, 80910]
  ];
  assert.equal(events.length, expected.length);
  for (let idx = 0; idx < events.length; idx++) {
    assert.equal(events[idx].length, expected[idx].length);
    assert.equal(events[idx][0], expected[idx][0]);
    assert.equal(events[idx][1], expected[idx][1]);
  }
});
