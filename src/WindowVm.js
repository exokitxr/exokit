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
  window.phase = 0; // XXX
  // window.rendered = false;
  window.promise = null;
  // window.syncs = null;

  window.evalAsync = scriptString => window.runAsync(JSON.stringify({method: 'eval', scriptString}));

  window.on('resize', ({width, height}) => {
    console.log('got resize', width, height);
    window.width = width;
    window.height = height;
  });
  window.on('framebuffer', framebuffer => {
    console.log('got framebuffer', framebuffer);
    window.document.framebuffer = framebuffer;
  });
  window.on('error', err => {
    console.warn(err.stack);
  });
  window.destroy = (destroy => function() {
    GlobalContext.windows.splice(GlobalContext.windows.indexOf(window), 1);

    return new Promise((accept, reject) => {
      destroy.apply(this, arguments);

      window.on('exit', () => {
        accept();
      });
    });
  })(window.destroy);
  
  GlobalContext.windows.push(window);

  return window;
};
module.exports._makeWindow = _makeWindow;
