const native = require("../fsevents.node");
const { rm, touch, rename } = require("./utils/fs.js");
const { run, sleep } = require("./utils/misc.js");
const path = require("path");
const assert = require("assert");

const DIR = process.argv[2];

// Helper function to check if required flags are present
function hasFlags(actualFlags, requiredFlags) {
  return requiredFlags.every(flag => (actualFlags & flag) !== 0);
}

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

  // Instead of checking exact flag values, check for required flags
  const constants = {
    ItemCreated: 256,
    ItemRemoved: 512,
    ItemRenamed: 2048,
    ItemModified: 4096,
    ItemIsFile: 65536
  };

  const expectedPatterns = [
    { path: path.join(DIR, "created"), requiredFlags: [constants.ItemCreated, constants.ItemIsFile] },
    { path: path.join(DIR, "created"), requiredFlags: [constants.ItemRenamed, constants.ItemIsFile] },
    { path: path.join(DIR, "renamed"), requiredFlags: [constants.ItemRenamed, constants.ItemIsFile] },
    { path: path.join(DIR, "renamed"), requiredFlags: [constants.ItemRemoved, constants.ItemIsFile] },
  ];

  assert.equal(events.length, expectedPatterns.length);
  for (let idx = 0; idx < events.length; idx++) {
    assert.equal(events[idx].length, 3);  // [path, flags, id]
    assert.equal(events[idx][0], expectedPatterns[idx].path);
    assert(hasFlags(events[idx][1], expectedPatterns[idx].requiredFlags), 
      `Event ${idx}: flags ${events[idx][1]} should contain required flags ${expectedPatterns[idx].requiredFlags.join(', ')}`);
  }
});
