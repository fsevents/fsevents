#!/bin/sh
echo "fallback to: v1.2.4"
curl  https://codeload.github.com/strongloop/fsevents/tar.gz/v1.2.4 | tar xz
pushd fsevents-1.2.4
npm install node-pre-gyp
npm run install
popd

mv fsevents.js fsevents.bak.js
echo "module.exports = require('./fsevents-1.2.4/fsevents.js');" > fsevents.js
