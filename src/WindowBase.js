const {Console} = require('console');
const {EventEmitter} = require('events');
const stream = require('stream');
const path = require('path');
const fs = require('fs');
const url = require('url');
const {URL} = url;
const vm = require('vm');
const util = require('util');
const crypto = require('crypto');
const {
  Worker: WorkerBase,
  workerData: {
    initModule,
    args,
  },
  parentPort,
} = require('worker_threads');

const {CustomEvent, DragEvent, ErrorEvent, Event, EventTarget, KeyboardEvent, MessageEvent, MouseEvent, WheelEvent, PromiseRejectionEvent} = require('./Event');
const {FileReader} = require('./File.js');
const {XMLHttpRequest, FormData} = require('window-xhr');
const fetch = require('window-fetch');
const {Request, Response, Headers, Blob} = fetch;
const WebSocket = require('ws/lib/websocket');

const {WorkerVm} = require('./WindowVm');
const GlobalContext = require('./GlobalContext');

const utils = require('./utils');

const btoa = s => Buffer.from(s, 'binary').toString('base64');
const atob = s => Buffer.from(s, 'base64').toString('binary');

const {
  nativeConsole,
  nativeCache,
} = require('./native-bindings');
const {process} = global;

GlobalContext.xrState = args.xrState;

const consoleStream = new stream.Writable();
consoleStream._write = (chunk, encoding, callback) => {
  nativeConsole.Log(chunk);
  callback();
};
consoleStream._writev = (chunks, callback) => {
  for (let i = 0; i < chunks.length; i++) {
    nativeConsole.Log(chunks[i]);
  }
  callback();
};
global.console = new Console(consoleStream);

URL.createObjectURL = blob => {
  const url = 'blob:' + GlobalContext.xrState.blobId[0]++;
  nativeCache.set(url, blob.buffer);
  return url;
};
URL.revokeObjectURL = url => {
  nativeCache.delete(url);
};
URL.lookupObjectURL = url => {
  const uint8Array = nativeCache.get(url);
  return uint8Array && new Blob([uint8Array], {
    type: 'application/octet-stream', // TODO: make this the correct type
  });
}

// global initialization

for (const k in EventEmitter.prototype) {
  global[k] = EventEmitter.prototype[k];
}
EventEmitter.call(global);

class Worker extends EventTarget {
  constructor(src) {
    super();

    if (src instanceof Blob) {
      src = 'data:application/javascript,' + src.buffer.toString('utf8');
    } else {
      const blob = URL.lookupObjectURL(src);
      src = blob ?
        'data:application/octet-stream;base64,' + blob.buffer.toString('base64')
      :
        _normalizeUrl(src);
    }

    const worker = new WorkerVm({
      initModule: path.join(__dirname, 'Worker.js'),
      args: {
        src,
        baseUrl: utils._getBaseUrl(src, baseUrl),
        args,
        xrState: args.xrState,
      },
    });
    worker.on('message', m => {
      const e = new MessageEvent('message', {
        data: m.message,
      });
      this.emit('message', e);
    });
    worker.on('error', err => {
      this.emit('error', err);
    });
    this.worker = worker;
  }

  postMessage(message, transferList) {
    this.worker.postMessage(message, transferList);
  }

  terminate() {
    this.worker.destroy();
  }

  get onmessage() {
    return this.listeners('message')[0];
  }
  set onmessage(onmessage) {
    this.on('message', onmessage);
  }
  get onerror() {
    return this.listeners('error')[0];
  }
  set onerror(onerror) {
    this.on('error', onerror);
  }
}

(self => {
  self.btoa = btoa;
  self.atob = atob;

  self.Event = Event;
  self.KeyboardEvent = KeyboardEvent;
  self.MouseEvent = MouseEvent;
  self.WheelEvent = WheelEvent;
  self.DragEvent = DragEvent;
  self.MessageEvent = MessageEvent;
  self.PromiseRejectionEvent = PromiseRejectionEvent;
  self.CustomEvent = CustomEvent;
  self.EventTarget = EventTarget;

  self.URL = URL;

  self.fetch = (u, options) => {
    if (typeof u === 'string') {
      const blob = URL.lookupObjectURL(u);
      if (blob) {
        return Promise.resolve(new Response(blob));
      } else {
        u = _normalizeUrl(u);
        return fetch(u, options);
      }
    } else {
      return fetch(u, options);
    }
  };
  self.Request = Request;
  self.Response = Response;
  self.Headers = Headers;
  self.Blob = Blob;
  self.FormData = FormData;
  self.XMLHttpRequest = (Old => {
    class XMLHttpRequest extends Old {
      open(method, url, async, username, password) {
        const blob = URL.lookupObjectURL(url);
        if (blob) {
          return super.open(method, blob, async, username, password);
        } else {
          url = _normalizeUrl(url);
          return super.open(method, url, async, username, password);
        }
      }
      get response() {
        if (this.responseType === 'blob') {
          return new Blob(super.response, {
            type: this.getResponseHeader('content-type') || 'application/octet-stream',
          });
        } else {
          return super.response;
        }
      }
    }
    for (const k in Old) {
      XMLHttpRequest[k] = Old[k];
    }
    return XMLHttpRequest;
  })(XMLHttpRequest);
  self.WebSocket = WebSocket;
  self.FileReader = FileReader;
  self.crypto = {
    getRandomValues(typedArray) {
      crypto.randomFillSync(Buffer.from(typedArray.buffer, typedArray.byteOffset, typedArray.byteLength));
      return typedArray;
    },

    subtle: {
      digest(algo, bytes) {
        switch (algo) {
          case 'SHA-1': {
            algo = 'sha1';
            break;
          }
          case 'SHA-256': {
            algo = 'sha256';
            break;
          }
          case 'SHA-384': {
            algo = 'sha384';
            break;
          }
          case 'SHA-512': {
            algo = 'sha512';
            break;
          }
          default: throw new Error(`unknown algorithm: ${algo}`);
        }
        const hash = crypto.createHash(algo).update(bytes).digest();
        const result = new ArrayBuffer(hash.byteLength);
        new Buffer(result).set(hash);
        return Promise.resolve(result);
      },
    },
  };

  self.Worker = Worker;

  self.postMessage = (message, transferList) => parentPort.postMessage({
    method: 'postMessage',
    message,
  }, transferList);
  Object.defineProperty(self, 'onmessage', {
    get() {
      return this.listeners('message')[0];
    },
    set(onmessage) {
      self.on('message', onmessage);
    },
  });
})(global);

