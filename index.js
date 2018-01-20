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

const {LocalStorage} = require('node-localstorage');

const utils = require('jsdom/lib/jsdom/living/generated/utils');

const jsdom = require('jsdom');
const browserPoly = (s = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1';
  options.contentType = 'text/html';
  options.runScripts = 'dangerously';
  options.dataPath = options.dataPath || __dirname;
  options.beforeParse = window => {
    const basePath = options.url;
    const _normalizeUrl = url => {
      if (!/^.+?:/.test(url)) {
        url = basePath + ((!/\/$/.test(basePath) && !/^\//.test(url)) ? '/' : '') + url;
      }
      return url;
    };

    // synchronize script tag onload
    window.document[utils.implSymbol]._queue = {
      resume: () => {},
      push: callback => (err, data, response) => {
        // process.nextTick(() => {
          callback(err, data, response);
        // });
      },
    };

    window.fetch = (fetch => (url, options) => fetch(_normalizeUrl(url), options))(window.fetch);

    class Worker {
      constructor(src, options) {
        this.src = src;
        this.options = options;

        this.live = true;
        this.worker = null;
        this.queue = [];

        this.start();
      }

      start() {
        const {src, options} = this;

        if (typeof src === 'string') {
          fetch(_normalizeUrl(src))
            .then(res => {
              if (res.status >= 200 && res.status < 300) {
                return res.text();
              } else {
                return Promise.reject(new Error('worker fetch got invalid status code: ' + res.status));
              }
            })
            .then(codeString => {
              if (this.live) {
                this.worker = new TinyWorker(new Function(codeString), options);

                for (let i = 0; i < this.queue.length; i++) {
                  this.queue[i]();
                }
                this.queue.length = 0;
              }
            })
            .catch(err => {
              if (this.live) {
                console.warn(err);
              }
            });
        } else if (src instanceof Blob) {
          this.worker = new TinyWorker(new Function(src[Blob.BUFFER].toString('utf8')), options);
        } else {
          this.worker = new TinyWorker(src, options);
        }
      }

      addEventListener() {
        if (this.worker) {
          this.worker.addEventListener.apply(this.worker, arguments);
        } else {
          this.queue.push(() => {
            this.addEventListener.apply(this, arguments);
          });
        }
      }

      postMessage() {
        if (this.worker) {
          this.worker.postMessage.apply(this.worker, arguments);
        } else {
          this.queue.push(() => {
            this.postMessage.apply(this, arguments);
          });
        }
      }

      terminate() {
        this.live = false;

        if (this.worker) {
          this.worker.terminate();
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
