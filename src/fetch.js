const { Navigator } = require('./Navigator');
const fetch = require('window-fetch');

Object.assign(module.exports, fetch);

module.exports = (u, options) => {
  const o = Object.assign({}, options || {});
  const h = (o.headers instanceof fetch.Headers) ? o.headers : new fetch.Headers(o.headers);
  h.set('User-Agent', Navigator.userAgent);
  o.headers = h;
  return fetch(u, o);
};

