#!/usr/bin/env node

if (require.main === module && !/^1[12]\./.test(process.versions.node)) {
  throw new Error('node 11 or 12 required');
}
// const cwd = process.cwd();
// process.chdir(__dirname); // needed for global bin to find libraries

const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const fs = require('fs');
const url = require('url');
const net = require('net');
const child_process = require('child_process');
const os = require('os');
const util = require('util');
const repl = require('repl');

const core = require('./core.js');
const mkdirp = require('mkdirp');
const replHistory = require('repl.history');
const minimist = require('minimist');

const {version} = require('../package.json');
const {defaultEyeSeparation, maxNumTrackers} = require('./constants.js');
const symbols = require('./symbols');
const {THREE} = core;

const nativeBindings = require(path.join(__dirname, 'native-bindings.js'));

const nativeWorker = require('worker-native');
nativeBindings.nativeWindow.setEventLoop(nativeWorker.getEventLoop());

const GlobalContext = require('./GlobalContext');
GlobalContext.args = {};
GlobalContext.version = '';
GlobalContext.commands = [];

const args = (() => {
  if (require.main === module) {
    const minimistArgs = minimist(process.argv.slice(2), {
      boolean: [
        'version',
        'home',
        'log',
        'perf',
        'performance',
        'frame',
        'minimalFrame',
        'tab',
        'quit',
        'blit',
        'headless',
      ],
      string: [
        'webgl',
        'xr',
        'size',
        'download',
        'replace',
      ],
      alias: {
        v: 'version',
        h: 'home',
        l: 'log',
        w: 'webgl',
        x: 'xr',
        p: 'performance',
        perf: 'performance',
        s: 'size',
        f: 'frame',
        m: 'minimalFrame',
        t: 'tab',
        q: 'quit',
        b: 'blit',
        r: 'replace',
        n: 'headless',
        d: 'download',
      },
    });
    return {
      version: minimistArgs.version,
      url: minimistArgs._[0] || '',
      home: minimistArgs.home,
      log: minimistArgs.log,
      webgl: minimistArgs.webgl || '2',
      xr: minimistArgs.xr || 'all',
      performance: !!minimistArgs.performance,
      size: minimistArgs.size,
      frame: minimistArgs.frame,
      minimalFrame: minimistArgs.minimalFrame,
      tab: minimistArgs.tab,
      quit: minimistArgs.quit,
      blit: minimistArgs.blit,
      replace: Array.isArray(minimistArgs.replace) ? minimistArgs.replace : ((minimistArgs.replace !== undefined) ? [minimistArgs.replace] : []),
      headless: minimistArgs.headless,
      download: minimistArgs.download !== undefined ? (minimistArgs.download || path.join(process.cwd(), 'downloads')) : undefined,
    };
  } else {
    return {};
  }
})();

core.setArgs(args);
core.setVersion(version);

const dataPath = (() => {
  const candidatePathPrefixes = [
    os.homedir(),
    __dirname,
    os.tmpdir(),
  ];
  for (let i = 0; i < candidatePathPrefixes.length; i++) {
    const candidatePathPrefix = candidatePathPrefixes[i];
    if (candidatePathPrefix) {
      const ok = (() => {
        try {
         fs.accessSync(candidatePathPrefix, fs.constants.W_OK);
         return true;
        } catch(err) {
          return false;
        }
      })();
      if (ok) {
        return path.join(candidatePathPrefix, '.exokit');
      }
    }
  }
  return null;
})();

const windows = [];
GlobalContext.windows = windows;
// const contexts = [];

