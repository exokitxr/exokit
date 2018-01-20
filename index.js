const path = require('path');
const url = require('url');

const fetch = require('window-fetch');
const {Response, Blob} = fetch;
const JsdomBlob = require('jsdom/lib/jsdom/living/generated/Blob');
JsdomBlob.interface = Blob;

const DomImplementation = require('jsdom/lib/jsdom/living/generated/DOMImplementation');
const DomImplementationImpl = require('jsdom/lib/jsdom/living/nodes/DOMImplementation-impl');
DomImplementationImpl.implementation.prototype._hasFeature = (hasFeature => function(feature, version) {
  return (feature === 'FetchExternalResources' && version === 'script') || hasFeature.apply(this, arguments);
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

const TinyWorker = require("tiny-worker");
class Worker extends TinyWorker {
  constructor(src, options) {
    if (src instanceof Blob) {
      super(new Function(src[Blob.BUFFER].toString('utf8')), options);
    } else {
      super(src, options);
    }
  }
}
Window.prototype.Worker = Worker;

const {LocalStorage} = require('node-localstorage');

const jsdom = require('jsdom');
const browserPoly = (s = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1';
  options.contentType = 'text/html';
  options.runScripts = 'dangerously';
  options.dataPath = options.dataPath || __dirname;

  const basePath = options.url;

  const result = new jsdom.JSDOM(s, options);
  const {window} = result;
  window.fetch = (fetch => function(url, options) {
    if (!/^.+?:/.test(url)) {
      url = basePath + (!/^\//.test(url) ? '/' : '') + url;
    }
    return fetch(url, options);
  })(window.fetch)
  window.localStorage = new LocalStorage(path.join(options.dataPath, 'localStorage'));

  return result;
};
browserPoly.fetch = fetch;
module.exports = browserPoly;
