#!/usr/bin/env node

const cwd = process.cwd();
process.chdir(__dirname); // needed for global bin to find libraries

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

const {version} = require('./package.json');
const nativeBindingsModulePath = path.join(__dirname, 'native-bindings.js');
const {THREE} = core;
const nativeBindings = require(nativeBindingsModulePath);
const {nativeVideo, nativeVr, nativeLm, nativeMl, nativeWindow, nativeAnalytics} = nativeBindings;

const GlobalContext = require('./src/GlobalContext');
GlobalContext.commands = [];

const dataPath = path.join(os.homedir() || __dirname, '.exokit');
const DEFAULT_FPS = 60; // TODO: Use different FPS for device.requestAnimationFrame vs window.requestAnimationFrame
const VR_FPS = 90;
const ML_FPS = 60;
const MLSDK_PORT = 17955;

const contexts = [];
const _windowHandleEquals = (a, b) => a[0] === b[0] && a[1] === b[1];

let _takeScreenshot = false;

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
      ],
      string: [
        'tab',
        'webgl',
        'xr',
        'size',
        'image',
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
    };
  } else {
    return {};
  }
})();

nativeBindings.nativeGl.onconstruct = (gl, canvas) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  const windowSpec = (() => {
    try {
      const visible = !args.image && canvas.ownerDocument.documentElement.contains(canvas);
      const {hidden} = canvas.ownerDocument;
      const firstWindowHandle = contexts.length > 0 ? contexts[0].getWindowHandle() : null;
      const firstGl = contexts.length > 0 ? contexts[0] : null;
      return nativeWindow.create(canvasWidth, canvasHeight, visible && !hidden, hidden, firstWindowHandle, firstGl);
    } catch (err) {
      console.warn(err.message);
      return null;
    }
  })();

  if (windowSpec) {
    const [windowHandle, sharedFramebuffer, sharedColorTexture, sharedDepthStencilTexture, sharedMsFramebuffer, sharedMsColorTexture, sharedMsDepthStencilTexture, vao] = windowSpec;

    gl.setWindowHandle(windowHandle);
    gl.setDefaultVao(vao);

    gl.canvas = canvas;

    const document = canvas.ownerDocument;
    const window = document.defaultView;

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

    const cleanups = [];

    const {hidden} = document;
    if (hidden) {
      const [framebuffer, colorTexture, depthStencilTexture, msFramebuffer, msColorTexture, msDepthStencilTexture] = nativeWindow.createRenderTarget(gl, canvasWidth, canvasHeight, sharedColorTexture, sharedDepthStencilTexture);

      gl.setDefaultFramebuffer(msFramebuffer);

      gl.resize = (width, height) => {
        nativeWindow.setCurrentWindowContext(windowHandle);
        nativeWindow.resizeRenderTarget(gl, width, height, framebuffer, colorTexture, depthStencilTexture, msFramebuffer, msColorTexture, msDepthStencilTexture);
      };

      document._emit('framebuffer', {
        framebuffer,
        colorTexture,
        depthStencilTexture,
        render() {
          nativeWindow.setCurrentWindowContext(windowHandle);

          // color blit is linear, depth/stencil is nearest
          nativeWindow.blitFrameBuffer(gl, msFramebuffer, framebuffer, canvas.width, canvas.height, canvas.width, canvas.height, true, false, false);
          nativeWindow.blitFrameBuffer(gl, msFramebuffer, framebuffer, canvas.width, canvas.height, canvas.width, canvas.height, false, true, true);
        },
      });
    } else {
      gl.resize = (width, height) => {
        nativeWindow.setCurrentWindowContext(windowHandle);
        nativeWindow.resizeRenderTarget(gl, width, height, sharedFramebuffer, sharedColorTexture, sharedDepthStencilTexture, sharedMsFramebuffer, sharedMsColorTexture, sharedMsDepthStencilTexture);
      };
    }
    Object.defineProperty(gl, 'drawingBufferWidth', {
      get() {
        return canvas.width;
      },
    });
    Object.defineProperty(gl, 'drawingBufferHeight', {
      get() {
        return canvas.height;
      },
    });

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

    cleanups.push(() => {
      nativeWindow.setCurrentWindowContext(windowHandle);

      if (gl === vrPresentState.glContext) {
        nativeVr.VR_Shutdown();

        vrPresentState.glContext = null;
        vrPresentState.system = null;
        vrPresentState.compositor = null;
      }
      if (gl === mlGlContext) {
        mlGlContext = null;
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
    });

    gl.destroy = (destroy => function() {
      destroy.call(this);

      for (let i = 0; i < cleanups.length; i++) {
        cleanups[i]();
      }
    })(gl.destroy);

    contexts.push(gl);
    fps = nativeWindow.getRefreshRate();

    canvas.ownerDocument.defaultView.on('unload', () => {
      gl.destroy();
    });

    return true;
  } else {
    return false;
  }
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
const framebufferArray = new Uint32Array(2);
const transformArray = new Float32Array(7 * 2);
const projectionArray = new Float32Array(16 * 2);
const viewportArray = new Uint32Array(4);
const handsArray = [
  new Float32Array(handEntrySize),
  new Float32Array(handEntrySize),
];
const controllersArray = new Float32Array((3 + 4 + 3) * 2);

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
};
let renderWidth = 0;
let renderHeight = 0;
const depthNear = 0.1;
const depthFar = 10000.0;
if (nativeVr) {
  nativeVr.requestPresent = function(layers) {
    if (!vrPresentState.glContext) {
      const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
      if (layer) {
        const canvas = layer.source;
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }
        const window = canvas.ownerDocument.defaultView;

        const windowHandle = context.getWindowHandle();
        nativeWindow.setCurrentWindowContext(windowHandle);

        fps = VR_FPS;

        const vrContext = vrPresentState.vrContext || nativeVr.getContext();
        const system = vrPresentState.system || nativeVr.VR_Init(nativeVr.EVRApplicationType.Scene);
        const compositor = vrPresentState.compositor || vrContext.compositor.NewCompositor();

        const lmContext = vrPresentState.lmContext || (nativeLm && new nativeLm());

        const {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2;
        renderWidth = halfWidth;
        renderHeight = height;

        const cleanups = [];

        const [fbo, tex, depthStencilTex, msFbo, msTex, msDepthStencilTex] = nativeWindow.createRenderTarget(context, width, height, 0, 0, 0, 0);

        context.setDefaultFramebuffer(msFbo);

        vrPresentState.isPresenting = true;
        vrPresentState.vrContext = vrContext;
        vrPresentState.system = system;
        vrPresentState.compositor = compositor;
        vrPresentState.glContext = context;
        vrPresentState.msFbo = msFbo;
        vrPresentState.msTex = msTex;
        vrPresentState.msDepthTex = msDepthStencilTex;
        vrPresentState.fbo = fbo;
        vrPresentState.tex = tex;
        vrPresentState.depthTex = depthStencilTex;
        vrPresentState.cleanups = cleanups;

        vrPresentState.lmContext = lmContext;

        const _attribute = (name, value) => {
          if (name === 'width' || name === 'height') {
            nativeWindow.setCurrentWindowContext(windowHandle);

            nativeWindow.resizeRenderTarget(context, canvas.width, canvas.height, fbo, tex, depthStencilTex, msFbo, msTex, msDepthStencilTex);
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

        return {
          width,
          height,
          framebuffer: msFbo,
        };
      } else {
        throw new Error('no HTMLCanvasElement source provided');
      }
    } else {
      /* const {width: halfWidth, height} = vrPresentState.system.GetRecommendedRenderTargetSize();
      const width = halfWidth * 2; */

      return {
        width: renderWidth * 2,
        height: renderHeight,
        framebuffer: vrPresentState.msFbo,
      };
    }
  };
  nativeVr.exitPresent = function() {
    if (vrPresentState.isPresenting) {
      nativeVr.VR_Shutdown();

      nativeWindow.destroyRenderTarget(vrPresentState.msFbo, vrPresentState.msTex, vrPresentState.msDepthStencilTex);
      nativeWindow.destroyRenderTarget(vrPresentState.fbo, vrPresentState.tex, vrPresentState.msDepthTex);

      const context = vrPresentState.glContext;
      nativeWindow.setCurrentWindowContext(context.getWindowHandle());
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
let mlContext = null;
let mlFbo = null;
let mlTex = null;
let mlDepthTex = null;
let mlMsFbo = null;
let mlMsTex = null;
let mlMsDepthTex = null;
let mlGlContext = null;
let mlCleanups = null;
let mlHasPose = false;
if (nativeMl) {
  mlContext = new nativeMl();
  nativeMl.requestPresent = function(layers) {
    if (!mlGlContext) {
      const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
      if (layer) {
        const canvas = layer.source;
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }
        const window = canvas.ownerDocument.defaultView;

        const windowHandle = context.getWindowHandle();
        nativeWindow.setCurrentWindowContext(windowHandle);

        fps = ML_FPS;

        const initResult = mlContext.Present(windowHandle);
        if (initResult) {
          const {width: halfWidth, height} = initResult;
          const width = halfWidth * 2;
          renderWidth = halfWidth;
          renderHeight = height;
          
          const [fbo, tex, depthStencilTex, msFbo, msTex, msDepthStencilTex] = nativeWindow.createRenderTarget(context, width, height, 0, 0, 0, 0);
          mlFbo = fbo;
          mlTex = tex;
          mlDepthTex = depthStencilTex;
          mlMsFbo = msFbo;
          mlMsTex = msTex;
          mlMsDepthTex = msDepthStencilTex;

          const cleanups = [];
          mlCleanups = cleanups;

          const _attribute = (name, value) => {
            if (name === 'width' || name === 'height') {
              nativeWindow.setCurrentWindowContext(windowHandle);

              nativeWindow.resizeRenderTarget(context, canvas.width, canvas.height, fbo, tex, depthStencilTex, msFbo, msTex, msDepthStencilTex);
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

          context.setDefaultFramebuffer(mlMsFbo);

          mlGlContext = context;

          return {
            width,
            height,
            framebuffer: mlMsFbo,
          };
        } else {
          throw new Error('failed to present ml context');
        }
      } else {
        throw new Error('no HTMLCanvasElement source provided');
      }
    } else {
      return {
        width: renderWidth * 2,
        height: renderHeight,
        framebuffer: mlMsFbo,
      };
    }
  };
  nativeMl.exitPresent = function() {
    nativeWindow.destroyRenderTarget(mlMsFbo, mlMsTex, mlMsDepthTex);
    nativeWindow.destroyRenderTarget(mlFbo, mlTex, mlDepthTex);

    nativeWindow.setCurrentWindowContext(mlGlContext.getWindowHandle());
    mlGlContext.setDefaultFramebuffer(0);

    for (let i = 0; i < mlCleanups.length; i++) {
      mlCleanups[i]();
    }

    mlFbo = null;
    mlTex = null;
    mlDepthTex = null;
    mlMsFbo = null;
    mlMsTex = null;
    mlMsDepthTex = null;
    mlGlContext = null;
    mlCleanups = null;
    mlHasPose = false;
  };

  if (!nativeMl.IsSimulated()) {
    nativeMl.InitLifecycle();
  } else {
    // try to connect to MLSDK
    const s = net.connect(MLSDK_PORT, '127.0.0.1', () => {
      s.destroy();

      nativeMl.InitLifecycle();
    });
    s.on('error', () => {});
  }
}

nativeWindow.setEventHandler((type, data) => {
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
        if (data.keyCode === 27) {
          if (window.top.document.pointerLockElement) {
            window.top.document.exitPointerLock();
            handled = true;
          }
          if (window.top.document.fullscreenElement) {
            window.top.document.exitFullscreen();
            handled = true;
          }
        }
        //122 is F11 key
        if (data.keyCode === 122){
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

core.setVersion(version);

let innerWidth = 1280; // XXX do not track this globally
let innerHeight = 1024;
let fps = DEFAULT_FPS;
const _getFrameTimeMax = () => ~~(1000 / fps);
const _getFrameTimeMin = () => 0;

const _bindWindow = (window, newWindowCb) => {
  window.innerWidth = innerWidth;
  window.innerHeight = innerHeight;
  window.on('unload', () => {
    clearTimeout(timeout);
  });
  window.on('navigate', newWindowCb);
  window.document.on('paste', e => {
    e.clipboardData = new window.DataTransfer();
    if (contexts.length > 0) {
      const context = contexts[0];
      const windowHandle = context.getWindowHandle();
      const clipboardContents = nativeWindow.getClipboard(windowHandle).slice(0, 256);
      const dataTransferItem = new window.DataTransferItem('string', 'text/plain', clipboardContents);
      e.clipboardData.items.push(dataTransferItem);
    }
  });

  window.document.addEventListener('pointerlockchange', () => {
    const {pointerLockElement} = window.document;

    if (pointerLockElement) {
      for (let i = 0; i < contexts.length; i++) {
        const context = contexts[i];

        if (context.canvas.ownerDocument.defaultView === window) {
          const windowHandle = context.getWindowHandle();

          if (nativeWindow.isVisible(windowHandle)) {
            nativeWindow.setCursorMode(windowHandle, false);
            break;
          }
        }
      }
    } else {
      for (let i = 0; i < contexts.length; i++) {
        const context = contexts[i];

        if (context.canvas.ownerDocument.defaultView === window) {
          const windowHandle = context.getWindowHandle();

          if (nativeWindow.isVisible(windowHandle)) {
            nativeWindow.setCursorMode(windowHandle, true);
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

          if (nativeWindow.isVisible(windowHandle)) {
            nativeWindow.setFullscreen(windowHandle);
            break;
          }
        }
      }
    } else {
      for (let i = 0; i < contexts.length; i++) {
        const context = contexts[i];

        if (context.canvas.ownerDocument.defaultView === window) {
          const windowHandle = context.getWindowHandle();

          if (nativeWindow.isVisible(windowHandle)) {
            nativeWindow.exitFullscreen(windowHandle);
            break;
          }
        }
      }
    }
  });
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
  const _blit = () => {
    for (let i = 0; i < contexts.length; i++) {
      const context = contexts[i];

      if (context.isDirty()) {
        const windowHandle = context.getWindowHandle();
        nativeWindow.setCurrentWindowContext(windowHandle);
        context.flush();

        if (nativeWindow.isVisible(windowHandle) || vrPresentState.glContext === context || mlGlContext === context) {
          if (vrPresentState.glContext === context && vrPresentState.hasPose) {
            nativeWindow.blitFrameBuffer(context, vrPresentState.msFbo, vrPresentState.fbo, vrPresentState.glContext.canvas.width, vrPresentState.glContext.canvas.height, vrPresentState.glContext.canvas.width, vrPresentState.glContext.canvas.height, true, false, false);

            vrPresentState.compositor.Submit(context, vrPresentState.tex);
            vrPresentState.hasPose = false;

            nativeWindow.blitFrameBuffer(context, vrPresentState.fbo, 0, vrPresentState.glContext.canvas.width * (args.blit ? 0.5 : 1), vrPresentState.glContext.canvas.height, window.innerWidth, window.innerHeight, true, false, false);
          } else if (mlGlContext === context && mlHasPose) {
            nativeWindow.blitFrameBuffer(context, mlMsFbo, mlFbo, mlGlContext.canvas.width, mlGlContext.canvas.height, mlGlContext.canvas.width, mlGlContext.canvas.height, true, false, false);

            mlContext.SubmitFrame(mlFbo, mlGlContext.canvas.width, mlGlContext.canvas.height);
            mlHasPose = false;

            nativeWindow.blitFrameBuffer(context, mlFbo, 0, mlGlContext.canvas.width, mlGlContext.canvas.height, window.innerWidth, window.innerHeight, true, false, false);
          }

          nativeWindow.swapBuffers(windowHandle);

          numDirtyFrames++;
          _checkDirtyFrameTimeout();

          context.clearDirty();
        }
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

  window.on('unload', () => {
    clearTimeout(timeout);
  });
  window.on('navigate', newWindowCb);
  window.document.on('paste', e => {
    e.clipboardData = new window.DataTransfer();
    if (contexts.length > 0) {
      const context = contexts[0];
      const windowHandle = context.getWindowHandle();
      const clipboardContents = nativeWindow.getClipboard(windowHandle).slice(0, 256);
      const dataTransferItem = new window.DataTransferItem('string', 'text/plain', clipboardContents);
      e.clipboardData.items.push(dataTransferItem);
    }
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

  window.addEventListener('error', err => {
    console.warn('got error', err);
  });

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
    } else if (mlGlContext && mlGlContext.canvas.ownerDocument.defaultView === window) {
      mlHasPose = mlContext.WaitGetPoses(framebufferArray, transformArray, projectionArray, viewportArray, controllersArray);
      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.wait += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }

      if (mlHasPose) {
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
        leftGamepad.axes[0] = controllersArray[controllersArrayIndex];
        leftGamepad.axes[1] = controllersArray[controllersArrayIndex + 1];
        controllersArrayIndex += 2;

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
        rightGamepad.axes[0] = controllersArray[controllersArrayIndex];
        rightGamepad.axes[1] = controllersArray[controllersArrayIndex + 1];
        controllersArrayIndex += 2;

        gamepads[1] = rightGamepad;

        // update ml frame
        window.top.updateVrFrame({
          depthNear,
          depthFar,
          frameData,
          stageParameters,
          gamepads,
          context: mlContext,
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
    nativeWindow.pollEvents();
    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.events += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    // update media frames
    nativeVideo.Video.updateAll();
    // update magic leap pre state
    if (nativeMl && mlGlContext) {
      nativeMl.PrePollEvents(mlContext);
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
    
    // update magic leap post state
    if (nativeMl && mlGlContext) {
      nativeMl.PostPollEvents(mlContext, mlGlContext, mlFbo, mlGlContext.canvas.width, mlGlContext.canvas.height);
    }
    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.media += diff;
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
      runtimePath = path.join(__dirname, 'node_modules', 'native-openvr-deps/bin/linux64');
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
                } else {
                  reject(err);
                }
              });
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
      u = 'file://' + path.resolve(cwd, u);
    }
    return core.load(u, {
      dataPath,
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
    if (args.quit) {
      process.exit();
    }
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
  if (nativeAnalytics) {
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

  core.setArgs(args);
  core.setNativeBindingsModule(nativeBindingsModulePath);

  _prepare()
    .then(() => _start());
} else {
  core.setNativeBindingsModule(nativeBindingsModulePath);
}

module.exports = core;
