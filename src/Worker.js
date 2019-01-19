(() => {

const path = require('path');
const fs = require('fs');
const url = require('url');
const {URL} = url;
const vm = require('vm');
const {workerData} = require('worker_threads');
const {args} = workerData;

const {createImageBitmap} = require('./DOM.js');
const fetch = require('window-fetch');
const {XMLHttpRequest} = require('xmlhttprequest');
const WebSocket = require('ws/lib/websocket');
const {FileReader} = require('./File.js');

const {src} = args;
const baseUrl = (src => {
  if (/^https?:/.test(src)) {
    const u = new URL(src);
    u.pathname = path.dirname(u.pathname) + '/';
    return u.href;
  } else {
    return 'file://' + process.cwd();
  }
})(src);
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
const filename = _normalizeUrl(src);

global.self = global;
global.addEventListener = global.on;
global.removeEventListener = global.removeListener; 
global.location = url.parse(filename);
global.fetch = (s, options) => fetch(_normalizeUrl(s), options);
global.XMLHttpRequest = XMLHttpRequest;
global.WebSocket = WebSocket;
global.importScripts = importScripts;
global.postMessage = postMessage;
global.createImageBitmap = createImageBitmap;
global.FileReader = FileReader;

global.on('message', m => {
  if (typeof global.onmessage === 'function') {
    global.onmessage(m);
  }
});
global.on('error', err => {
  if (typeof global.onerror === 'function') {
    global.onerror(err);
  }
});

const _handleError = err => {
  if (onerror) {
    onerror(err);
  }
};
process.on('uncaughtException', _handleError);
process.on('unhandledRejection', _handleError);

const exp = getScript(filename);
vm.runInThisContext(exp, {
  filename: /^https?:/.test(filename) ? filename : 'data-url://',
});

})();
