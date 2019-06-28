const path = require('path');
const fs = require('fs');
const url = require('url');

const mkdirp = require('mkdirp');
const parseIntStrict = require('parse-int');

const symbols = require('./symbols');
const vm = require('vm');
const fetch = require('window-fetch');

function _getBaseUrl(u, currentBaseUrl = '') {
  let result;
  if (/^file:\/\//.test(u)) {
    result = u;
  } else if (/^(?:data|blob):/.test(u)) {
    result = currentBaseUrl;
  } else {
    const parsedUrl = url.parse(u);
    result = url.format({
      protocol: parsedUrl.protocol || 'http:',
      host: parsedUrl.host || '127.0.0.1',
      pathname: parsedUrl.pathname.replace(/\/[^\/]*\.[^\/]*$/, '') || '/',
    });
  }
  if (!/\/$/.test(result) && !/\/[^\/]*?\.[^\/]*?$/.test(result)) {
    result += '/';
  }
  return result;
}
module.exports._getBaseUrl = _getBaseUrl;

function _normalizeUrl(src, baseUrl) {
  if (/^[/][/]/.test(src)) {
    src = new URL(baseUrl || 'https://www.example.com').protocol + src;
  }
  if (!/^(?:https?|data|blob):/.test(src)) {
    return new URL(src, baseUrl).href
      .replace(/^(file:\/\/)\/([a-z]:.*)$/i, '$1$2');
  } else {
    return src;
  }
}
module.exports._normalizeUrl = _normalizeUrl;

async function _fetch(url) {
  const res = await fetch(url)
  if (res.status >= 200 && res.status < 300) {
    return await res.text();
  } else {
    throw new Error('fetch got invalid status code: ' + res.status + ' : ' + url);
  }
}
module.exports._fetch = _fetch;

var _G = typeof global !== 'undefined' ? global : 
         typeof window !== 'undefined' ? window :
         typeof self !== 'undefined' ? self : this;
var _context = vm.createContext(_G);

async function _runMJS(code, {context, ...opts} = {}, fetch = _fetch) {
  if (!vm.SourceTextModule) {
    throw new Error('ECMAScript modules are not supported: ' + url);
  }
  if (!context) {
    context = _context;
  }
  const script = new vm.SourceTextModule(code, {context, ...opts});
  await script.link(async (path, {url, context}) => {
    url = _normalizeUrl(path, url);
    const code = await fetch(url);
    return new vm.SourceTextModule(code, {url, context});
  });
  script.instantiate();
  return await script.evaluate();
}
module.exports._runMJS = _runMJS;

function _runJS(code, opts) {
  return vm.runInThisContext(code, opts);
}
module.exports._runJS = _runJS;

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
