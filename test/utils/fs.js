const fs = require('fs');
const path = require('path');

exports.rmrf = async (file)=>{
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
exports.rm = async (file, stat)=>{
  stat = stat || await exports.stat(file);
  return await new Promise((resolve, reject) => {
    fs[stat.isDirectory() ? 'rmdir' : 'unlink'](file, (err) => {
      if (err) return reject(err);
      resolve();
    });
  });
};
exports.readdir = async (file)=>{
  return await new Promise((resolve, reject)=>{
    fs.readdir(file, (err, files)=>{
      if (err) return reject(err);
      resolve(files.map((child) => path.join(file, child)));
    });
  });
};
exports.stat = async (file)=>{
  return await new Promise((resolve, reject)=>{
    fs.stat(file, (err, stat)=>{
      if (err) return reject(err);
      if (!stat) return reject(new Error(`no stat for: ${file}`));
      resolve(stat);
    });
  });
};
exports.mkdir = async (file)=>{
  return await new Promise((resolve, reject)=>{
    fs.mkdir(file, (err)=>{
      if (err) return reject(err);
      resolve();
    });
  });
};
exports.write = async (file, content)=>{
  return await new Promise((resolve, reject)=>{
    fs.writeFile(file, content, (err) => {
      if (err) return reject(err);
      resolve();
    });
  });
};
exports.read = async (file) => {
  return await new Promise((resolve, reject)=>{
    fs.readFile(file, 'utf-8', (err, content)=>{
      if (err) return reject(err);
      resolve(content);
    });
  });
};
exports.chmod = async (file, mode) => {
  return await new Promise((resolve, reject) => {
    fs.chmod(file, mode, (err)=> {
      if (err) return reject(err);
      resolve();
    });
  });
};
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
exports.rename = async (orig, name) => {
  return await new Promise((resolve, reject)=>{
    fs.rename(orig, name, (err)=>{
      if (err) return reject(err);
      resolve();
    });
  });
};
