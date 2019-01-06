#!/usr/bin/env node

const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const fs = require('fs');
const url = require('url');
const net = require('net');
const child_process = require('child_process');
const os = require('os');
const repl = require('repl');

const core = require('./core.js');
const mkdirp = require('mkdirp');
const replHistory = require('repl.history');
const minimist = require('minimist');
const UPNG = require('upng-js');

const {version} = require('../package.json');
const symbols = require('./symbols');
const {THREE} = core;

const nativeBindingsModulePath = path.join(__dirname, 'native-bindings.js');
const nativeBindings = require(nativeBindingsModulePath);

const GlobalContext = require('./GlobalContext');
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
        'quit',
        'blit',
        'uncapped',
        'require',
        'headless',
      ],
      string: [
        'tab',
        'webgl',
        'xr',
        'size',
        'image',
        'download',
      ],
      alias: {
        v: 'version',
        h: 'home',
        l: 'log',
        t: 'tab',
        w: 'webgl',
        x: 'xr',
        p: 'performance',
        perf: 'performance',
        s: 'size',
        f: 'frame',
        m: 'minimalFrame',
        q: 'quit',
        b: 'blit',
        u: 'uncapped',
        i: 'image',
        r: 'require',
        n: 'headless',
        d: 'download',
      },
    });
    return {
      version: minimistArgs.version,
      url: minimistArgs._[0] || '',
      home: minimistArgs.home || !!minimistArgs.tab,
      log: minimistArgs.log,
      tab: minimistArgs.tab,
      webgl: minimistArgs.webgl || '2',
      xr: minimistArgs.xr || 'all',
      performance: !!minimistArgs.performance,
      size: minimistArgs.size,
      frame: minimistArgs.frame,
      minimalFrame: minimistArgs.minimalFrame,
      quit: minimistArgs.quit,
      blit: minimistArgs.blit,
      uncapped: minimistArgs.uncapped,
      image: minimistArgs.image,
      require: minimistArgs.require,
      headless: minimistArgs.headless,
      download: minimistArgs.download,
    };
  } else {
    return {};
  }
})();

core.setArgs(args);
core.setVersion(version);
core.setNativeBindingsModule(nativeBindingsModulePath);

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
const DEFAULT_FPS = 60; // TODO: Use different FPS for device.requestAnimationFrame vs window.requestAnimationFrame
const VR_FPS = 90;
const ML_FPS = 60;
const MLSDK_PORT = 17955;

const contexts = [];
GlobalContext.contexts = contexts;
const _windowHandleEquals = (a, b) => a[0] === b[0] && a[1] === b[1];

let _takeScreenshot = false;

nativeBindings.nativeGl.onconstruct = (gl, canvas) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  gl.canvas = canvas;

  const document = canvas.ownerDocument;
  const window = document.defaultView;

  const {nativeWindow} = nativeBindings;
  const windowSpec = (() => {
    if (!window[symbols.optionsSymbol].args.headless) {
      try {
        const visible = !args.image && document.documentElement.contains(canvas);
        const {hidden} = document;
        const firstWindowHandle = contexts.length > 0 ? contexts[0].getWindowHandle() : null;
        return nativeWindow.create3d(canvasWidth, canvasHeight, visible && !hidden, firstWindowHandle, gl);
      } catch (err) {
        console.warn(err.message);
        return null;
      }
    } else {
      return null;
    }
  })();

  if (windowSpec) {
    const [windowHandle, sharedFramebuffer, sharedColorTexture, sharedDepthStencilTexture, sharedMsFramebuffer, sharedMsColorTexture, sharedMsDepthStencilTexture, vao] = windowSpec;

    gl.setWindowHandle(windowHandle);
    gl.setDefaultVao(vao);

    const nativeWindowSize = nativeWindow.getFramebufferSize(windowHandle);
    const nativeWindowHeight = nativeWindowSize.height;
    const nativeWindowWidth = nativeWindowSize.width;

    // Calculate devicePixelRatio.
    window.devicePixelRatio = nativeWindowWidth / canvasWidth;

    // Tell DOM how large the window is.
    window.innerHeight = nativeWindowHeight / window.devicePixelRatio;
    window.innerWidth = nativeWindowWidth / window.devicePixelRatio;

    const title = `Exokit ${version}`;
    nativeWindow.setWindowTitle(windowHandle, title);

    const {hidden} = document;
    if (hidden) {
      const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeWindow.createRenderTarget(gl, canvasWidth, canvasHeight, sharedColorTexture, sharedDepthStencilTexture, sharedMsColorTexture, sharedMsDepthStencilTexture);

      gl.setDefaultFramebuffer(msFbo);

      gl.resize = (width, height) => {
        nativeWindow.setCurrentWindowContext(windowHandle);
        nativeWindow.resizeRenderTarget(gl, width, height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
      };

      // TODO: handle multiple child canvases
      document.framebuffer = {
        canvas,
        msFbo,
        msTex,
        msDepthTex,
        fbo,
        tex,
        depthTex,
      };
    } else {
      gl.resize = (width, height) => {
        nativeWindow.setCurrentWindowContext(windowHandle);
        nativeWindow.resizeRenderTarget(gl, width, height, sharedFramebuffer, sharedColorTexture, sharedDepthStencilTexture, sharedMsFramebuffer, sharedMsColorTexture, sharedMsDepthStencilTexture);
      };
    }

    const ondomchange = () => {
      process.nextTick(() => { // show/hide synchronously emits events
        if (!hidden) {
          const domVisible = canvas.ownerDocument.documentElement.contains(canvas);
          const windowVisible = nativeWindow.isVisible(windowHandle);
          if (domVisible) {
            if (!windowVisible) {
              nativeWindow.show(windowHandle);
            }
          } else {
            if (windowVisible) {
              nativeWindow.hide(windowHandle);
            }
          }
        }
      });
    };
    canvas.ownerDocument.on('domchange', ondomchange);

    gl.destroy = (destroy => function() {
      destroy.call(this);

      nativeWindow.setCurrentWindowContext(windowHandle);

      if (gl === vrPresentState.glContext) {
        nativeBindings.nativeVr.VR_Shutdown();

        vrPresentState.glContext = null;
        vrPresentState.system = null;
        vrPresentState.compositor = null;
      }
      if (gl === mlPresentState.mlGlContext) {
        mlPresentState.mlGlContext = null;
      }

      nativeWindow.destroy(windowHandle);
      canvas._context = null;

      if (hidden) {
        document._emit('framebuffer', null);
      }
      canvas.ownerDocument.removeListener('domchange', ondomchange);

      contexts.splice(contexts.indexOf(gl), 1);

      if (!contexts.some(context => nativeWindow.isVisible(context.getWindowHandle()))) { // no more windows
        process.exit();
      }
    })(gl.destroy);

    canvas.ownerDocument.defaultView.on('unload', () => {
      gl.destroy();
    });
  } else {
    gl.destroy();
  }

  contexts.push(gl);
  fps = nativeWindow.getRefreshRate();
};

