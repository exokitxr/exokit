const nativeWorker = require('worker-native');
const GlobalContext = require('./GlobalContext');

GlobalContext.fakeVrDisplayEnabled = false; // XXX move this

const _makeWindow = (options = {}, parent = null, top = null) => {
  nativeWorker.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: {
      src,
    },
  });

  GlobalContext.windows.push(window);
  window.on('destroy', () => {
    GlobalContext.windows.splice(GlobalContext.windows.indexOf(window), 1);
  });

  return window;
};
module.exports._makeWindow = _makeWindow;
GlobalContext._makeWindow = _makeWindow;

const _makeWindowWithDocument = (s, options, parent, top) => { // XXX fold this into Window
  const window = _makeWindow(options, parent, top);
  window.document = _parseDocument(s, window);
  return window;
};
module.exports._makeWindowWithDocument = _makeWindowWithDocument;
