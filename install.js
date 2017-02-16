if (process.platform === 'darwin') {
  var spawn = require('child_process').spawn;
  var child = spawn('node-pre-gyp', ['install', '--fallback-to-build'], {stdio: 'inherit'});
  child.on('close', function(code) {process.exit(code)});
}