const xrState = (() => {
  const _makeSab = size => {
    const sab = new SharedArrayBuffer(size);
    let index = 0;
    return (c, n) => {
      const result = new c(sab, index, n);
      index += result.byteLength;
      return result;
    };
  };
  const _makeTypedArray = _makeSab(1024);

  const result = {};
  
  result.windowHandle = _makeTypedArray(Uint32Array, 2);
  const windowHandle = nativeBindings.nativeWindow.createWindowHandle();
  result.windowHandle.set(windowHandle);
  nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);
  
  result.isPresenting = _makeTypedArray(Uint32Array, 1);
  result.renderWidth = _makeTypedArray(Float32Array, 1);
  result.renderHeight = _makeTypedArray(Float32Array, 1);
  result.depthNear = _makeTypedArray(Float32Array, 1);
  result.depthNear[0] = 0.1;
  result.depthFar = _makeTypedArray(Float32Array, 1);
  result.depthFar[0] = 10000.0;
  result.position = _makeTypedArray(Float32Array, 3);
  result.orientation = _makeTypedArray(Float32Array, 4);
  result.orientation[3] = 1;
  result.leftViewMatrix = _makeTypedArray(Float32Array, 16);
  result.leftViewMatrix.set(Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]));
  result.rightViewMatrix = _makeTypedArray(Float32Array, 16);
  result.rightViewMatrix.set(result.leftViewMatrix);
  result.leftProjectionMatrix = _makeTypedArray(Float32Array, 16);
  result.leftProjectionMatrix.set(Float32Array.from([
    0.8000000000000002, 0, 0, 0,
    0, 1.0000000000000002, 0, 0,
    0, 0, -1.002002002002002, -1,
    0, 0, -0.20020020020020018, 0,
  ]));
  result.rightProjectionMatrix = _makeTypedArray(Float32Array, 16);
  result.rightProjectionMatrix.set(result.leftProjectionMatrix);
  result.leftOffset = _makeTypedArray(Float32Array, 3);
  result.leftOffset.set(Float32Array.from([-defaultEyeSeparation/2, 0, 0]));
  result.rightOffset = _makeTypedArray(Float32Array, 3);
  result.leftOffset.set(Float32Array.from([defaultEyeSeparation/2, 0, 0]));
  result.leftFov = _makeTypedArray(Float32Array, 4);
  result.leftFov.set(Float32Array.from([45, 45, 45, 45]));
  result.rightFov = _makeTypedArray(Float32Array, 4);
  result.rightFov.set(result.leftFov);
  const _makeGamepad = () => ({
    connected: _makeTypedArray(Uint32Array, 1),
    position: _makeTypedArray(Float32Array, 3),
    orientation: (() => {
      const result = _makeTypedArray(Float32Array, 4);
      result[3] = 1;
      return result;
    })(),
    direction: (() => { // derived
      const result = _makeTypedArray(Float32Array, 4);
      result[2] = -1;
      return result;
    })(),
    transformMatrix: (() => { // derived
      const result = _makeTypedArray(Float32Array, 16);
      result.set(Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]));
      return result;
    })(),
    buttons: (() => {
      const result = Array(6);
      for (let i = 0; i < result.length; i++) {
        result[i] = {
          pressed: _makeTypedArray(Uint32Array, 1),
          touched: _makeTypedArray(Uint32Array, 1),
          value: _makeTypedArray(Float32Array, 1),
        };
      }
      return result;
    })(),
    axes: _makeTypedArray(Float32Array, 10),
  });
  result.gamepads = (() => {
    const result = Array(2 + maxNumTrackers);
    for (let i = 0; i < result.length; i++) {
      result[i] = _makeGamepad();
    }
    return result;
  })();
  result.id = _makeTypedArray(Uint32Array, 1);
  result.fakeVrDisplayEnabled = _makeTypedArray(Uint32Array, 1);
  
  return result;
})();
GlobalContext.xrState = xrState;
GlobalContext.fakeVrDisplayEnabled = false; // XXX globalize this

const topRequestContext = nativeWorker.makeRequestContext();
topRequestContext.setSyncHandler(m => {
  switch (m.method) {
    case 'runSync': {
      let result, err;
      try {
        global._ = m.arg;
        result = eval(m.jsString);
      } catch(e) {
        err = e;
      } finally {
        global._ = undefined;
      }
      
      if (!err) {
        return result;
      } else {
        throw err;
      }
    }
    default: {
      throw new Error(`top request context got unknown message type '${m.method}'`);
    }
  }
});
nativeWorker.setTopRequestContext(topRequestContext);

