const URL = require('url').URL;

let id = 0;
const urls = new Map();
URL.createObjectURL = blob => {
  const url = 'blob:' + id++;
  urls.set(url, blob);
  return url;
};
URL.revokeObjectURL = url => {
  urls.delete(url);
};

module.exports.urls = urls;
