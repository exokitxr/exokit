const nativeWorker = require('worker-native');
const GlobalContext = require('./GlobalContext');

const _makeWindow = (options = {}) => {
  const window = nativeWorker.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: options,
  });
  
  window.tickAnimationFrame = () => {
    window.runDetached(`window.tickAnimationFrame();`);
  };
  {
    const requestHandlers = {};
    window.setRequestHandler = (method, handler) => {
      requestHandlers[method] = handler;
    };
    window.oninternalmessage = async m => {
      switch (m.type) {
        case 'postRequestAsync': {
          const handler = requestHandlers[m.method];
          if (handler) {
            let result, error;
            try {
              result = await handler(args);
            } catch(err) {
              error = err;
            }
            window,postInternalMessage({
              type: 'postRequestAsync',
              id: m.id,
              result,
              error,
            });
          }
          break;
        }
      }
    };
  }
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
