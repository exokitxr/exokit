const path = require('path');
const fs = require('fs');
const url = require('url');

const symbols = require('./symbols');
const mkdirp = require('mkdirp');
const parseIntStrict = require('parse-int');

function _getBaseUrl(u) {
  let baseUrl;
  if (/^file:\/\/(.*)$/.test(u)) {
    baseUrl = u;
  } else {
    const parsedUrl = url.parse(u);
    baseUrl = url.format({
      protocol: parsedUrl.protocol || 'http:',
      host: parsedUrl.host || '127.0.0.1',
      pathname: parsedUrl.pathname.replace(/\/[^\/]*\.[^\/]*$/, '') || '/',
    });
  }
  if (!/\/$/.test(baseUrl) && !/\./.test(baseUrl.match(/\/([^\/]*)$/)[1])) {
    baseUrl = baseUrl + '/';
  }
  return baseUrl;
}
module.exports._getBaseUrl = _getBaseUrl;

function _normalizeUrl(src, baseUrl) {
  if (!/^(?:https?|data|blob):/.test(src)) {
    return new URL(src, baseUrl).href
      .replace(/^(file:\/\/)\/([a-z]:.*)$/i, '$1$2');
  } else {
    return src;
  }
}
module.exports._normalizeUrl = _normalizeUrl;

const _makeHtmlCollectionProxy = (el, query) => new Proxy(el, {
  get(target, prop) {
    const propN = parseIntStrict(prop);
    if (propN !== undefined) {
      return el.querySelectorAll(query)[propN];
    } else if (prop === 'length') {
      return el.querySelectorAll(query).length;
    } else {
      return undefined;
    }
  },
  set(target, prop, value) {
    return true;
  },
  deleteProperty(target, prop) {
    return true;
  },
  has(target, prop) {
    if (typeof prop === 'number') {
      return el.querySelectorAll(query)[prop] !== undefined;
    } else if (prop === 'length') {
      return true;
    } else {
      return false;
    }
  },
});
module.exports._makeHtmlCollectionProxy = _makeHtmlCollectionProxy;

const _elementGetter = (self, attribute) => self.listeners(attribute).filter(l => l[symbols.listenerSymbol])[0];
module.exports._elementGetter = _elementGetter;

const _elementSetter = (self, attribute, cb) => {
  const listener = _elementGetter(self, attribute);
  if (listener) {
    self.removeEventListener(attribute, listener);
    listener[symbols.listenerSymbol] = false;
  }
  
  if (typeof cb === 'function') {
    self.addEventListener(attribute, cb);
    cb[symbols.listenerSymbol] = true;
  }
};
module.exports._elementSetter = _elementSetter;