nativeBindings.nativeCanvasRenderingContext2D.onconstruct = (ctx, canvas) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  ctx.canvas = canvas;

  const window = canvas.ownerDocument.defaultView;

  const {nativeWindow} = nativeBindings;
  const windowSpec = (() => {
    if (!window[symbols.optionsSymbol].args.headless) {
      try {
        const firstWindowHandle = contexts.length > 0 ? contexts[0].getWindowHandle() : null;
        return nativeWindow.create2d(canvasWidth, canvasHeight, firstWindowHandle);
      } catch (err) {
        console.warn(err.message);
        return null;
      }
    } else {
      return null;
    }
  })();

  if (windowSpec) {
    const [windowHandle, tex] = windowSpec;

    ctx.setWindowHandle(windowHandle);
    ctx.setTexture(tex, canvasWidth, canvasHeight);
    
    ctx.destroy = (destroy => function() {
      destroy.call(this);
      
      nativeWindow.destroy(windowHandle);
      canvas._context = null;
      
      contexts.splice(contexts.indexOf(ctx), 1);
    })(ctx.destroy);
  } else {
    ctx.destroy();
  }

  contexts.push(ctx);
};

const zeroMatrix = new THREE.Matrix4();
const localFloat32Array = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array2 = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array3 = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array4 = new Float32Array(16);
const localOffsetArray = new Float32Array(3);
const localOffsetArray2 = new Float32Array(3);
const localFovArray = new Float32Array(4);
const localFovArray2 = new Float32Array(4);
const localGamepadArray = new Float32Array(24);

const handEntrySize = (1 + (5 * 5)) * (3 + 3);
const transformArray = new Float32Array(7 * 2);
const projectionArray = new Float32Array(16 * 2);
const handsArray = [
  new Float32Array(handEntrySize),
  new Float32Array(handEntrySize),
];
const controllersArray = new Float32Array((3 + 4 + 6) * 2);

const localVector = new THREE.Vector3();
const localVector2 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();
const _normalizeMatrixArray = float32Array => {
  if (isNaN(float32Array[0])) {
    zeroMatrix.toArray(float32Array);
  }
};

