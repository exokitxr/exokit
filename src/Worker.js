(() => {

const path = require('path');
const fs = require('fs');
const url = require('url');
const {URL} = url;
const vm = require('vm');
const {workerData: {args}} = require('worker_threads');

const {createImageBitmap} = require('./DOM.js');
const fetch = require('window-fetch');
const {XMLHttpRequest} = require('window-xhr');
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
setBaseUrl(baseUrl);
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

const _mapMessageType = type => {
  if (type === 'message') {
    return 'clientMessage';
  }
  return type;
};

global.self = global;
global.addEventListener = (type, fn) => global.on(_mapMessageType(type), fn);
global.removeEventListener = (type, fn) => global.removeListener(_mapMessageType(type), fn); 
global.location = url.parse(filename);
global.fetch = (s, options) => fetch(_normalizeUrl(s), options);
global.XMLHttpRequest = XMLHttpRequest;
global.WebSocket = WebSocket;
global.importScripts = importScripts;
global.postMessage = (oldPostMessage => (message, transferList) => oldPostMessage({
  data: message,
}, transferList))(postMessage);
global.createImageBitmap = createImageBitmap;
global.FileReader = FileReader;

global.on('message', m => {
  global.emit('clientmessage', {
    data: m,
  });
});
global.on('clientmessage', m => {
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
  if (global.onerror) {
    global.onerror(err);
  }
};
process.on('uncaughtException', _handleError);
process.on('unhandledRejection', _handleError);

/* const exp = getScript(filename);
vm.runInThisContext(exp, {
  filename: /^https?:/.test(filename) ? filename : 'data-url://',
}); */
importScripts(filename);

})();
