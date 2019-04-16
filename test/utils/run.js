
var assert = require('assert');
var EventEmitter = require('events').EventEmitter; 
exports.test = test;
function test(name, fn) {
  console.log('TAP version 13');
  var started = false;
  var planned = -1;
  var count = 0;
  var timeout = null;
  var tst = new EventEmitter();
  tst.plan = function (cnt) { planned = cnt; };
  tst.ok = function (cond, msg) {
    if (!started) {
      console.log('1..' + (planned > 0 ? planned : 'N'));
      started = true;
    }
    if (timeout) clearTimeout(timeout);
    count++;
    if (cond) {
      console.log('ok ' + count + ' ' + msg);
    } else {
      console.log('not ok ' + count + ' ' + msg);
    }
    planned--;
    if (!planned) {
      tst.emit('end');
    } else {
      timeout = setTimeout(function () {
        process.exit(1);
      }, 5000);
    }
  };
  tst.end = function () {
    if (timeout) clearTimeout(timeout);
    tst.emit('end');
  };
  fn(tst);
}