const vrPresentState = {
  vrContext: null,
  isPresenting: false,
  system: null,
  compositor: null,
  glContext: null,
  msFbo: null,
  msTex: null,
  msDepthTex: null,
  fbo: null,
  tex: null,
  depthTex: null,
  cleanups: null,
  hasPose: false,
  lmContext: null,
  layers: [],
};
GlobalContext.vrPresentState = vrPresentState;
let renderWidth = 0;
let renderHeight = 0;
const depthNear = 0.1;
const depthFar = 10000.0;
if (nativeBindings.nativeVr) {
  nativeBindings.nativeVr.requestPresent = function(layers) {
    const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
    if (layer) {
      const canvas = layer.source;
      
      if (!vrPresentState.glContext) {
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }
        const window = canvas.ownerDocument.defaultView;

        const windowHandle = context.getWindowHandle();
        nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);

        fps = VR_FPS;

        const vrContext = vrPresentState.vrContext || nativeBindings.nativeVr.getContext();
        const system = vrPresentState.system || nativeBindings.nativeVr.VR_Init(nativeBindings.nativeVr.EVRApplicationType.Scene);
        const compositor = vrPresentState.compositor || vrContext.compositor.NewCompositor();

        const lmContext = vrPresentState.lmContext || (nativeBindings.nativeLm && new nativeBindings.nativeLm());

        const {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2;
        renderWidth = halfWidth;
        renderHeight = height;

        const cleanups = [];

        const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeBindings.nativeWindow.createRenderTarget(context, width, height, 0, 0, 0, 0);

        context.setDefaultFramebuffer(msFbo);

        vrPresentState.isPresenting = true;
        vrPresentState.vrContext = vrContext;
        vrPresentState.system = system;
        vrPresentState.compositor = compositor;
        vrPresentState.glContext = context;
        vrPresentState.msFbo = msFbo;
        vrPresentState.msTex = msTex;
        vrPresentState.msDepthTex = msDepthTex;
        vrPresentState.fbo = fbo;
        vrPresentState.tex = tex;
        vrPresentState.depthTex = depthTex;
        vrPresentState.cleanups = cleanups;

        vrPresentState.lmContext = lmContext;

        canvas.framebuffer = {
          width,
          height,
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };

        const _attribute = (name, value) => {
          if (name === 'width' || name === 'height') {
            nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);

            nativeBindings.nativeWindow.resizeRenderTarget(context, canvas.width, canvas.height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
          }
        };
        canvas.on('attribute', _attribute);
        cleanups.push(() => {
          canvas.removeListener('attribute', _attribute);
        });

        window.top.updateVrFrame({
          renderWidth,
          renderHeight,
          force: true,
        });

        return canvas.framebuffer;
      } else if (canvas.ownerDocument.framebuffer) {
        const {width, height} = canvas;
        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = canvas.ownerDocument.framebuffer;
        return {
          width,
          height,
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };
      } else {
        /* const {width: halfWidth, height} = vrPresentState.system.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2; */

        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = vrPresentState;
        return {
          width: renderWidth * 2,
          height: renderHeight,
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };
      }
    } else {
      throw new Error('no HTMLCanvasElement source provided');
    }
  };
  nativeBindings.nativeVr.exitPresent = function() {
    if (vrPresentState.isPresenting) {
      nativeBindings.nativeVr.VR_Shutdown();

      nativeBindings.nativeWindow.destroyRenderTarget(vrPresentState.msFbo, vrPresentState.msTex, vrPresentState.msDepthStencilTex);
      nativeBindings.nativeWindow.destroyRenderTarget(vrPresentState.fbo, vrPresentState.tex, vrPresentState.msDepthTex);

      const context = vrPresentState.glContext;
      nativeBindings.nativeWindow.setCurrentWindowContext(context.getWindowHandle());
      context.setDefaultFramebuffer(0);

      for (let i = 0; i < vrPresentState.cleanups.length; i++) {
        vrPresentState.cleanups[i]();
      }

      vrPresentState.isPresenting = false;
      vrPresentState.system = null;
      vrPresentState.compositor = null;
      vrPresentState.glContext = null;
      vrPresentState.msFbo = null;
      vrPresentState.msTex = null;
      vrPresentState.msDepthTex = null;
      vrPresentState.fbo = null;
      vrPresentState.tex = null;
      vrPresentState.depthTex = null;
      vrPresentState.cleanups = null;
    }

    return Promise.resolve();
  };
}
const mlPresentState = {
  mlContext: null,
  mlFbo: null,
  mlTex: null,
  mlDepthTex: null,
  mlMsFbo: null,
  mlMsTex: null,
  mlMsDepthTex: null,
  mlGlContext: null,
  mlCleanups: null,
  mlHasPose: false,
  layers: [],
};
GlobalContext.mlPresentState = mlPresentState;
if (nativeBindings.nativeMl) {
  mlPresentState.mlContext = new nativeBindings.nativeMl();
  nativeBindings.nativeMl.requestPresent = function(layers) {
    const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
    if (layer) {
      const canvas = layer.source;
      
      if (!mlPresentState.mlGlContext) {
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }
        const window = canvas.ownerDocument.defaultView;

        const windowHandle = context.getWindowHandle();
        nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);

        fps = ML_FPS;

        const initResult = mlPresentState.mlContext.Present(windowHandle, context);
        if (initResult) {
          const {
            width: halfWidth,
            height,
            fbo,
            colorTex: tex,
            depthStencilTex: depthTex,
            msFbo,
            msColorTex: msTex,
            msDepthStencilTex: msDepthTex,
          } = initResult;
          const width = halfWidth * 2;
          renderWidth = halfWidth;
          renderHeight = height;

          mlPresentState.mlFbo = fbo;
          mlPresentState.mlTex = tex;
          mlPresentState.mlDepthTex = depthTex;
          mlPresentState.mlMsFbo = msFbo;
          mlPresentState.mlMsTex = msTex;
          mlPresentState.mlMsDepthTex = msDepthTex;

          canvas.framebuffer = {
            width,
            height,
            msFbo,
            msTex,
            msDepthTex,
            fbo,
            tex,
            depthTex,
          };

          const cleanups = [];
          mlPresentState.mlCleanups = cleanups;

          const _attribute = (name, value) => {
            if (name === 'width' || name === 'height') {
              nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);

              nativeBindings.nativeWindow.resizeRenderTarget(context, canvas.width, canvas.height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
            }
          };
          canvas.on('attribute', _attribute);
          cleanups.push(() => {
            canvas.removeListener('attribute', _attribute);
          });

          window.top.updateVrFrame({
            renderWidth,
            renderHeight,
            force: true,
          });

          context.setDefaultFramebuffer(msFbo);

          mlPresentState.mlGlContext = context;

          return canvas.framebuffer;
        } else {
          throw new Error('failed to present ml context');
        }
      } else if (canvas.ownerDocument.framebuffer) {
        const {width, height} = canvas;
        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = canvas.ownerDocument.framebuffer;
        
        return {
          width,
          height,
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };
      } else {
        return {
          width: renderWidth * 2,
          height: renderHeight,
          msFbo: mlPresentState.mlMsFbo,
          msTex: mlPresentState.mlMsTex,
          msDepthTex: mlPresentState.mlMsDepthTex,
          fbo: mlPresentState.mlFbo,
          tex: mlPresentState.mlTex,
          depthTex: mlPresentState.mlDepthTex,
        };
      }
    } else {
      throw new Error('no HTMLCanvasElement source provided');
    }
  };
  nativeBindings.nativeMl.exitPresent = function() {
    nativeBindings.nativeWindow.destroyRenderTarget(mlPresentState.mlMsFbo, mlPresentState.mlMsTex, mlPresentState.mlMsDepthTex);
    nativeBindings.nativeWindow.destroyRenderTarget(mlPresentState.mlFbo, mlPresentState.mlTex, mlPresentState.mlDepthTex);

    nativeBindings.nativeWindow.setCurrentWindowContext(mlPresentState.mlGlContext.getWindowHandle());
    mlPresentState.mlGlContext.setDefaultFramebuffer(0);

    for (let i = 0; i < mlPresentState.mlCleanups.length; i++) {
      mlPresentState.mlCleanups[i]();
    }

    mlPresentState.mlFbo = null;
    mlPresentState.mlTex = null;
    mlPresentState.mlDepthTex = null;
    mlPresentState.mlMsFbo = null;
    mlPresentState.mlMsTex = null;
    mlPresentState.mlMsDepthTex = null;
    mlPresentState.mlGlContext = null;
    mlPresentState.mlCleanups = null;
    mlPresentState.mlHasPose = false;
  };

  const _mlLifecycleEvent = e => {
    console.log('got ml lifecycle event', e);

    switch (e) {
      case 'newInitArg': {
        break;
      }
      case 'stop':
      case 'pause': {
        if (mlPresentState.mlContext) {
          mlPresentState.mlContext.Exit();
        }
        nativeBindings.nativeMl.DeinitLifecycle();
        process.exit();
        break;
      }
      case 'resume': {
        break;
      }
      case 'unloadResources': {
        break;
      }
      default: {
        console.warn('invalid ml lifecycle event', e);
        break;
      }
    }
  };
  const _mlKeyboardEvent = e => {
    // console.log('got ml keyboard event', e);

    if (mlPresentState.mlGlContext) {
      const {canvas} = mlPresentState.mlGlContext;
      const window = canvas.ownerDocument.defaultView;

      switch (e.type) {
        case 'keydown': {
          let handled = false;
          if (e.keyCode === 27) { // ESC
            if (window.top.document.pointerLockElement) {
              window.top.document.exitPointerLock();
              handled = true;
            }
            if (window.top.document.fullscreenElement) {
              window.top.document.exitFullscreen();
              handled = true;
            }
          }
          if (e.keyCode === 122) { // F11
            if (window.top.document.fullscreenElement) {
              window.top.document.exitFullscreen();
              handled = true;
            } else {
              window.top.document.requestFullscreen();
              handled = true;
            }
          }

          if (!handled) {
            canvas.dispatchEvent(new window.KeyboardEvent(e.type, e));
          }
          break;
        }
        case 'keyup':
        case 'keypress': {
          canvas.dispatchEvent(new window.KeyboardEvent(e.type, e));
          break;
        }
        default:
          break;
      }
    }
  };
  if (!nativeBindings.nativeMl.IsSimulated()) {
    nativeBindings.nativeMl.InitLifecycle(_mlLifecycleEvent, _mlKeyboardEvent);
  } else {
    // try to connect to MLSDK
    const s = net.connect(MLSDK_PORT, '127.0.0.1', () => {
      s.destroy();

      nativeBindings.nativeMl.InitLifecycle(_mlLifecycleEvent, _mlKeyboardEvent);
    });
    s.on('error', () => {});
  }
}

const fakePresentState = {
  layers: [],
};
GlobalContext.fakePresentState = fakePresentState;

