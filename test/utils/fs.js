const fs = require('fs');
const utils = require('util');
const path = require('path');

exports.rmrf = async (file) => {
  let stat;
  try {
    stat = await exports.stat(file);
  } catch(err) {
    //ignore
  }
  if (!stat) return;
  if (stat.isDirectory()) {
    const files = await exports.readdir(file);
    await Promise.all(files.map(exports.rmrf));
  }
  await exports.rm(file, stat);
}

exports.rm = async (file, stat) => {
  stat = stat || await exports.stat(file);
  return await new Promise((resolve, reject) => {
    fs[stat.isDirectory() ? 'rmdir' : 'unlink'](file, (err) => {
      if (err) return reject(err);
      resolve();
    });
  });
};
exports.readdir = async (dir) => {
  const files = await utils.promisify(fs.readdir)(dir);
  return files.map((child) => path.join(file, child));
};
exports.stat = utils.promisify(fs.stat);
exports.mkdir = utils.promisify(fs.mkdir);
exports.write = utils.promisify(fs.writeFile);
exports.read = utils.promisify(fs.readFile);
exports.chmod = utils.promisify(fs.chmod);
exports.rename = utils.promisify(fs.rename);
exports.touch = async (file) => {
  try {
    await exports.stat(file);
  } catch(err) {
    await exports.write(file, '');
  }

  return await new Promise((resolve, reject) => {
    fs.utimes(file, Date.now(), Date.now(), (err) => {
      if (err) return reject(err);
      resolve();
    });
  });
};

