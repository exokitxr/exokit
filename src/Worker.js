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
  if (/^(?:https?|file):/.test(src)) {
    const u = new URL(src);
    u.pathname = path.dirname(u.pathname) + '/';
    return u.href;
  } else {
    return 'file://' + process.cwd();
  }
})(src);
setBaseUrl(baseUrl);
const _normalizeUrl = src => {
  if (!/^(?:file|data|blob):/.test(src)) {
    return new URL(src, baseUrl).href;
  } else {
    return src;
  }
};
const filename = _normalizeUrl(src);

global.self = global;
global.addEventListener = (type, fn) => global.on(type, fn);
global.removeEventListener = (type, fn) => global.removeListener(type, fn);
global.location = url.parse(filename);
global.fetch = (s, options) => fetch(_normalizeUrl(s), options);
global.XMLHttpRequest = XMLHttpRequest;
global.WebSocket = WebSocket;
global.importScripts = importScripts;
global.createImageBitmap = createImageBitmap;
global.FileReader = FileReader;

global.on('error', err => {
  const {onerror} = global;
  onerror && onerror(err);
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
process.nexTick(() => { // importScripts will block, so make sure we are done setup first
  importScripts(filename);
});