nativeBindings.nativeWindow.setEventHandler((type, data) => {
  const {windowHandle} = data;
  const context = contexts.find(context => _windowHandleEquals(context.getWindowHandle(), windowHandle));
  const {canvas} = context;
  const window = canvas.ownerDocument.defaultView;

  if (context) {
    switch (type) {
      case 'focus': {
        const {focused} = data;
        if (!focused && window.top.document.pointerLockElement) {
          window.top.document.exitPointerLock();
        }
        break;
      }
      case 'framebufferResize': {
        const {width, height} = data;
        innerWidth = width;
        innerHeight = height;

        window.innerWidth = innerWidth / window.devicePixelRatio;
        window.innerHeight = innerHeight / window.devicePixelRatio;
        window.dispatchEvent(new window.Event('resize'));
        break;
      }
      case 'keydown': {
        let handled = false;
        if (data.keyCode === 27) { // ESC
          if (window.top.document.pointerLockElement) {
            window.top.document.exitPointerLock();
            handled = true;
          }
          if (window.top.document.fullscreenElement) {
            window.top.document.exitFullscreen();
            handled = true;
          }
        }
        if (data.keyCode === 122) { // F11
          if (window.top.document.fullscreenElement) {
            window.top.document.exitFullscreen();
            handled = true;
          } else {
            window.top.document.requestFullscreen();
            handled = true;
          }
        }

        if (!handled) {
          canvas.dispatchEvent(new window.KeyboardEvent(type, data));
        }
        break;
      }
      case 'keyup':
      case 'keypress': {
        canvas.dispatchEvent(new window.KeyboardEvent(type, data));
        break;
      }
      case 'mousedown':
      case 'mouseup':
      case 'click': {
        canvas.dispatchEvent(new window.MouseEvent(type, data));
        break;
      }
      case 'wheel': {
        canvas.dispatchEvent(new window.WheelEvent(type, data));
        break;
      }
      case 'mousemove': {
        canvas.dispatchEvent(new window.MouseEvent(type, data));
        break;
      }
      case 'drop': {
        const _readFiles = paths => {
          const result = [];

          return Promise.all(paths.map(p =>
            new Promise((accept, reject) => {
              fs.lstat(p, (err, stats) => {
                if (!err) {
                  if (stats.isFile()) {
                    fs.readFile(p, (err, data) => {
                      if (!err) {
                        const file = new window.Blob([data]);
                        file.name = path.basename(p);
                        file.path = p;
                        result.push(file);

                        accept();
                      } else {
                        reject(err);
                      }
                    });
                  } else if (stats.isDirectory()) {
                    fs.readdir(p, (err, fileNames) => {
                      if (!err) {
                        _readFiles(fileNames.map(fileName => path.join(p, fileName)))
                          .then(files => {
                            result.push.apply(result, files);

                            accept();
                          })
                          .catch(err => {
                            reject(err);
                          });
                      } else {
                        reject(err);
                      }
                    });
                  } else {
                    accept();
                  }
                } else {
                  reject(err);
                }
              });
            })
          ))
            .then(() => result);
        };

        _readFiles(data.paths)
          .then(files => {
            const dataTransfer = new window.DataTransfer({
              files,
            });
            const e = new window.DragEvent('drop');
            e.dataTransfer = dataTransfer;
            canvas.dispatchEvent(e);
          })
          .catch(err => {
            console.warn(err.stack);
          });
        break;
      }
      case 'quit': {
        context.destroy();
        break;
      }
    }
  } else {
    console.warn('got native window event with no matching context', {type, data});
  }
});

let innerWidth = 1280; // XXX do not track this globally
let innerHeight = 1024;
let fps = DEFAULT_FPS;
const _getFrameTimeMax = () => ~~(1000 / fps);
const _getFrameTimeMin = () => 0;
const isMac = os.platform() === 'darwin';

