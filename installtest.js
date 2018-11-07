const { utimesSync } = require('fs');
const native = require('./fsevents.node');
let timeout = setTimeout(() => native.stop(handle), 500);
let handle = native.start(__dirname, async(...args)=>{
  timeout = timeout && clearTimeout(timeout);
  handle = (await Promise.resolve(handle)) && native.stop(handle);
  process.stdout.write('working\n');
});
utimesSync(__filename, Date.now(), Date.now());
process.stdout.write('testing installation: ');
