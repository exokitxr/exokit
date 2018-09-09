const parseIntStrict = require('parse-int');
const url = require('url');
const GlobalContext = require('./GlobalContext');

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

/**
 * Convert (normalize prototype) native object to pass to JS.
 * Or convert JS object when passing to native.
 * Node's vm module was slow so there's vm-one for one-way bindings.
 * github.com/modulesio/vm-one
 * nativeVm.setPrototype will do the nan casting setPrototype between types.
 * Required for instanceof to work.
 *
 * @param obj - Object to convert.
 * @param targetContext - window or vm context that prototype will get retrieved from.
 */
const _normalizeBuffer = (function () {
  let dummyWindow;

  return (obj, targetContext) => {
    // Dummy window to reference original prototypes
    // (e.g., in case Promise gets overwritten by polyfill).
    if (!dummyWindow) {
      let vmo = GlobalContext.nativeVm.make();
      dummyWindow = vmo.getGlobal();
      dummyWindow.vm = vmo;
    }

    const name = obj && obj.constructor && obj.constructor.name;

    switch (name) {
      case 'Buffer':
      case 'ArrayBuffer':
      case 'Uint8Array':
      case 'Uint8ClampedArray':
      case 'Int8Array':
      case 'Uint16Array':
      case 'Int16Array':
      case 'Uint32Array':
      case 'Int32Array':
      case 'Float32Array':
      case 'Float64Array':
      case 'DataView': {
        if (!(obj instanceof targetContext[name])) {
          GlobalContext.nativeVm.setPrototype(obj, targetContext[name].prototype);
        }
        break;
      }
      case 'Blob': {
        if (!(obj.buffer instanceof targetContext.Buffer)) {
          GlobalContext.nativeVm.setPrototype(obj.buffer, targetContext.Buffer.prototype);
        }
        break;
      }
      case 'Promise': {
        if (!(obj instanceof targetContext[name])) {
          if (targetContext.prototype === dummyWindow.prototype) {
            // If passing to JS, cast to the original Promise prototype in case it was
            // overwritten by the site (e.g., polyfill).
            GlobalContext.nativeVm.setPrototype(obj, dummyWindow.Promise.prototype);
          } else {
            GlobalContext.nativeVm.setPrototype(obj, targetContext[name].prototype);
          }
        }
        break;
      }
    }
    return obj;
  };
})();
module.exports._normalizeBuffer = _normalizeBuffer;

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