const _bindWindow = (window, newWindowCb) => {
  window.innerWidth = innerWidth;
  window.innerHeight = innerHeight;

  window.on('unload', () => {
    clearTimeout(timeout);
  });
  window.on('navigate', newWindowCb);
  window.document.on('paste', e => {
    e.clipboardData = new window.DataTransfer();
    const clipboardContents = nativeWindow.getClipboard().slice(0, 256);
    const dataTransferItem = new window.DataTransferItem('string', 'text/plain', clipboardContents);
    e.clipboardData.items.push(dataTransferItem);
  });
  window.on('vrdisplaypresentchange', e => {
    if (e.display) {
      const gamepads = [leftGamepad, rightGamepad];
      for (let i = 0; i < gamepads.length; i++) {
        gamepads[i].ontriggerhapticpulse = (value, duration) => {
          if (vrPresentState.isPresenting) {
            value = Math.min(Math.max(value, 0), 1);
            const deviceIndex = vrPresentState.system.GetTrackedDeviceIndexForControllerRole(i + 1);

            const startTime = Date.now();
            const _recurse = () => {
              if ((Date.now() - startTime) < duration) {
                vrPresentState.system.TriggerHapticPulse(deviceIndex, 0, value * 4000);
                setTimeout(_recurse, 50);
              }
            };
            setTimeout(_recurse, 50);
          }
        };
      }
    } else {
      const gamepads = [leftGamepad, rightGamepad];
      for (let i = 0; i < gamepads.length; i++) {
        gamepads[i].ontriggerhapticpulse = null
      }
    }
  });
  window.document.addEventListener('pointerlockchange', () => {
    const {pointerLockElement} = window.document;

    if (pointerLockElement) {
      for (let i = 0; i < contexts.length; i++) {
        const context = contexts[i];

        if (context.canvas.ownerDocument.defaultView === window) {
          const windowHandle = context.getWindowHandle();

          if (nativeBindings.nativeWindow.isVisible(windowHandle)) {
            nativeBindings.nativeWindow.setCursorMode(windowHandle, false);
            break;
          }
        }
      }
    } else {
      for (let i = 0; i < contexts.length; i++) {
        const context = contexts[i];

        if (context.canvas.ownerDocument.defaultView === window) {
          const windowHandle = context.getWindowHandle();

          if (nativeBindings.nativeWindow.isVisible(windowHandle)) {
            nativeBindings.nativeWindow.setCursorMode(windowHandle, true);
            break;
          }
        }
      }
    }
  });
  window.document.addEventListener('fullscreenchange', () => {
    const {fullscreenElement} = window.document;

    if (fullscreenElement) {
      for (let i = 0; i < contexts.length; i++) {
        const context = contexts[i];

        if (context.canvas.ownerDocument.defaultView === window) {
          const windowHandle = context.getWindowHandle();

          if (nativeBindings.nativeWindow.isVisible(windowHandle)) {
            nativeBindings.nativeWindow.setFullscreen(windowHandle);
            break;
          }
        }
      }
    } else {
      for (let i = 0; i < contexts.length; i++) {
        const context = contexts[i];

        if (context.canvas.ownerDocument.defaultView === window) {
          const windowHandle = context.getWindowHandle();

          if (nativeBindings.nativeWindow.isVisible(windowHandle)) {
            nativeBindings.nativeWindow.exitFullscreen(windowHandle);
            break;
          }
        }
      }
    }
  });
  if (args.quit) {
    window.document.resources.addEventListener('drain', () => {
      console.log('drain');
      process.exit();
    });
  }
  window.addEventListener('destroy', e => {
    const {window} = e;
    for (let i = 0; i < contexts.length; i++) {
      const context = contexts[i];
      if (context.canvas.ownerDocument.defaultView === window) {
        context.destroy();
      }
    }
    clearTimeout(timeout);
  });
  window.addEventListener('error', err => {
    console.warn('got error', err);
  });

  const _saveImage = (context, name) => {
    const _flipImage = (width, height, stride, arrayBuffer) => {
      const uint8Array = new Uint8Array(arrayBuffer);

      const arrayBuffer2 = new ArrayBuffer(arrayBuffer.byteLength);
      const uint8Array2 = new Uint8Array(arrayBuffer2);
      for (let y = 0; y < height; y++) {
        const yBottom = height - y - 1;
        uint8Array2.set(uint8Array.slice(yBottom * width * stride, (yBottom + 1) * width * stride), y * width * stride);
      }
      console.log(uint8Array2[0], uint8Array2[1], uint8Array2[2], uint8Array2[3]);
      return arrayBuffer2;
    };

    const gl = context;
    const {canvas} = gl;
    const {width, height} = canvas;

    const arrayBuffer = new ArrayBuffer(width * height * 4);
    const uint8Array = new Uint8Array(arrayBuffer);
    gl.readPixels(0, 0, width, height, gl.RGBA, gl.UNSIGNED_BYTE, uint8Array);
    const result = Buffer.from(UPNG.encode([
      _flipImage(width, height, 4, arrayBuffer),
    ], width, height, 0));
    console.dir({width, height, image: name, result: result.length});
    fs.writeFileSync(name, result);
  }
  const _decorateModelViewProjection = (o, layer, display, factor) => {
    if (!o.viewports) {
      o.viewports = [
        new Float32Array(4),
        new Float32Array(4),
      ];
    }
    if (!o.modelView) {
      o.modelView = [
        new Float32Array(16),
        new Float32Array(16),
      ];
    }
    if (!o.projection) {
      o.projection = [
        new Float32Array(16),
        new Float32Array(16),
      ];
    }
    
    const [{leftBounds, rightBounds}] = display.getLayers();
    const {renderWidth, renderHeight} = display.getEyeParameters('left');
    const offsetMatrix = localMatrix2.compose(localVector.fromArray(layer.xrOffset.position), localQuaternion.fromArray(layer.xrOffset.orientation), localVector2.fromArray(layer.xrOffset.scale));
    
    o.viewports[0][0] = leftBounds[0]*renderWidth * factor;
    o.viewports[0][1] = leftBounds[1]*renderHeight;
    o.viewports[0][2] = leftBounds[2]*renderWidth * factor;
    o.viewports[0][3] = leftBounds[3]*renderHeight;
    o.viewports[1][0] = rightBounds[0]*renderWidth * factor;
    o.viewports[1][1] = rightBounds[1]*renderHeight;
    o.viewports[1][2] = rightBounds[2]*renderWidth * factor;
    o.viewports[1][3] = rightBounds[3]*renderHeight;
    localMatrix.fromArray(display._frameData.leftViewMatrix)
      .multiply(offsetMatrix)
      .toArray(o.modelView[0]);
    localMatrix.fromArray(display._frameData.rightViewMatrix)
      .multiply(offsetMatrix)
      .toArray(o.modelView[1]);
    o.projection[0].set(display._frameData.leftProjectionMatrix);
    o.projection[1].set(display._frameData.rightProjectionMatrix);
  };
  const _decorateModelViewProjections = (layers, display, factor) => {
    for (let i = 0; i < layers.length; i++) {
      const layer = layers[i];
      if (layer.constructor.name === 'HTMLIFrameElement' && layer.browser) {
        _decorateModelViewProjection(layer.browser, layer, display, factor);
      }
    }
  };
  const _blit = () => {
    for (let i = 0; i < contexts.length; i++) {
      const context = contexts[i];

      const isDirty = (!!context.isDirty && context.isDirty()) || mlPresentState.mlGlContext === context;
      if (isDirty) {
        const windowHandle = context.getWindowHandle();

        const {nativeWindow} = nativeBindings;
        nativeWindow.setCurrentWindowContext(windowHandle);
        if (isMac) {
          context.flush();
        }

        const isVisible = nativeWindow.isVisible(windowHandle) || vrPresentState.glContext === context || mlPresentState.mlGlContext === context;
        if (isVisible) {
          if (vrPresentState.glContext === context && vrPresentState.hasPose) {
            if (vrPresentState.layers.length > 0) {
              const {vrDisplay, xrDisplay} = window[symbols.mrDisplaysSymbol];
              _decorateModelViewProjections(vrPresentState.layers, vrDisplay, 2); // note: vrDisplay mirrors xrDisplay
              nativeWindow.composeLayers(context, vrPresentState.fbo, vrPresentState.layers);
            } else {
              nativeWindow.blitFrameBuffer(context, vrPresentState.msFbo, vrPresentState.fbo, vrPresentState.glContext.canvas.width, vrPresentState.glContext.canvas.height, vrPresentState.glContext.canvas.width, vrPresentState.glContext.canvas.height, true, false, false);
            }

            vrPresentState.compositor.Submit(context, vrPresentState.tex);
            vrPresentState.hasPose = false;

            nativeWindow.blitFrameBuffer(context, vrPresentState.fbo, 0, vrPresentState.glContext.canvas.width * (args.blit ? 0.5 : 1), vrPresentState.glContext.canvas.height, window.innerWidth, window.innerHeight, true, false, false);
          } else if (mlPresentState.mlGlContext === context && mlPresentState.mlHasPose) {
            if (mlPresentState.layers.length > 0) { // TODO: composition can be directly to the output texture array
              const {mlDisplay, xmDisplay} = window[symbols.mrDisplaysSymbol];
              _decorateModelViewProjections(mlPresentState.layers, mlDisplay, 2); // note: mlDisplay mirrors xmDisplay
              nativeWindow.composeLayers(context, mlPresentState.mlFbo, mlPresentState.layers);
            } else {
              nativeWindow.blitFrameBuffer(context, mlPresentState.mlMsFbo, mlPresentState.mlFbo, mlPresentState.mlGlContext.canvas.width, mlPresentState.mlGlContext.canvas.height, mlPresentState.mlGlContext.canvas.width, mlPresentState.mlGlContext.canvas.height, true, false, false);
            }

            mlPresentState.mlContext.SubmitFrame(context, mlPresentState.mlFbo, mlPresentState.mlGlContext.canvas.width, mlPresentState.mlGlContext.canvas.height);
            mlPresentState.mlHasPose = false;

            // nativeWindow.blitFrameBuffer(context, mlPresentState.mlFbo, 0, mlPresentState.mlGlContext.canvas.width, mlPresentState.mlGlContext.canvas.height, window.innerWidth, window.innerHeight, true, false, false);
          } else if (fakePresentState.layers.length > 0) {
            const {fakeVrDisplay} = window[symbols.mrDisplaysSymbol];
            _decorateModelViewProjections(fakePresentState.layers, fakeVrDisplay, 1);
            nativeWindow.composeLayers(context, 0, fakePresentState.layers);
          }
        }

        if (isMac) {
          context.bindFramebufferRaw(context.FRAMEBUFFER, null);
        }
        nativeWindow.swapBuffers(windowHandle);
        if (isMac) {
          const drawFramebuffer = context.getFramebuffer(context.DRAW_FRAMEBUFFER);
          if (drawFramebuffer) {
            context.bindFramebuffer(context.DRAW_FRAMEBUFFER, drawFramebuffer);
          }

          const readFramebuffer = context.getFramebuffer(context.READ_FRAMEBUFFER);
          if (readFramebuffer) {
            context.bindFramebuffer(context.READ_FRAMEBUFFER, readFramebuffer);
          }
        }

        numDirtyFrames++;
        _checkDirtyFrameTimeout();

        context.clearDirty();
      }
    }
  }

  let lastFrameTime = Date.now();
  const timestamps = {
    frames: 0,
    last: Date.now(),
    idle: 0,
    wait: 0,
    prepare: 0,
    events: 0,
    media: 0,
    user: 0,
    submit: 0,
    total: 0,
  };
  const TIMESTAMP_FRAMES = DEFAULT_FPS;
  const [leftGamepad, rightGamepad] = core.getAllGamepads();
  const gamepads = [null, null];
  const frameData = new window.VRFrameData();
  const stageParameters = new window.VRStageParameters();
  let timeout = null;
  let numDirtyFrames = 0;
  const dirtyFrameContexts = [];
  const _checkDirtyFrameTimeout = () => {
    if (dirtyFrameContexts.length > 0) {
      const removedDirtyFrameContexts = [];

      for (let i = 0; i < dirtyFrameContexts.length; i++) {
        const dirtyFrameContext = dirtyFrameContexts[i];
        const {dirtyFrames} = dirtyFrameContext;
        if (numDirtyFrames > dirtyFrames && contexts.length > 0) {
          const {fn} = dirtyFrameContext;
          fn(null, contexts[0]);

          removedDirtyFrameContexts.push(dirtyFrameContext);
        }
      }

      for (let i = 0; i < removedDirtyFrameContexts.length; i++) {
        const removedDirtyFrameContext = removedDirtyFrameContexts[i];
        dirtyFrameContexts.splice(dirtyFrameContexts.indexOf(removedDirtyFrameContext), 1);
      }
    }
  };
  const setDirtyFrameTimeout = ({
    dirtyFrames = 1,
    timeout = 1000,
  } = {}, fn) => {
    if (numDirtyFrames > dirtyFrames && contexts.length > 0) {
      process.nextTick(() => {
        fn(null, contexts[0]);
      });
    } else {
      const localTimeout = setTimeout(() => {
        const err = new Error('timed out');
        err.code = 'ETIMEOUT';
        fn(err);

        dirtyFrameContexts.splice(dirtyFrameContexts.indexOf(dirtyFrameContext), 1);
      }, timeout);
      const dirtyFrameContext = {
        dirtyFrames,
        fn: (err, context) => {
          fn(err, context);

          clearTimeout(localTimeout);
        },
      };
      dirtyFrameContexts.push(dirtyFrameContext);
    }
  };
  window.setDirtyFrameTimeout = setDirtyFrameTimeout;

  const _recurse = () => {
    if (args.performance) {
      if (timestamps.frames >= TIMESTAMP_FRAMES) {
        console.log(`${(TIMESTAMP_FRAMES/(timestamps.total/1000)).toFixed(0)} FPS | ${timestamps.idle}ms idle | ${timestamps.wait}ms wait | ${timestamps.prepare}ms prepare | ${timestamps.events}ms events | ${timestamps.media}ms media | ${timestamps.user}ms user | ${timestamps.submit}ms submit`);

        timestamps.frames = 0;
        timestamps.idle = 0;
        timestamps.wait = 0;
        timestamps.prepare = 0;
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

    const {fakeVrDisplay} = window[symbols.mrDisplaysSymbol];
    if (fakeVrDisplay.isPresenting) {
      fakeVrDisplay.waitGetPoses();
    }
    if (vrPresentState.isPresenting && vrPresentState.glContext && vrPresentState.glContext.canvas.ownerDocument.defaultView === window) {
      // wait for frame
      vrPresentState.compositor.WaitGetPoses(
        vrPresentState.system,
        localFloat32Array, // hmd
        localFloat32Array2, // left controller
        localFloat32Array3 // right controller
      );
      vrPresentState.hasPose = true;
      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.wait += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }
      _normalizeMatrixArray(localFloat32Array);
      _normalizeMatrixArray(localFloat32Array2);
      _normalizeMatrixArray(localFloat32Array3);

      // build frame data
      const hmdMatrix = localMatrix.fromArray(localFloat32Array);

      hmdMatrix.decompose(localVector, localQuaternion, localVector2);
      frameData.pose.set(localVector, localQuaternion);

      hmdMatrix.getInverse(hmdMatrix);

      vrPresentState.system.GetEyeToHeadTransform(0, localFloat32Array4);
      localMatrix2.fromArray(localFloat32Array4);
      localMatrix2.decompose(localVector, localQuaternion, localVector2);
      localVector.toArray(localOffsetArray);
      const leftOffset = localOffsetArray;
      localMatrix2
        .getInverse(localMatrix2)
        .multiply(hmdMatrix);
      localMatrix2.toArray(frameData.leftViewMatrix);

      vrPresentState.system.GetProjectionMatrix(0, depthNear, depthFar, localFloat32Array4);
      _normalizeMatrixArray(localFloat32Array4);
      frameData.leftProjectionMatrix.set(localFloat32Array4);

      vrPresentState.system.GetProjectionRaw(0, localFovArray);
      for (let i = 0; i < localFovArray.length; i++) {
        localFovArray[i] = Math.atan(localFovArray[i]) / Math.PI * 180;
      }
      const leftFov = localFovArray;

      vrPresentState.system.GetEyeToHeadTransform(1, localFloat32Array4);
      _normalizeMatrixArray(localFloat32Array4);
      localMatrix2.fromArray(localFloat32Array4);
      localMatrix2.decompose(localVector, localQuaternion, localVector2);
      localVector.toArray(localOffsetArray2);
      const rightOffset = localOffsetArray2;
      localMatrix2
        .getInverse(localMatrix2)
        .multiply(hmdMatrix);
      localMatrix2.toArray(frameData.rightViewMatrix);

      vrPresentState.system.GetProjectionMatrix(1, depthNear, depthFar, localFloat32Array4);
      _normalizeMatrixArray(localFloat32Array4);
      frameData.rightProjectionMatrix.set(localFloat32Array4);

      vrPresentState.system.GetProjectionRaw(1, localFovArray2);
      for (let i = 0; i < localFovArray2.length; i++) {
        localFovArray2[i] = Math.atan(localFovArray2[i]) / Math.PI * 180;
      }
      const rightFov = localFovArray2;

      // build stage parameters
      // vrPresentState.system.GetSeatedZeroPoseToStandingAbsoluteTrackingPose(localFloat32Array4);
      // _normalizeMatrixArray(localFloat32Array4);
      // stageParameters.sittingToStandingTransform.set(localFloat32Array4);

      // build gamepads data
      vrPresentState.system.GetControllerState(0, localGamepadArray);
      if (!isNaN(localGamepadArray[0])) {
        // matrix
        localMatrix.fromArray(localFloat32Array2);
        localMatrix.decompose(localVector, localQuaternion, localVector2);
        localVector.toArray(leftGamepad.pose.position);
        localQuaternion.toArray(leftGamepad.pose.orientation);

        leftGamepad.buttons[0].pressed = localGamepadArray[4] !== 0; // pad
        leftGamepad.buttons[1].pressed = localGamepadArray[5] !== 0; // trigger
        leftGamepad.buttons[2].pressed = localGamepadArray[3] !== 0; // grip
        leftGamepad.buttons[3].pressed = localGamepadArray[2] !== 0; // menu
        leftGamepad.buttons[4].pressed = localGamepadArray[1] !== 0; // system

        leftGamepad.buttons[0].touched = localGamepadArray[9] !== 0; // pad
        leftGamepad.buttons[1].touched = localGamepadArray[10] !== 0; // trigger
        leftGamepad.buttons[2].touched = localGamepadArray[8] !== 0; // grip
        leftGamepad.buttons[3].touched = localGamepadArray[7] !== 0; // menu
        leftGamepad.buttons[4].touched = localGamepadArray[6] !== 0; // system

        for (let i = 0; i < 10; i++) {
          leftGamepad.axes[i] = localGamepadArray[11+i];
        }
        leftGamepad.buttons[1].value = leftGamepad.axes[2]; // trigger

        gamepads[0] = leftGamepad;
      } else {
        gamepads[0] = null;
      }

      vrPresentState.system.GetControllerState(1, localGamepadArray);
      if (!isNaN(localGamepadArray[0])) {
        // matrix
        localMatrix.fromArray(localFloat32Array3);
        localMatrix.decompose(localVector, localQuaternion, localVector2);
        localVector.toArray(rightGamepad.pose.position);
        localQuaternion.toArray(rightGamepad.pose.orientation);

        rightGamepad.buttons[0].pressed = localGamepadArray[4] !== 0; // pad
        rightGamepad.buttons[1].pressed = localGamepadArray[5] !== 0; // trigger
        rightGamepad.buttons[2].pressed = localGamepadArray[3] !== 0; // grip
        rightGamepad.buttons[3].pressed = localGamepadArray[2] !== 0; // menu
        rightGamepad.buttons[4].pressed = localGamepadArray[1] !== 0; // system

        rightGamepad.buttons[0].touched = localGamepadArray[9] !== 0; // pad
        rightGamepad.buttons[1].touched = localGamepadArray[10] !== 0; // trigger
        rightGamepad.buttons[2].touched = localGamepadArray[8] !== 0; // grip
        rightGamepad.buttons[3].touched = localGamepadArray[7] !== 0; // menu
        rightGamepad.buttons[4].touched = localGamepadArray[6] !== 0; // system

        for (let i = 0; i < 10; i++) {
          rightGamepad.axes[i] = localGamepadArray[11+i];
        }
        rightGamepad.buttons[1].value = rightGamepad.axes[2]; // trigger

        gamepads[1] = rightGamepad;
      } else {
        gamepads[1] = null;
      }

      if (vrPresentState.lmContext) {
        vrPresentState.lmContext.WaitGetPoses(handsArray);
      }

      // update vr frame
      window.top.updateVrFrame({
        depthNear,
        depthFar,
        renderWidth,
        renderHeight,
        leftOffset,
        leftFov,
        rightOffset,
        rightFov,
        frameData,
        stageParameters,
        gamepads,
        handsArray,
      });

      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.prepare += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }
    } else if (mlPresentState.mlGlContext && mlPresentState.mlGlContext.canvas.ownerDocument.defaultView === window) {
      mlPresentState.mlHasPose = mlPresentState.mlContext.WaitGetPoses(mlPresentState.mlGlContext, mlPresentState.mlMsFbo, renderWidth*2, renderHeight, transformArray, projectionArray, controllersArray);
      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.wait += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }

      if (mlPresentState.mlHasPose) {
        const depthNear = 0.1;
        const depthFar = 100;

        localVector.fromArray(transformArray, 0);
        localQuaternion.fromArray(transformArray, 3);
        localVector2.set(1, 1, 1);
        localMatrix.compose(localVector, localQuaternion, localVector2).getInverse(localMatrix);
        frameData.pose.set(localVector, localQuaternion);
        localMatrix.toArray(frameData.leftViewMatrix);
        frameData.leftProjectionMatrix.set(projectionArray.slice(0, 16));

        localVector.fromArray(transformArray, 3 + 4);
        localQuaternion.fromArray(transformArray, 3 + 4 + 3);
        // localVector2.set(1, 1, 1);
        localMatrix.compose(localVector, localQuaternion, localVector2).getInverse(localMatrix);
        localMatrix.toArray(frameData.rightViewMatrix);
        frameData.rightProjectionMatrix.set(projectionArray.slice(16, 32));

        let controllersArrayIndex = 0;
        leftGamepad.pose.position.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 3));
        controllersArrayIndex += 3;
        leftGamepad.pose.orientation.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 4));
        controllersArrayIndex += 4;
        const leftTriggerValue = controllersArray[controllersArrayIndex];
        leftGamepad.buttons[1].value = leftTriggerValue;
        const leftTriggerPushed = leftTriggerValue > 0.5;
        leftGamepad.buttons[1].touched = leftTriggerPushed;
        leftGamepad.buttons[1].pressed = leftTriggerPushed;
        leftGamepad.axes[2] = leftTriggerValue;
        controllersArrayIndex++;
        const leftBumperValue = controllersArray[controllersArrayIndex];
        leftGamepad.buttons[2].value = leftBumperValue;
        const leftBumperPushed = leftBumperValue > 0.5;
        leftGamepad.buttons[2].touched = leftBumperPushed;
        leftGamepad.buttons[2].pressed = leftBumperPushed;
        controllersArrayIndex++;
        const leftHomeValue = controllersArray[controllersArrayIndex];
        leftGamepad.buttons[3].value = leftHomeValue;
        const leftHomePushed = leftHomeValue > 0.5;
        leftGamepad.buttons[3].touched = leftHomePushed;
        leftGamepad.buttons[3].pressed = leftHomePushed;
        controllersArrayIndex++;
        leftGamepad.axes[0] = controllersArray[controllersArrayIndex];
        leftGamepad.axes[1] = controllersArray[controllersArrayIndex + 1];
        const leftPadValue = controllersArray[controllersArrayIndex + 2];
        leftGamepad.buttons[0].value = leftPadValue;
        const leftPadTouched = leftPadValue > 0;
        const leftPadPushed = leftPadValue > 0.5;
        leftGamepad.buttons[0].touched = leftPadTouched;
        leftGamepad.buttons[0].pressed = leftPadPushed;
        controllersArrayIndex += 3;

        gamepads[0] = leftGamepad;

        rightGamepad.pose.position.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 3));
        controllersArrayIndex += 3;
        rightGamepad.pose.orientation.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 4));
        controllersArrayIndex += 4;
        const rightTriggerValue = controllersArray[controllersArrayIndex];
        rightGamepad.buttons[1].value = rightTriggerValue;
        const rightTriggerPushed = rightTriggerValue > 0.5;
        rightGamepad.buttons[1].touched = rightTriggerPushed;
        rightGamepad.buttons[1].pressed = rightTriggerPushed;
        rightGamepad.axes[2] = rightTriggerValue;
        controllersArrayIndex++;
        const rightBumperValue = controllersArray[controllersArrayIndex];
        rightGamepad.buttons[2].value = rightBumperValue;
        const rightBumperPushed = rightBumperValue > 0.5;
        rightGamepad.buttons[2].touched = rightBumperPushed;
        rightGamepad.buttons[2].pressed = rightBumperPushed;
        controllersArrayIndex++;
        const rightHomeValue = controllersArray[controllersArrayIndex];
        rightGamepad.buttons[3].value = rightHomeValue;
        const rightHomePushed = rightHomeValue > 0.5;
        rightGamepad.buttons[3].touched = rightHomePushed;
        rightGamepad.buttons[3].pressed = rightHomePushed;
        controllersArrayIndex++;
        rightGamepad.axes[0] = controllersArray[controllersArrayIndex];
        rightGamepad.axes[1] = controllersArray[controllersArrayIndex + 1];
        const rightPadValue = controllersArray[controllersArrayIndex + 2];
        rightGamepad.buttons[0].value = rightPadValue;
        const rightPadTouched = rightPadValue > 0;
        const rightPadPushed = rightPadValue > 0.5;
        rightGamepad.buttons[0].touched = rightPadTouched;
        rightGamepad.buttons[0].pressed = rightPadPushed;
        controllersArrayIndex += 3;

        gamepads[1] = rightGamepad;

        // update ml frame
        window.top.updateVrFrame({
          depthNear,
          depthFar,
          frameData,
          stageParameters,
          gamepads,
          context: mlPresentState.mlContext,
        });
      }

      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.prepare += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }
    } else {
      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.wait += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }
    }

    // poll for window events
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
    nativeBindings.nativeBrowser.Browser.updateAll();
    // update magic leap state
    if (mlPresentState.mlGlContext) {
      if (mlPresentState.mlGlContext.canvas.ownerDocument.defaultView === window) {
        nativeBindings.nativeMl.Update(mlPresentState.mlContext, mlPresentState.mlGlContext);
      }
      nativeBindings.nativeMl.Poll();
    }
    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.media += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    // trigger requestAnimationFrame
    if (args.frame || args.minimalFrame) {
      console.log('-'.repeat(80) + 'start frame');
    }
    window.tickAnimationFrame();
    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.user += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }
    if (args.image && _takeScreenshot) {
      _saveImage(contexts[contexts.length - 1], args.image);
      process.exit(0);
    }
    _blit();
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
    const now = Date.now();
    const timeoutDelay = (() => {
      if (args.uncapped) {
        return 0;
      } else {
        const frameTimeMax = _getFrameTimeMax();
        const frameTimeMin = _getFrameTimeMin();
        return Math.min(Math.max(frameTimeMax - ~~(now - lastFrameTime), frameTimeMin), frameTimeMax);
      }
    })();
    timeout = setTimeout(_recurse, timeoutDelay);
    lastFrameTime = now;
  };
  timeout = setTimeout(_recurse);
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

