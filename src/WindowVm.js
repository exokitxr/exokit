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
      stdin: true,
      stdout: true,
      stderr: true,
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
        case 'navigate': {
          // XXX hook this
          // this.emit(m.type, m.event);
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
    /* setTimeout(() => {
      worker.stdin.write('lol\n');
    }, 2000); */
    process.stdin.pipe(worker.stdin);
    worker.stdout.pipe(process.stdout);
    worker.stderr.pipe(process.stderr);

    /* worker.stdout.setEncoding('utf8');
    worker.stdout.on('data', d => {
      console.log(JSON.stringify(d));
    }); */

    this.worker = worker;
    this.requestKeys = 0;
    this.queue = {};
  }

  queueRequest(fn) {
    const requestKey = this.requestKeys++;
    this.queue[requestKey] = fn;
    return requestKey;
  }

  /* runRepl(jsString, transferList) {
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
  } */
  runAsync(jsString, arg, transferList) {
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
        jsString,
        arg,
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

    return Promise.resolve(); // XXX
    /* return new Promise((accept, reject) => {
      destroy.apply(this, arguments);

      window.on('exit', () => {
        accept();
      });
    }); */
  })(window.destroy);
  
  GlobalContext.windows.push(window);

  return window;
};
module.exports._makeWindow = _makeWindow;