let innerWidth = 1280; // XXX do not track this globally
let innerHeight = 1024;

const _startTopRenderLoop = () => {
  const timestamps = {
    frames: 0,
    last: Date.now(),
    idle: 0,
    wait: 0,
    events: 0,
    media: 0,
    user: 0,
    submit: 0,
    total: 0,
  };
  const TIMESTAMP_FRAMES = 100;

  const _topRenderLoop = async () => {
    if (args.performance) {
      if (timestamps.frames >= TIMESTAMP_FRAMES) {
        console.log(`${(TIMESTAMP_FRAMES/(timestamps.total/1000)).toFixed(0)} FPS | ${timestamps.idle}ms idle | ${timestamps.wait}ms wait | ${timestamps.events}ms events | ${timestamps.media}ms media | ${timestamps.user}ms user | ${timestamps.submit}ms submit`);

        timestamps.frames = 0;
        timestamps.idle = 0;
        timestamps.wait = 0;
        timestamps.events = 0;
        timestamps.media = 0;
        timestamps.user = 0;
        timestamps.submit = 0;
        timestamps.total = 0;
      } else {
        timestamps.frames++;
      }
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.idle += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    await Promise.all(windows.map(window => window.tickAnimationFrame('wait')));

    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.wait += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    // poll operating system events
    nativeBindings.nativeWindow.pollEvents();
    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.events += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    // update media frames
    nativeBindings.nativeVideo.Video.updateAll();
    nativeBindings.nativeBrowser && nativeBindings.nativeBrowser.Browser.updateAll(); // XXX unlock when oculus mobile supports it

    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.media += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    if (args.frame || args.minimalFrame) {
      console.log('-'.repeat(80) + 'start frame');
    }

    // tick animation frames
    await Promise.all(windows.map(window => window.tickAnimationFrame('top')));
    
    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.user += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    await Promise.all(windows.map(window => window.tickAnimationFrame('submit')));

    // lastFrameTime = Date.now()

    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.submit += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    if (args.frame || args.minimalFrame) {
      console.log('-'.repeat(80) + 'end frame');
    }

    // wait for next frame
    immediate = setImmediate(_topRenderLoop);
  };
  let immediate = setImmediate(_topRenderLoop);

  return {
    stop() {
      clearImmediate(immediate);
      immediate = null;
    },
  };
};
_startTopRenderLoop();

const _bindWindow = (window, newWindowCb) => {
  window.innerWidth = innerWidth;
  window.innerHeight = innerHeight;

  // XXX move these to WindowVm/Window
  window.on('navigate', newWindowCb);
  /* window.document.on('paste', e => {
    e.clipboardData = new window.DataTransfer();
    const clipboardContents = nativeWindow.getClipboard().slice(0, 256);
    const dataTransferItem = new window.DataTransferItem('string', 'text/plain', clipboardContents);
    e.clipboardData.items.push(dataTransferItem);
  }); */
  /* if (args.quit) {
    window.document.resources.addEventListener('drain', () => {
      console.log('drain');
      process.exit();
    });
  } */
  
  window.on('destroy', e => {
    const {window} = e;
    for (let i = 0; i < contexts.length; i++) {
      const context = contexts[i];
      if (context.canvas.ownerDocument.defaultView === window) {
        context.destroy();
      }
    }
  });
  
  window.on('error', err => {
    console.warn('got error', err);
  });
};
const _bindDirectWindow = newWindow => {
  _bindWindow(newWindow, _bindDirectWindow);
};
core.load = (load => function() {
  return load.apply(this, arguments)
    .then(window => {
      _bindDirectWindow(window);

      return Promise.resolve(window);
    });
})(core.load);

const _prepare = () => Promise.all([
  (() => {
    if (!process.env['DISPLAY']) {
      process.env['DISPLAY'] = ':0.0';
    }
  })(),
  (() => {
    let rootPath = null;
    let runtimePath = null;
    const platform = os.platform();
    if (platform === 'win32') {
      rootPath = path.join(os.homedir(), 'AppData', 'Local', 'openvr');
      runtimePath = 'C:\\Program Files (x86)\\Steam\\steamapps\\common\\SteamVR';
    } else if (platform === 'darwin') {
      rootPath = path.join('/Users/', os.userInfo().username, '/Library/Application Support/OpenVR/.openvr');
      runtimePath = path.join(__dirname, '/node_modules/native-openvr-deps/bin/osx64');
    } else if (platform === 'linux') {
      rootPath = path.join(os.userInfo().homedir, '.config/openvr');
      runtimePath = path.join(__dirname, '..', 'node_modules', 'native-openvr-deps/bin/linux64');
    }

    if (rootPath !== null) {
      const openvrPathsPath = path.join(rootPath, 'openvrpaths.vrpath');

      return new Promise((accept, reject) => {
        fs.lstat(openvrPathsPath, (err, stats) => {
          if (err) {
            if (err.code === 'ENOENT') {
              mkdirp(rootPath, err => {
                if (!err) {
                  const jsonString = JSON.stringify({
                    "config" : [ rootPath ],
                    "external_drivers" : null,
                    "jsonid" : "vrpathreg",
                    "log" : [ rootPath ],
                    "runtime" : [
                       runtimePath,
                     ],
                    "version" : 1
                  }, null, 2);
                  fs.writeFile(openvrPathsPath, jsonString, err => {
                    if (!err) {
                      accept();
                    } else {
                      reject(err);
                    }
                  });
                } else if (err.code === 'EACCES') {
                  accept();
                } else {
                  reject(err);
                }
              });
            } else if (err.code === 'EACCES') {
              accept();
            } else {
              reject(err);
            }
          } else {
            accept();
          }
        });
      });
    } else {
      return Promise.resolve();
    }
  })(),
  new Promise((accept, reject) => {
    mkdirp(dataPath, err => {
      if (!err) {
        accept();
      } else {
        reject(err);
      }
    });
  }),
]);

const realityTabsUrl = 'file://' + path.join(__dirname, '..', 'examples', 'realitytabs.html');
const _start = () => {
  let {url: u} = args;
  if (!u && args.home) {
    u = realityTabsUrl;
  }
  if (u) {
    if (u === '.') {
      console.warn('NOTE: You ran `exokit . <url>`\n(Did you mean to run `node . <url>` or `exokit <url>` instead?)')
    }
    u = u.replace(/^exokit:/, '');
    if (args.tab) {
      u = `${realityTabsUrl}?t=${encodeURIComponent(u)}`
    }
    if (u && !url.parse(u).protocol) {
      u = 'file://' + path.resolve(process.cwd(), u);
    }
    const replacements = (() => {
      const result = {};
      for (let i = 0; i < args.replace.length; i++) {
        const replaceArg = args.replace[i];
        const replace = replaceArg.split(' ');
        if (replace.length === 2) {
          result[replace[0]] = 'file://' + path.resolve(process.cwd(), replace[1]);
        } else {
          console.warn(`invalid replace argument: ${replaceArg}`);
        }
      }
      return result;
    })();
    return core.load(u, {
      dataPath,
      args,
      replacements,
    });
  } else {
    let window = null;
    const _bindReplWindow = newWindow => {
      _bindWindow(newWindow, _bindReplWindow);
      window = newWindow;
    };
    _bindReplWindow(core('', {
      dataPath,
    }));

    const prompt = '[x] ';

    const replEval = async (cmd, context, filename, callback) => {
      cmd = cmd.slice(0, -1); // remove trailing \n

      let result, err;
      let match;

      if (/^[a-z]+:\/\//.test(cmd)) {
        cmd = `window.location.href = ${JSON.stringify(cmd)};`;
      } else if (/^\s*<(?:\!\-*)?[a-z]/i.test(cmd)) {
        cmd = `(() => {
          const e = window.document.createElement('div');
          e.innerHTML = ${JSON.stringify(cmd)};
          if (e.childNodes.length === 0) {
            return window._ = undefined;
          } else if (e.childNodes.length === 1) {
            return window._ = e.childNodes[0];
          } else {
            return window._ = e.childNodes;
          }
        })();`;
      } else if (match = cmd.match(/^\s*(?:const|var|let)?\s*([a-z][a-z0-9]*)\s*=\s*(<(?:\!\-*)?[a-z].*)$/im)) {
        const name = match[1];
        const src = match[2];
        cmd = `(() => {
          const name = ${JSON.stringify(name)};
          const e = window.document.createElement('div');
          e.innerHTML = ${JSON.stringify(src)};
          if (e.childNodes.length === 0) {
            return window[name] = window._ = undefined;
          } else if (e.childNodes.length === 1) {
            return window[name] = window._ = e.childNodes[0];
          } else {
            return window[name] = window._ = e.childNodes;
          }
        })();`;
      }
      try {
        result = await window.runRepl(cmd);
      } catch(e) {
        err = e;
      }

      if (!err) {
        if (result !== undefined) {
          r.setPrompt(prompt);
        }
      } else {
        if (err.name === 'SyntaxError') {
          err = new repl.Recoverable(err);
        }
      }

      GlobalContext.commands.push(cmd);

      callback(err, {[util.inspect.custom]() { return result; }});
    };
    const r = repl.start({
      prompt,
      eval: replEval,
    });
    replHistory(r, path.join(dataPath, '.repl_history'));
    r.on('exit', () => {
      process.exit();
    });
  }
};

if (require.main === module) {
  if (!nativeBindings.nativePlatform) { // not a mobile platform
    require(path.join(__dirname, 'bugsnag'));
    require('fault-zone').registerHandler((stack, stackLen) => {
      const message = new Buffer(stack, 0, stackLen).toString('utf8');
      console.warn(message);
      child_process.execFileSync(process.argv[0], [
        path.join(__dirname, 'bugsnag.js'),
      ], {
        input: message,
      });
      process.exit(1);
    });
  }
  if (args.log) {
    const RedirectOutput = require('redirect-output').default;
    new RedirectOutput({
      flags: 'a',
    }).write(path.join(dataPath, 'log.txt'));
  }

  const _logStack = err => {
    console.warn(err);
  };
  process.on('uncaughtException', _logStack);
  process.on('unhandledRejection', _logStack);
  EventEmitter.defaultMaxListeners = 100;

  if (args.version) {
    console.log(version);
    process.exit(0);
  }
  if (args.size) {
    const match = args.size.match(/^([0-9]+)x([0-9]+)$/);
    if (match) {
      const w = parseInt(match[1], 10);
      const h = parseInt(match[2], 10);
      if (w > 0 && h > 0) {
        innerWidth = w;
        innerHeight = h;
      }
    }
  }
  if (args.frame || args.minimalFrame) {
    bindings.nativeGl = (OldWebGLRenderingContext => {
      function WebGLRenderingContext() {
        const result = Reflect.construct(OldWebGLRenderingContext, arguments);
        for (const k in result) {
          if (typeof result[k] === 'function') {
            result[k] = (old => function() {
              if (GlobalContext.args.frame) {
                console.log(k, arguments);
              } else if (GlobalContext.args.minimalFrame) {
                console.log(k);
              }
              return old.apply(this, arguments);
            })(result[k]);
          }
        }
        return result;
      }
      for (const k in OldWebGLRenderingContext) {
        WebGLRenderingContext[k] = OldWebGLRenderingContext[k];
      }
      return WebGLRenderingContext;
    })(bindings.nativeGl);
  }

  _prepare()
    .then(() => _start())
    .catch(err => {
      console.warn(err.stack);
      process.exit(1);
    });
}

module.exports = core;
