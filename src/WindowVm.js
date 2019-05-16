const {EventEmitter} = require('events');
const path = require('path');
const {isMainThread, parentPort, Worker} = require('worker_threads');
const GlobalContext = require('./GlobalContext');
const {process} = global;

const windowBasePath = path.join(__dirname, 'WindowBase.js');
class WorkerVm extends EventEmitter {
  constructor(options = {}) {
    super();

    const worker = new Worker(windowBasePath, {
      workerData: {
        initModule: options.initModule,
        args: options.args,
      },
    });
    worker.on('message', m => {
      switch (m.method) {
        case 'runAsync': {
          if (isMainThread) {
            const _respond = (error, result) => {
              const targetWindowId = m.windowIdPath[0];
              const targetWindow = GlobalContext.windows.find(window => window.id === targetWindowId);

              if (targetWindow) {
                targetWindow.runAsync({
                  method: 'response',
                  result,
                  error,
                  windowIdPath: m.windowIdPath.slice(1),
                  requestKey: m.requestKey,
                });
              } else {
                console.warn(`unknown response window id: ${targetWindowId}`);
              }
            };

            switch (m.request.method) {
              case 'requestPresent': {
                GlobalContext.requestPresent()
                  .then(({hmdType}) => {
                    console.log('top request present 2');
                    _respond(null, {hmdType});
                  })
                  .catch(err => {
                    console.warn(err.stack);
                  });
                break;
              }
              case 'exitPresent': {
                GlobalContext.exitPresent()
                  .then(() => {
                    _respond(null, null);
                  })
                  .catch(err => {
                    console.warn(err.stack);
                  });
                break;
              }
              case 'requestHitTest': {
                const {request} = m;
                const {origin, direction, coordinateSystem} = request;
                GlobalContext.requestHitTest(origin, direction, coordinateSystem)
                  .then(result => {
                    _respond(null, result);
                  })
                  .catch(err => {
                    console.warn(err.stack);
                  });
                break;
              }
              default: {
                console.warn(`unknown runAsync request method: ${m.request.method}`);
                break;
              }
            }
          } else {
            parentPort.postMessage({
              method: 'runAsync',
              request: m.request,
              windowIdPath: [GlobalContext.id].concat(m.windowIdPath),
              requestKey: m.requestKey,
            });
          }
          break;
        }
        case 'response': {
          const fn = this.queue[m.requestKey];

          if (fn) {
            fn(m.error, m.result);
            delete this.queue[m.requestKey];
          } else {
            console.warn(`unknown response request key: ${m.requestKey}`);
          }
          break;
        }
        case 'postMessage': {
          this.emit('message', m);
          break;
        }
        case 'emit': {
          this.emit(m.type, m.event);
          break;
        }
        default: {
          throw new Error(`worker got unknown message type '${m.method}'`);
          break;
        }
      }
    });
    worker.on('error', err => {
      this.emit('error', err);
    });
    worker.on('exit', () => {
      this.emit('exit');
    });

    this.worker = worker;
    this.requestKeys = 0;
    this.queue = {};
  }

  queueRequest(fn) {
    const requestKey = this.requestKeys++;
    this.queue[requestKey] = fn;
    return requestKey;
  }

  runRepl(jsString, transferList) {
    return new Promise((accept, reject) => {
      const requestKey = this.queueRequest((err, result) => {
        if (!err) {
          accept(result);
        } else {
          reject(err);
        }
      });
      this.worker.postMessage({
        method: 'runRepl',
        jsString,
        requestKey,
      }, transferList);
    });
  }
  runAsync(request, transferList) {
    return new Promise((accept, reject) => {
      const requestKey = this.queueRequest((err, result) => {
        if (!err) {
          accept(result);
        } else {
          reject(err);
        }
      });
      this.worker.postMessage({
        method: 'runAsync',
        request,
        requestKey,
      }, transferList);
    });
  }
  postMessage(message, transferList) {
    this.worker.postMessage({
      method: 'postMessage',
      message,
    }, transferList);
  }

  destroy() {
    const symbols = Object.getOwnPropertySymbols(this.worker);
    const publicPortSymbol = symbols.find(s => s.toString() === 'Symbol(kPublicPort)');
    const publicPort = this.worker[publicPortSymbol];
    publicPort.close();
  }

  get onmessage() {
    return this.listeners('message')[0];
  }
  set onmessage(onmessage) {
    this.on('message', onmessage);
  }

  get onerror() {
    return this.listeners('error')[0];
  }
  set onerror(onerror) {
    this.on('error', onerror);
  }

  get onexit() {
    return this.listeners('exit')[0];
  }
  set onexit(onexit) {
    this.on('exit', onexit);
  }
}
module.exports.WorkerVm = WorkerVm;

const _exitWindow = o => {
  window.destroy = (destroy => function() {
    GlobalContext.windows.splice(GlobalContext.windows.indexOf(window), 1);
    for (const k in window.queue) {
      window.queue[k]();
    }

    return new Promise((accept, reject) => {
      window.on('exit', () => {
        accept();
      });

      destroy.apply(this, arguments);
    });
  })(window.destroy);
  return 0;
};
const _clean = o => {
  const result = {};
  for (const k in o) {
    const v = o[k];
    if (typeof v !== 'function') {
      result[k] = v;
    }
  }
  return result;
};
const _makeWindow = (options = {}, handlers = {}) => {
  const id = Atomics.add(GlobalContext.xrState.id, 0, 1) + 1;
  const window = new WorkerVm({
    initModule: path.join(__dirname, 'Window.js'),
    args: {
      options: _clean(options),
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

  window.evalAsync = scriptString => window.runAsync({method: 'eval', scriptString});

  window.on('resize', ({width, height}) => {
    // console.log('got resize', width, height);
    window.width = width;
    window.height = height;
  });
  window.on('framebuffer', framebuffer => {
    // console.log('got framebuffer', framebuffer);
    window.document.framebuffer = framebuffer;
  });
  window.on('navigate', ({href}) => {
    window.destroy()
      .then(() => {
        options.onnavigate && options.onnavigate(href);
      })
      .catch(err => {
        console.warn(err.stack);
      });
  });
  window.on('error', err => {
    console.warn(err.stack);
  });
  window.destroy = (destroy => function() {
    GlobalContext.windows.splice(GlobalContext.windows.indexOf(window), 1);
    for (const k in window.queue) {
      window.queue[k]();
    }

    return new Promise((accept, reject) => {
      window.on('exit', () => {
        accept();
      });

      destroy.apply(this, arguments);
    });
  })(window.destroy);

  GlobalContext.windows.push(window);

  return window;
};
module.exports._makeWindow = _makeWindow;
module.exports._exitWindow = _exitWindow;