const _start = () => {
  let {url: u} = args;
  if (!u && args.home) {
    u = 'file://' + path.join(path.dirname(require.resolve('exokit-home')), 'index.html') + (args.tag ? ('?t=' + encodeURIComponent(args.tab)) : '');
  }
  if (u) {
    if (u === '.') {
      console.warn('NOTE: You ran `exokit . <url>`\n(Did you mean to run `node . <url>` or `exokit <url>` instead?)')
    }
    u = u.replace(/^exokit:/, '');
    if (u && !url.parse(u).protocol) {
      u = 'file://' + path.resolve(process.cwd(), u);
    }
    return core.load(u, {
      dataPath,
      args,
    })
      .then(window => {
        if (args.image) {
          window.setDirtyFrameTimeout({
            dirtyFrames: 100,
            timeout: 5000,
          }, (err, gl) => {
            if (!err) {
              _takeScreenshot = true;
            } else {
              throw err;
            }
          });
        }
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

    let lastUnderscore = window._;
    const replEval = (cmd, context, filename, callback) => {
      cmd = cmd.slice(0, -1); // remove trailing \n

      let result, err = null, match;

      if (/^[a-z]+:\/\//.test(cmd)) {
        window.location.href = cmd;
      } else if (/^\s*<(?:\!\-*)?[a-z]/i.test(cmd)) {
        const e = window.document.createElement('div');
        e.innerHTML = cmd;
        if (e.childNodes.length === 0) {
          result = undefined;
        } else if (e.childNodes.length === 1) {
          result = e.childNodes[0];
        } else {
          result = e.childNodes;
        }
      } else if (match = cmd.match(/^\s*(?:const|var|let)?\s*([a-z][a-z0-9]*)\s*=\s*(<(?:\!\-*)?[a-z].*)$/im)) {
        const e = window.document.createElement('div');
        e.innerHTML = match[2];
        if (e.childNodes.length === 0) {
          result = undefined;
        } else if (e.childNodes.length === 1) {
          result = e.childNodes[0];
        } else {
          result = e.childNodes;
        }
        window[match[1]] = result;
      } else {
        try {
          result = window.vm.run(cmd, filename);
        } catch(e) {
          err = e;
        }
      }

      if (!err) {
        if (window._ === lastUnderscore) {
          window._ = result;
          lastUnderscore = result;
        }
        if (result !== undefined) {
          r.setPrompt(prompt);
        }
      } else {
        if (err.name === 'SyntaxError') {
          err = new repl.Recoverable(err);
        }
      }

      GlobalContext.commands.push(cmd);

      callback(err, result);
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
  if (nativeBindings.nativeAnalytics) {
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

  _prepare()
    .then(() => _start())
    .catch(err => {
      console.warn(err.stack);
      process.exit(1);
    });
}

module.exports = core;
