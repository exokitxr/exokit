const {Console} = require('console');
const {EventEmitter} = require('events');
const stream = require('stream');
const path = require('path');
const fs = require('fs');
const vm = require('vm');
const util = require('util');
const {Worker, workerData, parentPort} = require('worker_threads');
const {MessageEvent} = require('./Event');
const {
  nativeConsole,
} = require('./native-bindings');
const {process} = global;

// global initialization

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

for (const k in EventEmitter.prototype) {
  global[k] = EventEmitter.prototype[k];
}
EventEmitter.call(global);

global.postMessage = (message, transferList) => parentPort.postMessage({
  method: 'postMessage',
  message,
}, transferList);
Object.defineProperty(global, 'onmessage', {
  get() {
    return this.listeners('message')[0];
  },
  set(onmessage) {
    global.on('message', onmessage);
  },
});

global.windowEmit = (type, event, transferList) => parentPort.postMessage({
  method: 'emit',
  type,
  event,
}, transferList);

let baseUrl = '';
function setBaseUrl(newBaseUrl) {
  baseUrl = newBaseUrl;
}
global.setBaseUrl = setBaseUrl;

const _normalizeUrl = src => {
  if (!/^(?:data|blob):/.test(src)) {
    const match = baseUrl.match(/^(file:\/\/)(.*)$/);
    if (match) {
      return match[1] + path.join(match[2], src);
    } else {
      return new URL(src, baseUrl).href;
    }
  } else {
    return src;
  }
};
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
    const sab = new SharedArrayBuffer(Int32Array.BYTES_PER_ELEMENT*2 + 5 * 1024 * 1024);
    const int32Array = new Int32Array(sab);
    const worker = new Worker(path.join(__dirname, 'request.js'), {
      workerData: {
        url: _normalizeUrl(url),
        int32Array,
      },
    });
    worker.on('error', err => {
      console.warn(err.stack);
    });
    Atomics.wait(int32Array, 0, 0);
    const status = new Uint32Array(sab, 0, 1)[0];
    const length = new Uint32Array(sab, Int32Array.BYTES_PER_ELEMENT, 1)[0];
    const result = Buffer.from(sab, Int32Array.BYTES_PER_ELEMENT*2, length).toString('utf8');
    if (status === 1) {
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
        result = window.onrunasync ? window.onrunasync(m.jsString) : null;
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

if (workerData.args) {
  global.args = workerData.args;
}

process.on('uncaughtException', err => {
  console.warn(err.stack);
});
process.on('unhandledRejection', err => {
  console.warn(err.stack);
});

if (workerData.initModule) {
  require(workerData.initModule);
}

if (!workerData.args.require) {
  global.require = undefined;
}
global.process = undefined;