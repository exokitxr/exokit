const parseIntStrict = require('parse-int');
const url = require('url');
const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');

function _getBaseUrl(u) {
  let baseUrl;
  if (/^file:\/\/(.*)$/.test(u)) {
    baseUrl = u;
  } else {
    const parsedUrl = url.parse(u);
    baseUrl = url.format({
      protocol: parsedUrl.protocol || 'http:',
      host: parsedUrl.host || '127.0.0.1',
      pathname: parsedUrl.pathname,
      search: parsedUrl.search,
    });
  }
  if (!/\/$/.test(baseUrl) && !/\./.test(baseUrl.match(/\/([^\/]*)$/)[1])) {
    baseUrl = baseUrl + '/';
  }
  return baseUrl;
}
module.exports._getBaseUrl = _getBaseUrl;

function _makeNormalizeUrl(baseUrl) {
  return src => {
    if (!/^[a-z]+:\/\//i.test(src)) {
      src = new URL(src, baseUrl).href
        .replace(/^(file:\/\/)\/([a-z]:.*)$/i, '$1$2');
    }
    return src;
  };
}
module.exports._makeNormalizeUrl = _makeNormalizeUrl;

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

const _runJavascript = (jsString, window, filename = 'script', lineOffset = 0, colOffset = 0) => {
  try {
    window.vm.run(jsString, filename, lineOffset, colOffset);
  } catch (err) {
    console.warn(err.stack);
  }
};
module.exports._runJavascript = _runJavascript;

const NORMALIZE_LIST = [
  'ArrayBuffer',
  'Buffer',
  'DataView',
  'Float32Array',
  'Float64Array',
  'Int16Array',
  'Int32Array',
  'Int8Array',
  'Promise',
  'Uint16Array',
  'Uint32Array',
  'Uint8Array',
  'Uint8ClampedArray'
];

/**
 * Normalize prototype between native and V8.
 * Node's vm module was slow so there's vm-one for one-way bindings:
 *   github.com/modulesio/vm-one
 * nativeVm.setPrototype will do the nan casting setPrototype between types.
 * Required for instanceof to work.
 *
 * @param obj - Object to convert.
 * @param targetContext - window or vm context that prototype will get retrieved from.
 */
const _normalizePrototype = (obj, targetContext) => {
  if (!obj || typeof obj !== 'object') { return obj; }

  let name = obj && obj.constructor && obj.constructor.name;

  if (NORMALIZE_LIST.indexOf(name) === -1) { return obj; }

  const isToWindow = !!targetContext[symbols.prototypesSymbol];

  if (isToWindow && obj instanceof targetContext[symbols.prototypesSymbol][name]) {
    return obj;
  }
  if (!isToWindow && obj instanceof targetContext[name]) {
    return obj;
  }

  // Convert Buffer's ArrayBuffer.
  if (name === 'Buffer') {
    _normalizePrototype(obj, targetContext);
    _normalizePrototype(obj.buffer, targetContext);
    return obj;
  }

  // Convert Blob's buffer.
  if (name === 'Blob') {
    _normalizePrototype(obj.buffer, targetContext);
    return obj;
  }

  // Normalize to window prototype.
  if (isToWindow) {
    GlobalContext.nativeVm.setPrototype(obj, targetContext[symbols.prototypesSymbol][name].prototype ||
                                             targetContext[name].prototype);
    return obj;
  }

  // Normalize to native prototype.
  GlobalContext.nativeVm.setPrototype(obj, targetContext[name].prototype);
  return obj;
};
module.exports._normalizePrototype = _normalizePrototype;

/**
 * Store prototypes on window object to be used alongside _normalizePrototype.
 * In case they get mangled by a website.
 */
module.exports._storeOriginalWindowPrototypes = function (window, prototypesSymbol) {
  window[prototypesSymbol] = {};
  NORMALIZE_LIST.forEach(prototypeName => {
    window[prototypesSymbol][prototypeName] = window[prototypeName];
  });
};

const _elementGetter = (self, attribute) => self.listeners(attribute)[0];
module.exports._elementGetter = _elementGetter;

const _elementSetter = (self, attribute, cb) => {
  if (typeof cb === 'function') {
    self.addEventListener(attribute, cb);
  } else {
    const listeners = self.listeners(attribute);
    for (let i = 0; i < listeners.length; i++) {
      self.removeEventListener(attribute, listeners[i]);
    }
  }
};
module.exports._elementSetter = _elementSetter;
