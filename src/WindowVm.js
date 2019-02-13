const nativeWorker = require('worker-native');
const GlobalContext = require('./GlobalContext');

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
  
  window.tickAnimationFrame = () => window.runAsync(`return window.tickAnimationFrame();`);
  window.oninternalmessage = async m => {
    switch (m.type) {
      case 'postRequestAsync': {
        const handler = requestHandlers[m.method];
        let result, error;
        if (handler) {
          try {
            result = await handler(args);
          } catch(err) {
            error = err;
          }
        } else {
          error = new Error(`method not found: ${m.method}`);
        }
        window.postInternalMessage({
          type: 'postRequestAsync',
          id: m.id,
          result,
          error,
        });
        break;
      }
    }
  };
  window.on('exit', () => {
    window.emit('destroy');
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
