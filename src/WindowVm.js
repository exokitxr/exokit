const nativeWorker = require('worker-native');
const GlobalContext = require('./GlobalContext');

const _makeWindow = (options = {}) => {
  const window = nativeWorker.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: options,
  });
  
  window.on('exit', () => {
    window.emit('destroy');
  });
  
  window.tickAnimationFrame = () => {
    window.runDetached(`window.tickAnimationFrame();`);
  };

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
