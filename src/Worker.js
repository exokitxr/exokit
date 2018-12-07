(() => {

const path = require('path');
const fs = require('fs');
const url = require('url');
const {URL} = url;
const vm = require('vm');
const fetch = require('window-fetch');
const {XMLHttpRequest} = require('xmlhttprequest');
const WebSocket = require('ws/lib/websocket');
const {Worker, parentPort, workerData} = require('worker_threads');

const baseUrl = (src => {
  if (/^https?:/.test(src)) {
    const u = new URL(src);
    u.pathname = path.dirname(u.pathname) + '/';
    return u.href;
  } else {
    return 'file://' + process.cwd();
  }
})(initMessage.data.src);
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

global.self = global;

(() => {
  let onmessage = null;
  Object.defineProperty('onmessage', {
    get() {
      return onmessage;
    },
    set(fn) {
      onmessage = fn;
    },
  });
})();
(() => {
  let onerror = null;
  Object.defineProperty('onerror', {
    get() {
      return onerror;
    },
    set(fn) {
      onerror = fn;
    },
  });
})();
global.addEventListener = (event, fn) => {
  if (event === 'message') {
    onmessage = fn;
  }
  if (event === 'error') {
    onerror = fn;
  }
};
global.removeEventListener = (event, fn) => {
  if (event === 'message' && onmessage === fn) {
    onmessage = null;
  }
  if (event === 'error' && onerror === fn) {
    onerror = null;
  }
};
global.location = url.parse(filename);
global.fetch = (s, options) => fetch(_normalizeUrl(s), options);
global.XMLHttpRequest = XMLHttpRequest;
global.WebSocket = WebSocket;
global.importScripts = importScripts;
global.postMessage = postMessage;
global.createImageBitmap = createImageBitmap;
global.FileReader = FileReader;

const _handleError = err => {
  if (onerror) {
    onerror(err);
  }
};
process.on('uncaughtException', _handleError);
process.on('unhandledRejection', _handleError);
const filename = _normalizeUrl(initMessage.data.src);
const exp = getScript(filename);

vm.runInThisContext(exp, {
  filename: /^https?:/.test(filename) ? filename : 'data-url://',
});

})();
