const path = require('path');
const nativeWorker = require('worker-native');
const {nativeWindow} = require('./native-bindings');
const GlobalContext = require('./GlobalContext');

const _makeWindow = (options = {}) => {
  const id = Atomics.add(GlobalContext.xrState.id, 0, 1) + 1;
  const window = nativeWorker.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: {
      options,
      id,
      args: GlobalContext.args,
      version: GlobalContext.version,
      xrState: GlobalContext.xrState,
    },
  });
  window.id = id;
  window.tickAnimationFrame = type => window.runAsync(`window.tickAnimationFrame("${type}");`);
  window.promise = null;
  window.syncs = null;
  
  window.on('resize', ({width, height}) => {
    window.width = width;
    window.height = height;
  });
  window.on('framebuffer', framebuffer => {
    window.document.framebuffer = framebuffer;
  });
  window.on('error', err => {
    console.warn(err.stack);
  });
  /* window.on('exit', () => {
    window.emit('destroy');
  }); */
  window.destroy = (destroy => function() {
    destroy.apply(this, arguments);
    
    window.emit('destroy');
    
    GlobalContext.windows.splice(GlobalContext.windows.indexOf(window), 1);
  })(window.destroy);
  
  GlobalContext.windows.push(window);

  return window;
};
module.exports._makeWindow = _makeWindow;
