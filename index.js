const path = require('path');
const url = require('url');

const fetch = require('window-fetch');
const {Response, Blob} = fetch;
const JsdomBlob = require('jsdom/lib/jsdom/living/generated/Blob');
JsdomBlob.interface = Blob;

const DomImplementation = require('jsdom/lib/jsdom/living/generated/DOMImplementation');
const DomImplementationImpl = require('jsdom/lib/jsdom/living/nodes/DOMImplementation-impl');
DomImplementationImpl.implementation.prototype._hasFeature = (hasFeature => function(feature, version) {
  return (feature === 'FetchExternalResources' && (version === 'link' || version === 'script' || version === 'iframe')) || hasFeature.apply(this, arguments);
})(DomImplementationImpl.implementation.prototype._hasFeature);

const HTMLCanvasElement = require('jsdom/lib/jsdom/living/nodes/HTMLCanvasElement-impl');
if (typeof global.nativeGl === 'undefined') {
  global.nativeGl = {};
}
const canvasImplementation = {
  getContext: () => nativeGl,
};
HTMLCanvasElement.implementation.prototype._getCanvas = () => canvasImplementation;

const Window = require('jsdom/lib/jsdom/browser/Window');

Window.prototype.fetch = (url, options) => {
  const blob = urls.get(url);
  if (blob) {
    return Promise.resolve(new Response(blob));
  } else {
    return fetch(url, options);
  }
};

// const {URL} = url;
let id = 0;
const urls = new Map();
const URL = require('whatwg-url/lib/URL').interface;
// Window.prototype.URL = URL;
URL.createObjectURL = blob => {
  const url = 'blob:' + id++;
  urls.set(url, blob);
  return url;
};
URL.revokeObjectURL = blob => {
  urls.delete(url);
};

const WebSocket = require('ws/lib/websocket');
Window.prototype.WebSocket = WebSocket;

const WindowWorker = require('window-worker');

const {LocalStorage} = require('node-localstorage');

const utils = require('jsdom/lib/jsdom/living/generated/utils');

const jsdom = require('jsdom');
const browserPoly = (s = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1';
  options.contentType = 'text/html';
  options.runScripts = 'dangerously';
  options.dataPath = options.dataPath || __dirname;
  options.beforeParse = window => {
    const baseUrl = options.url;
    const _normalizeUrl = url => {
      if (!/^.+?:/.test(url)) {
        url = baseUrl + ((!/\/$/.test(baseUrl) && !/^\//.test(url)) ? '/' : '') + url;
      }
      return url;
    };

    // synchronize script tag onload
    window.document[utils.implSymbol]._queue = {
      resume: () => {},
      push: callback => (err, data, response) => {
        callback(err, data, response);
      },
    };

    window.fetch = (fetch => (url, options) => fetch(_normalizeUrl(url), options))(window.fetch);

    class Worker extends WindowWorker {
      constructor(src, options) {
        if (src instanceof Blob) {
          super('data:application/javascript,' + src[Blob.BUFFER].toString('utf8'), options);
        } else {
          super(_normalizeUrl(src), options);
        }
      }
    }
    window.Worker = Worker;

    window.localStorage = new LocalStorage(path.join(options.dataPath, '.localStorage'));

    const rafCbs = [];
    window.requestAnimationFrame = fn => {
      rafCbs.push(fn);
    };
    window.clearAnimationFrame = fn => {
      const index = rafCbs.indexOf(fn);
      if (index !== -1) {
        rafCbs.splice(index, 1);
      }
    };
    window.tickAnimationFrame = () => {
      const localRafCbs = rafCbs.slice();
      rafCbs.length = 0;
      for (let i = 0; i < localRafCbs.length; i++) {
        localRafCbs[i]();
      }
    };
  };

  return new jsdom.JSDOM(s, options);
};
browserPoly.fetch = fetch;
module.exports = browserPoly;
