const nativeWorker = require('worker-native');
const GlobalContext = require('./GlobalContext');

GlobalContext.fakeVrDisplayEnabled = false; // XXX move this

  nativeWorker.make({
const _makeWindow = (options = {}) => {
    initModule: path.join(__dirname, 'Window.js'),
    args: options,
  });

  GlobalContext.windows.push(window);
  window.on('destroy', () => {
    GlobalContext.windows.splice(GlobalContext.windows.indexOf(window), 1);
  });

  return window;
};
module.exports._makeWindow = _makeWindow;
GlobalContext._makeWindow = _makeWindow;

const _makeWindowWithDocument = (s, options) => { // XXX fold this into Window
  const window = _makeWindow(options);
  window.document = _parseDocument(s, window);
  return window;
};
module.exports._makeWindowWithDocument = _makeWindowWithDocument;
