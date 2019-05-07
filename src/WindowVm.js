const {EventEmitter} = require('events');
const path = require('path');
const {Worker} = require('worker_threads');
const GlobalContext = require('./GlobalContext');

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
        case 'response': {
          const fn = this.queue[m.requestKey];

          if (fn) {
            fn(m.error, m.result);
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
      console.log('worker.on("error") TKTK', err);
      this.emit('error', err);
      console.log('worker.on("error") TKTK done', err);
    });
    worker.on('exit', () => {
      console.log('worker.on("exit") TKTK');
      this.emit('exit');
      console.log('worker.on("exit") TKTK done');
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
  runAsync(jsString, arg, transferList) {
    this.unresolveds = this.unresolveds || {};
    this.unresolvedsId = this.unresolvedsId || 0;
    const id = this.unresolvedsId++;
    console.log('TKTK runAsync', jsString, arg);
    return new Promise((accept, reject) => {
      this.unresolveds[id] = {accept, reject};
      console.log('TKTK runAsync 1', this.worker ? 'this.worker' : 'this.worker is NULL!!');
      const requestKey = this.queueRequest((err, result) => {
        console.log('TKTK response', err, result);
        if (!err) {
          delete this.unresolveds[id];
          accept(result);
        } else {
          delete this.unresolveds[id];
          reject(err);
        }
      });
      console.log('TKTK runAsync 2');
      this.worker.postMessage({
        method: 'runAsync',
        jsString,
        arg,
        requestKey,
      }, transferList);
      console.log('TKTK runAsync 3');
    });
  }
  postMessage(message, transferList) {
    this.worker.postMessage({
      method: 'postMessage',
      message,
    }, transferList);
  }
  
  destroy() {
    return new Promise((accept, reject) => {
      console.log('worker terminating', this.unresolveds);
      this.worker.terminate((err, exitCode) => {
        console.log('worker terminated', err, exitCode);
        let unresolveds = this.unresolveds;
        this.unresolveds = {};
        for (let {accept, reject} of Object.values(unresolveds)) {
          accept('terminated');
        }
        console.log('worker terminated and rejected unresolveds');
        accept();
      });
    });
  }
    /*
    console.log('publcPort destroy 1');
    const symbols = Object.getOwnPropertySymbols(this.worker);
    console.log('publcPort destroy 2');
    const publicPortSymbol = symbols.find(s => s.toString() === 'Symbol(kPublicPort)');
    console.log('publcPort destroy 3',publicPortSymbol);
    const publicPort = this.gorker[publicPortSymbol];
    console.log('publcPort destroy 4');
    publicPort.close();
    console.log('publcPort destroy 5');
  }
    */

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

const _makeWindow = (options = {}) => {
  const id = Atomics.add(GlobalContext.xrState.id, 0, 1) + 1;
  const window = new WorkerVm({
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
      console.log('destroy 1');
      window.on('exit', () => {
        console.log('destroy 5');
        accept();
        console.log('destroy 6');
      });
      console.log('destroy 2');
      destroy.apply(this, arguments).then(() => {
        console.log('destroy 3');
        accept();
        console.log('destroy 4');
      }).catch(() => {
        console.log('destroy reject');
        reject();
      });
    });
  })(window.destroy);
  
  GlobalContext.windows.push(window);

  return window;
};
module.exports._makeWindow = _makeWindow;
