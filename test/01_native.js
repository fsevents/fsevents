const native = require("../fsevents.node");
const { rm, touch, rename } = require("./utils/fs.js");
const { run, sleep } = require("./utils/misc.js");
const path = require("path");
const assert = require("assert");

const DIR = process.argv[2];

run(async () => {
  const events = [];
  const listener = native.start(native.global, DIR, native.flags.SinceNow, (...args) => events.push(args));

  await touch(path.join(DIR, "created"));
  await sleep(250);
  await rename(path.join(DIR, "created"), path.join(DIR, "renamed"));
  await sleep(250);
  await rm(path.join(DIR, "renamed"));
  await sleep(500);

  native.stop(listener);

  // FSEvents combines operation and metadata bits. Require each operation,
  // not incidental metadata accumulated by a particular macOS version.
  const file = native.constants.ItemIsFile;
  const expected = [
    [path.join(DIR, "created"), (native.constants.ItemCreated | file), 80865],
    [path.join(DIR, "created"), (native.constants.ItemRenamed | file), 80887],
    [path.join(DIR, "renamed"), (native.constants.ItemRenamed | file), 80888],
    [path.join(DIR, "renamed"), (native.constants.ItemRemoved | file), 80910],
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
