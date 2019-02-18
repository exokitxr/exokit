const path = require('path');
const nativeWorker = require('worker-native');
const {nativeWindow} = require('./native-bindings');
const GlobalContext = require('./GlobalContext');

const _makeWindow = (options = {}) => {
  const window = nativeWorker.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: {
      options,
      args: GlobalContext.args,
      version: GlobalContext.version,
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
