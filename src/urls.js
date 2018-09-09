const URL = require('url').URL;
const utils = require('./utils');

let id = 0;
const urls = new Map();
URL.createObjectURL = blob => {
  const url = 'blob:' + id++;
  urls.set(url, utils._normalizePrototype(blob, global));
  return url;
};
URL.revokeObjectURL = url => {
  urls.delete(url);
};

module.exports.urls = urls;
