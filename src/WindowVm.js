const path = require('path');
const nativeWorker = require('worker-native');
const GlobalContext = require('./GlobalContext');
const {nativeWindow} = require('./native-bindings');

const _makeWindow = (options = {}) => {
  if (!GlobalContext.xrState.windowHandle[0] && !GlobalContext.xrState.windowHandle[1]) {
    GlobalContext.xrState.windowHandle.set(nativeWindow.createNull());
  }
  
  const window = nativeWorker.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: {
      options,
      xrState: GlobalContext.xrState,
    },
  });
  
  window.tickAnimationFrame = type => window.runAsync(`window.tickAnimationFrame("${type}");`);
  window.on('exit', () => {
    window.emit('destroy');
  });

  return window;
};
module.exports._makeWindow = _makeWindow;
GlobalContext._makeWindow = _makeWindow;