let baseUrl = '';
function setBaseUrl(newBaseUrl) {
  baseUrl = newBaseUrl;
}
global.setBaseUrl = setBaseUrl;
const _normalizeUrl = src => utils._normalizeUrl(src, baseUrl);

const SYNC_REQUEST_BUFFER_SIZE = 5 * 1024 * 1024; // TODO: we can make this unlimited with a streaming buffer + atomics loop
function getScript(url) {
  let match;
  if (match = url.match(/^data:.+?(;base64)?,(.*)$/)) {
    if (match[1]) {
      return Buffer.from(match[2], 'base64').toString('utf8');
    } else {
      return match[2];
    }
  } else if (match = url.match(/^file:\/\/(.*)$/)) {
    return fs.readFileSync(match[1], 'utf8');
  } else {
    const sab = new SharedArrayBuffer(Int32Array.BYTES_PER_ELEMENT*3 + SYNC_REQUEST_BUFFER_SIZE);
    const int32Array = new Int32Array(sab);
    const worker = new WorkerBase(path.join(__dirname, 'request.js'), {
      workerData: {
        url: _normalizeUrl(url),
        int32Array,
      },
    });
    worker.on('error', err => {
      console.warn(err.stack);
    });
    Atomics.wait(int32Array, 0, 0);
    const status = new Uint32Array(sab, Int32Array.BYTES_PER_ELEMENT*1, 1)[0];
    const length = new Uint32Array(sab, Int32Array.BYTES_PER_ELEMENT*2, 1)[0];
    const result = Buffer.from(sab, Int32Array.BYTES_PER_ELEMENT*3, length).toString('utf8');
    if (status >= 200 && status < 300) {
      return result;
    } else {
      throw new Error(`fetch ${url} failed (${JSON.stringify(status)}): ${result}`);
    }
  }
}
function importScripts() {
  for (let i = 0; i < arguments.length; i++) {
    const importScriptPath = arguments[i];
    const importScriptSource = getScript(importScriptPath);
    vm.runInThisContext(importScriptSource, global, {
      filename: /^https?:/.test(importScriptPath) ? importScriptPath : 'data-url://',
    });
  }
}
global.importScripts = importScripts;

parentPort.on('message', m => {
  switch (m.method) {
    case 'runRepl': {
      let result, err;
      try {
        result = util.inspect(eval(m.jsString));
      } catch(e) {
        err = e.stack;
      }
      parentPort.postMessage({
        method: 'response',
        requestKey: m.requestKey,
        result,
        error: err,
      });
      break;
    }
    case 'runAsync': {
      let result, err;
      try {
        result = global.onrunasync ? global.onrunasync(m.request) : null;
      } catch(e) {
        err = e.stack;
      }
      if (!err) {
        Promise.resolve(result)
          .then(result => {
            parentPort.postMessage({
              method: 'response',
              requestKey: m.requestKey,
              result,
            });
          });
      } else {
        parentPort.postMessage({
          method: 'response',
          requestKey: m.requestKey,
          error: err,
        });
      }
      break;
    }
    case 'postMessage': {
      try {
        const e = new MessageEvent('messge', {
          data: m.message,
        });
        global.emit('message', e);
      } catch(err) {
        console.warn(err.stack);
      }
      break;
    }
    default: throw new Error(`invalid method: ${JSON.stringify(m.method)}`);
  }
});

function close() {
  global.onexit && global.onexit();
  process.exit(); // thread exit
};
global.close = close;
parentPort.on('close', close);

// run init module

/* if (workerData.args) {
  global.args = workerData.args;
} */

process.on('uncaughtException', err => {
  console.warn('uncaught exception:', (err && err.stack) || err);
});
process.on('unhandledRejection', err => {
  console.warn('unhandled rejection:', (err && err.stack) || err);
});

if (initModule) {
  require(initModule);
}

// Run a script for each new JS context before the page/worker JS loads
const onbeforeload = args.args.onbeforeload;
if (onbeforeload) {
  const finalScript = path.resolve(process.cwd(), onbeforeload);
  require(finalScript);
}

if (!args.require) {
  global.require = undefined;
}
global.process = undefined;
