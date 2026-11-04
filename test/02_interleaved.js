const native = require("../fsevents.node");
const { mkdir, rm, touch, rename } = require("./utils/fs.js");
const { run, sleep } = require("./utils/misc.js");
const path = require("path");
const assert = require("assert");

const DIR = process.argv[2];

run(async () => {
  await mkdir(`${DIR}/A`);
  await mkdir(`${DIR}/B`);
  await sleep(100);
  const events = [];

  const listenerA = native.start(native.global, `${DIR}/A`, native.flags.SinceNow, (...args) => events.push(args));

  await touch(path.join(`${DIR}/A`, "created"));
  await sleep(500);
  const listenerB = native.start(native.global, `${DIR}/B`, native.flags.SinceNow, (...args) => events.push(args));
  await sleep(500);
  native.stop(listenerA);
  await rename(path.join(`${DIR}/A`, "created"), path.join(`${DIR}/B`, "renamed"));
  await sleep(500);
  await rm(path.join(`${DIR}/B`, "renamed"));
  await sleep(500);

  native.stop(listenerB);

  // FSEvents combines operation and metadata bits. Require each operation,
  // not incidental metadata accumulated by a particular macOS version.
  const file = native.constants.ItemIsFile;
  const expected = [
    [path.join(`${DIR}/A`, "created"), (native.constants.ItemCreated | file), 80865],
    [path.join(`${DIR}/B`, "renamed"), (native.constants.ItemRenamed | file), 80888],
    [path.join(`${DIR}/B`, "renamed"), (native.constants.ItemRemoved | file), 80910],
  ];
  const incomplete = native.constants.MustScanSubDirs | native.constants.UserDropped | native.constants.KernelDropped;
  assert.equal(events.length, expected.length);
  for (let idx = 0; idx < events.length; idx++) {
    assert.equal(events[idx].length, expected[idx].length);
    assert.equal(events[idx][0], expected[idx][0]);
    assert.equal(events[idx][1] & expected[idx][1], expected[idx][1]);
    assert.equal(events[idx][1] & incomplete, 0);
  }
});
