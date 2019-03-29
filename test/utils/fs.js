const fs = require('fs');
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
  const files = await fs.promises.readdir(dir);
  return files.map((child) => path.join(file, child));
};
exports.stat = fs.promises.stat;
exports.mkdir = fs.promises.mkdir;
exports.write = fs.promises.writeFile;
exports.read = fs.promises.readFile;
exports.chmod = fs.promises.chmod
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
exports.rename = fs.promises.rename;
