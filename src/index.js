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
const repl = require('repl');

const core = require('./core.js');
const mkdirp = require('mkdirp');
const replHistory = require('repl.history');
const minimist = require('minimist');

const {version} = require('../package.json');
const {defaultEyeSeparation, maxNumTrackers} = require('./constants.js');
const symbols = require('./symbols');
const {THREE} = core;

const nativeBindingsModulePath = path.join(__dirname, 'native-bindings.js');
const nativeBindings = require(nativeBindingsModulePath);

const eventLoopNative = require('event-loop-native');
nativeBindings.nativeWindow.setEventLoop(eventLoopNative);

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
        'tab',
        'quit',
        'blit',
        'require',
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
        u: 'require',
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
      require: minimistArgs.require,
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
// const DEFAULT_FPS = 60; // TODO: Use different FPS for device.requestAnimationFrame vs window.requestAnimationFrame
// const VR_FPS = 90;
// const ML_FPS = 60;
const MLSDK_PORT = 17955;

const contexts = [];
GlobalContext.contexts = contexts;
const _windowHandleEquals = (a, b) => a[0] === b[0] && a[1] === b[1];

const windows = [];
GlobalContext.windows = windows;
// const _getTopWindow = () => windows.find(window => window.top === window);

nativeBindings.nativeGl.onconstruct = (gl, canvas) => {
  const canvasWidth = canvas.width;
  const canvasHeight = canvas.height;

  gl.canvas = canvas;

  const document = canvas.ownerDocument;
  const window = document.defaultView;

  const {nativeWindow} = nativeBindings;
  const windowSpec = (() => {
    if (!window[symbols.optionsSymbol].args.headless) {
      try {
        const visible = document.documentElement.contains(canvas);
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

    /* const nativeWindowSize = nativeWindow.getFramebufferSize(windowHandle);
    const nativeWindowHeight = nativeWindowSize.height;
    const nativeWindowWidth = nativeWindowSize.width;

    // Tell DOM how large the window is.
    window.innerHeight = nativeWindowSize.height;
    window.innerWidth = nativeWindowSize.width; */

    const title = `Exokit ${version}`;
    nativeWindow.setWindowTitle(windowHandle, title);

    const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeWindow.createRenderTarget(gl, canvasWidth, canvasHeight, sharedColorTexture, sharedDepthStencilTexture, sharedMsColorTexture, sharedMsDepthStencilTexture);
    gl.setDefaultFramebuffer(msFbo);
    gl.framebuffer = {
      msFbo,
      msTex,
      msDepthTex,
      fbo,
      tex,
      depthTex,
    };
    gl.resize = (width, height) => {
      nativeWindow.setCurrentWindowContext(windowHandle);
      nativeWindow.resizeRenderTarget(gl, width, height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
    };

    const {hidden} = document;
    if (hidden) {
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
        nativeBindings.nativeOpenVR.VR_Shutdown();

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

      const _hasMoreContexts = () => contexts.some(context => nativeWindow.isVisible(context.getWindowHandle()));
      if (!_hasMoreContexts()) {
        setImmediate(() => { // give time to create a replacement context
          if (!_hasMoreContexts()) {
            process.exit();
          }
        });
      }
    })(gl.destroy);

    canvas.ownerDocument.defaultView.on('unload', () => {
      gl.destroy();
    });
  } else {
    gl.destroy();
  }

  contexts.push(gl);
  // fps = nativeWindow.getRefreshRate();
};

nativeBindings.nativeCanvasRenderingContext2D.onconstruct = (ctx, canvas) => {
  const canvasWidth = canvas.width;
  const canvasHeight = canvas.height;

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
const localFloat32PoseArray = new Float32Array(16*(1+2+maxNumTrackers));
const localFloat32HmdPoseArray = new Float32Array(localFloat32PoseArray.buffer, localFloat32PoseArray.byteOffset + 0*Float32Array.BYTES_PER_ELEMENT*16, 16);
const localFloat32GamepadPoseArrays = [
  new Float32Array(localFloat32PoseArray.buffer, localFloat32PoseArray.byteOffset + 1*Float32Array.BYTES_PER_ELEMENT*16, 16),
  new Float32Array(localFloat32PoseArray.buffer, localFloat32PoseArray.byteOffset + 2*Float32Array.BYTES_PER_ELEMENT*16, 16),
];
const localFloat32TrackerPoseArrays = (() => {
  const result = Array(maxNumTrackers);
  for (let i = 0; i < maxNumTrackers; i++) {
    result[i] = new Float32Array(localFloat32PoseArray.buffer, localFloat32PoseArray.byteOffset + (3+i)*Float32Array.BYTES_PER_ELEMENT*16, 16);
  }
  return result;
})();
const localFloat32MatrixArray = new Float32Array(16);
const localFovArray = new Float32Array(4);
const localGamepadArray = new Float32Array(24);

// oculus desktop
const localPositionArray3 = new Float32Array(3);
const localQuaternionArray4 = new Float32Array(4);

const leftControllerPositionArray3 = new Float32Array(3);
const leftControllerQuaternionArray4 = new Float32Array(4);

const rightControllerPositionArray3 = new Float32Array(3);
const rightControllerQuaternionArray4 = new Float32Array(4);

// oculus mobile
const oculusMobilePoseFloat32Array = new Float32Array(3 + 4 + 1 + 4 + (16*2) + (16*2) + (16+12) + (16+12));

const handEntrySize = (1 + (5 * 5)) * (3 + 3);
const transformArray = new Float32Array(7 * 2);
const projectionArray = new Float32Array(16 * 2);
/* const handsArray = [
  new Float32Array(handEntrySize),
  new Float32Array(handEntrySize),
]; */
const controllersArray = new Float32Array((1 + 3 + 4 + 6) * 2);

const localVector = new THREE.Vector3();
const localVector2 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();

const vrPresentState = {
  vrContext: null,
  isPresenting: false,
  system: null,
  oculusSystem: null,
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

class XRState {
  constructor() {
    const sab = new SharedArrayBuffer(4*1024);
    let index = 0;
    const _makeTypedArray = (c, n) => {
      const result = new c(sab, index, n);
      index += result.byteLength;
      return result;
    };

    this.renderWidth = _makeTypedArray(Float32Array, 1);
    this.renderHeight = _makeTypedArray(Float32Array, 1);
    this.depthNear = _makeTypedArray(Float32Array, 1);
    this.depthNear[0] = 0.1;
    this.depthFar = _makeTypedArray(Float32Array, 1);
    this.depthFar[0] = 10000.0;
    this.position = _makeTypedArray(Float32Array, 3);
    this.orientation = _makeTypedArray(Float32Array, 4);
    this.orientation[3] = 1;
    this.leftViewMatrix = _makeTypedArray(Float32Array, 16);
    this.leftViewMatrix.set(Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]));
    this.rightViewMatrix = _makeTypedArray(Float32Array, 16);
    this.rightViewMatrix.set(this.leftViewMatrix);
    this.leftProjectionMatrix = _makeTypedArray(Float32Array, 16);
    this.leftProjectionMatrix.set(Float32Array.from([
      0.8000000000000002, 0, 0, 0,
      0, 1.0000000000000002, 0, 0,
      0, 0, -1.002002002002002, -1,
      0, 0, -0.20020020020020018, 0,
    ]));
    this.rightProjectionMatrix = _makeTypedArray(Float32Array, 16);
    this.rightProjectionMatrix.set(this.leftProjectionMatrix);
    this.leftOffset = _makeTypedArray(Float32Array, 3);
    this.leftOffset.set(Float32Array.from([-defaultEyeSeparation/2, 0, 0]));
    this.rightOffset = _makeTypedArray(Float32Array, 3);
    this.leftOffset.set(Float32Array.from([defaultEyeSeparation/2, 0, 0]));
    this.leftFov = _makeTypedArray(Float32Array, 4);
    this.leftFov.set(Float32Array.from([45, 45, 45, 45]));
    this.rightFov = _makeTypedArray(Float32Array, 4);
    this.rightFov.set(this.leftFov);
    const _makeGamepad = () => {
      return {
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
      };
    };
    this.gamepads = (() => {
      const result = Array(2+maxNumTrackers);
      for (let i = 0; i < result.length; i++) {
        result[i] = _makeGamepad();
      }
      return result;
    })();
    this.metrics = _makeTypedArray(Uint32Array, 2);
    this.devicePixelRatio = _makeTypedArray(Uint32Array, 1);
  }
}
const xrState = GlobalContext.xrState = new XRState();
if (nativeBindings.nativeOculusVR) {
  nativeBindings.nativeOculusVR.requestPresent = function (layers) {
    const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
    if (layer) {
      const canvas = layer.source;
      const window = canvas.ownerDocument.defaultView;

      if (!vrPresentState.glContext || (vrPresentState.glContext.canvas.ownerDocument.defaultView === window && vrPresentState.glContext !== canvas._context)) {
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }

        const windowHandle = context.getWindowHandle();
        nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);

        // fps = VR_FPS;

        const system = vrPresentState.oculusSystem || nativeBindings.nativeOculusVR.Oculus_Init();
        const lmContext = vrPresentState.lmContext || (nativeBindings.nativeLm && new nativeBindings.nativeLm());

        const {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;

        const cleanups = [];

        const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = system.CreateSwapChain(context, width, height);

        context.setDefaultFramebuffer(msFbo);

        vrPresentState.isPresenting = true;
        vrPresentState.oculusSystem = system;
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

            const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = system.CreateSwapChain(context, canvas.width, canvas.height);
            context.setDefaultFramebuffer(msFbo);
            vrPresentState.fbo = fbo;
            vrPresentState.tex = tex;
            vrPresentState.depthTex = depthTex;
            vrPresentState.msFbo = msFbo;
            vrPresentState.msTex = msTex;
            vrPresentState.msDepthTex = msDepthTex;
            canvas.framebuffer.fbo = fbo;
            canvas.framebuffer.tex = tex;
            canvas.framebuffer.depthTex = depthTex;
            canvas.framebuffer.msFbo = msFbo;
            canvas.framebuffer.msTex = msTex;
            canvas.framebuffer.msDepthTex = msDepthTex;
          }
        };
        canvas.on('attribute', _attribute);
        cleanups.push(() => {
          canvas.removeListener('attribute', _attribute);
        });

        /* window.top.updateVrFrame({
          renderWidth: xrState.renderWidth[0],
          renderHeight: xrState.renderHeight[0],
          force: true,
        }); */

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
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
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
  }
  nativeBindings.nativeOculusVR.exitPresent = function () {
    system.ExitPresent();

    return Promise.resolve();
  };
}
if (nativeBindings.nativeOpenVR) {
  nativeBindings.nativeOpenVR.requestPresent = function(layers) {
    const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
    if (layer) {
      const canvas = layer.source;
      const window = canvas.ownerDocument.defaultView;

      if (!vrPresentState.glContext || (vrPresentState.glContext.canvas.ownerDocument.defaultView === window && vrPresentState.glContext !== canvas._context)) {
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }

        const windowHandle = context.getWindowHandle();
        nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);

        // fps = VR_FPS;

        const vrContext = vrPresentState.vrContext || nativeBindings.nativeOpenVR.getContext();
        const system = vrPresentState.system || nativeBindings.nativeOpenVR.VR_Init(nativeBindings.nativeOpenVR.EVRApplicationType.Scene);
        const compositor = vrPresentState.compositor || vrContext.compositor.NewCompositor();

        const lmContext = vrPresentState.lmContext || (nativeBindings.nativeLm && new nativeBindings.nativeLm());

        let {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
        const MAX_TEXTURE_SIZE = 4096;
        const MAX_TEXTURE_SIZE_HALF = MAX_TEXTURE_SIZE/2;
        if (halfWidth > MAX_TEXTURE_SIZE_HALF) {
          const factor = halfWidth / MAX_TEXTURE_SIZE_HALF;
          halfWidth = MAX_TEXTURE_SIZE_HALF;
          height = Math.floor(height / factor);
        }
        const width = halfWidth * 2;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;

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

        /* window.top.updateVrFrame({
          renderWidth: xrState.renderWidth[0],
          renderHeight: xrState.renderHeight[0],
          force: true,
        }); */

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
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
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
  nativeBindings.nativeOpenVR.exitPresent = function() {
    if (vrPresentState.isPresenting) {
      nativeBindings.nativeOpenVR.VR_Shutdown();

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

const oculusMobileVrPresentState = {
  vrContext: null,
  isPresenting: false,
  glContext: null,
  msFbo: null,
  msTex: null,
  msDepthTex: null,
  fbo: null,
  tex: null,
  depthTex: null,
  cleanups: null,
  hasPose: false,
  layers: [],
};
GlobalContext.oculusMobileVrPresentState = oculusMobileVrPresentState;

if (nativeBindings.nativeOculusMobileVr) {
  nativeBindings.nativeOculusMobileVr.requestPresent = function (layers) {
    const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
    if (layer) {
      const canvas = layer.source;
      const window = canvas.ownerDocument.defaultView;

      if (!oculusMobileVrPresentState.glContext || (oculusMobileVrPresentState.glContext.canvas.ownerDocument.defaultView === window && oculusMobileVrPresentState.glContext !== canvas._context)) {
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }

        const windowHandle = context.getWindowHandle();
        nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);

        // fps = VR_FPS;

        const vrContext = oculusMobileVrPresentState.vrContext = oculusMobileVrPresentState.vrContext || nativeBindings.nativeOculusMobileVr.OculusMobile_Init(context.getWindowHandle());
        const {oculusMobileVrDisplay} = window[symbols.mrDisplaysSymbol];
        oculusMobileVrDisplay.id = nativeBindings.nativeOculusMobileVr.OculusMobile_GetDeviceType();
        const {width: halfWidth, height} = vrContext.GetRecommendedRenderTargetSize();
        const MAX_TEXTURE_SIZE = 4096;
        const MAX_TEXTURE_SIZE_HALF = MAX_TEXTURE_SIZE/2;
        if (halfWidth > MAX_TEXTURE_SIZE_HALF) {
          const factor = halfWidth / MAX_TEXTURE_SIZE_HALF;
          halfWidth = MAX_TEXTURE_SIZE_HALF;
          height = Math.floor(height / factor);
        }
        const width = halfWidth * 2;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;

        const cleanups = [];

        const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeBindings.nativeWindow.createRenderTarget(context, width, height, 0, 0, 0, 0);

        context.setDefaultFramebuffer(msFbo);

        oculusMobileVrPresentState.isPresenting = true;
        oculusMobileVrPresentState.vrContext = vrContext;
        oculusMobileVrPresentState.glContext = context;
        oculusMobileVrPresentState.msFbo = msFbo;
        oculusMobileVrPresentState.msTex = msTex;
        oculusMobileVrPresentState.msDepthTex = msDepthTex;
        oculusMobileVrPresentState.fbo = fbo;
        oculusMobileVrPresentState.tex = tex;
        oculusMobileVrPresentState.depthTex = depthTex;
        oculusMobileVrPresentState.cleanups = cleanups;

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

        /* window.top.updateVrFrame({
          renderWidth: xrState.renderWidth[0],
          renderHeight: xrState.renderHeight[0],
          force: true,
        }); */

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
        /* const {width: halfWidth, height} = oculusMobileVrPresentState.vrContext.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2; */

        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = oculusMobileVrPresentState;
        return {
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
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
  nativeBindings.nativeOculusMobileVr.exitPresent = function() {
    if (oculusMobileVrPresentState.isPresenting) {
      nativeBindings.nativeOculusMobileVr.OculusMobile_Shutdown();

      nativeBindings.nativeWindow.destroyRenderTarget(oculusMobileVrPresentState.msFbo, oculusMobileVrPresentState.msTex, oculusMobileVrPresentState.msDepthStencilTex);
      nativeBindings.nativeWindow.destroyRenderTarget(oculusMobileVrPresentState.fbo, oculusMobileVrPresentState.tex, oculusMobileVrPresentState.msDepthTex);

      const context = oculusMobileVrPresentState.glContext;
      nativeBindings.nativeWindow.setCurrentWindowContext(context.getWindowHandle());
      context.setDefaultFramebuffer(0);

      for (let i = 0; i < oculusMobileVrPresentState.cleanups.length; i++) {
        oculusMobileVrPresentState.cleanups[i]();
      }

      oculusMobileVrPresentState.isPresenting = false;
      oculusMobileVrPresentState.vrContext = null;
      oculusMobileVrPresentState.glContext = null;
      oculusMobileVrPresentState.msFbo = null;
      oculusMobileVrPresentState.msTex = null;
      oculusMobileVrPresentState.msDepthTex = null;
      oculusMobileVrPresentState.fbo = null;
      oculusMobileVrPresentState.tex = null;
      oculusMobileVrPresentState.depthTex = null;
      oculusMobileVrPresentState.cleanups = null;
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
      const window = canvas.ownerDocument.defaultView;

      if (!mlPresentState.mlGlContext || (mlPresentState.mlGlContext.canvas.ownerDocument.defaultView === window && mlPresentState.mlGlContext !== canvas._context)) {
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }

        const windowHandle = context.getWindowHandle();
        nativeBindings.nativeWindow.setCurrentWindowContext(windowHandle);

        // fps = ML_FPS;

        mlPresentState.mlContext.Present(windowHandle, context);

        const {width: halfWidth, height} = mlPresentState.mlContext.GetSize();
        const width = halfWidth * 2;

        const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeBindings.nativeWindow.createRenderTarget(context, width, height, 0, 0, 0, 0);
        mlPresentState.mlContext.SetContentTexture(tex);
        /* const {
          width: halfWidth,
          height,
          fbo,
          colorTex: tex,
          depthStencilTex: depthTex,
          msFbo,
          msColorTex: msTex,
          msDepthStencilTex: msDepthTex,
        } = initResult; */
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;

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

        context.setDefaultFramebuffer(msFbo);

        mlPresentState.mlGlContext = context;

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
        return {
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
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
  fakeVrDisplay: null,
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
      case 'windowResize': {
        const {width, height} = data;
        window.innerWidth = width;
        window.innerHeight = height;

        window.dispatchEvent(new window.Event('resize'));
        break;
      }
      /* case 'framebufferResize': {
        const {width, height} = data;
        innerWidth = width;
        innerHeight = height;

        window.innerWidth = innerWidth / window.devicePixelRatio;
        window.innerHeight = innerHeight / window.devicePixelRatio;
        window.dispatchEvent(new window.Event('resize'));
        break;
      } */
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

// let fps = DEFAULT_FPS;
const isMac = os.platform() === 'darwin';

const _startRenderLoop = () => {
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

        const isVisible = nativeWindow.isVisible(windowHandle) || vrPresentState.glContext === context || oculusMobileVrPresentState.glContext === context || mlPresentState.mlGlContext === context;
        if (isVisible) {
          const window = context.canvas.ownerDocument.defaultView;

          if (vrPresentState.glContext === context && vrPresentState.oculusSystem && vrPresentState.hasPose) {
            if (vrPresentState.layers.length > 0) {
              const {openVRDisplay} = window[symbols.mrDisplaysSymbol];
              _decorateModelViewProjections(vrPresentState.layers, openVRDisplay, 2); // note: openVRDisplay mirrors openVRDevice
              nativeWindow.composeLayers(context, vrPresentState.fbo, vrPresentState.layers);
            } else {
              nativeWindow.blitFrameBuffer(context, vrPresentState.msFbo, vrPresentState.fbo, vrPresentState.glContext.canvas.width, vrPresentState.glContext.canvas.height, vrPresentState.glContext.canvas.width, vrPresentState.glContext.canvas.height, true, false, false);
            }

            vrPresentState.oculusSystem.Submit();
            vrPresentState.hasPose = false;

            const width = vrPresentState.glContext.canvas.width * (args.blit ? 0.5 : 1);
            const height = vrPresentState.glContext.canvas.height;
            const {width: dWidth, height: dHeight} = nativeWindow.getFramebufferSize(windowHandle);
            nativeWindow.blitFrameBuffer(context, vrPresentState.msFbo, 0, width, height, dWidth, dHeight, true, false, false);
          } else if (vrPresentState.glContext === context && vrPresentState.system && vrPresentState.hasPose) {
            if (vrPresentState.layers.length > 0) {
              const {openVRDisplay} = window[symbols.mrDisplaysSymbol];
              _decorateModelViewProjections(vrPresentState.layers, openVRDisplay, 2); // note: openVRDisplay mirrors openVRDevice
              nativeWindow.composeLayers(context, vrPresentState.fbo, vrPresentState.layers);
            } else {
              nativeWindow.blitFrameBuffer(context, vrPresentState.msFbo, vrPresentState.fbo, vrPresentState.glContext.canvas.width, vrPresentState.glContext.canvas.height, vrPresentState.glContext.canvas.width, vrPresentState.glContext.canvas.height, true, false, false);
            }

            vrPresentState.compositor.Submit(context, vrPresentState.tex);
            vrPresentState.hasPose = false;

            const width = vrPresentState.glContext.canvas.width * (args.blit ? 0.5 : 1);
            const height = vrPresentState.glContext.canvas.height;
            const {width: dWidth, height: dHeight} = nativeWindow.getFramebufferSize(windowHandle);
            nativeWindow.blitFrameBuffer(context, vrPresentState.fbo, 0, width, height, dWidth, dHeight, true, false, false);
          } else if (oculusMobileVrPresentState.glContext === context && oculusMobileVrPresentState.hasPose) {
            if (oculusMobileVrPresentState.layers.length > 0) {
              const {oculusMobileVrDisplay} = window[symbols.mrDisplaysSymbol];
              _decorateModelViewProjections(oculusMobileVrPresentState.layers, vrDisplay, 2); // note: vrDisplay mirrors xrDisplay
              nativeWindow.composeLayers(context, oculusMobileVrPresentState.fbo, oculusMobileVrPresentState.layers);
            } else {
              nativeWindow.blitFrameBuffer(context, oculusMobileVrPresentState.msFbo, oculusMobileVrPresentState.fbo, oculusMobileVrPresentState.glContext.canvas.width, oculusMobileVrPresentState.glContext.canvas.height, oculusMobileVrPresentState.glContext.canvas.width, oculusMobileVrPresentState.glContext.canvas.height, true, false, false);
            }

            oculusMobileVrPresentState.vrContext.Submit(oculusMobileVrPresentState.glContext, oculusMobileVrPresentState.fbo, oculusMobileVrPresentState.glContext.canvas.width, oculusMobileVrPresentState.glContext.canvas.height);
            oculusMobileVrPresentState.hasPose = false;
          } else if (mlPresentState.mlGlContext === context && mlPresentState.mlHasPose) {
            if (mlPresentState.layers.length > 0) { // TODO: composition can be directly to the output texture array
              const {magicLeapDisplay} = window[symbols.mrDisplaysSymbol];
              _decorateModelViewProjections(mlPresentState.layers, magicLeapDisplay, 2); // note: magicLeapDisplay mirrors magicLeapDevice
              nativeWindow.composeLayers(context, mlPresentState.mlFbo, mlPresentState.layers);
            } else {
              nativeWindow.blitFrameBuffer(context, mlPresentState.mlMsFbo, mlPresentState.mlFbo, mlPresentState.mlGlContext.canvas.width, mlPresentState.mlGlContext.canvas.height, mlPresentState.mlGlContext.canvas.width, mlPresentState.mlGlContext.canvas.height, true, false, false);
            }

            mlPresentState.mlContext.SubmitFrame(mlPresentState.mlTex, mlPresentState.mlGlContext.canvas.width, mlPresentState.mlGlContext.canvas.height);
            mlPresentState.mlHasPose = false;
          } else {
            if (fakePresentState.layers.length > 0) {
              const {fakeVrDisplay} = window[symbols.mrDisplaysSymbol];
              _decorateModelViewProjections(fakePresentState.layers, fakeVrDisplay, 1);
              nativeWindow.composeLayers(context, context.framebuffer.fbo, fakePresentState.layers);
            } else {
              nativeWindow.blitFrameBuffer(context, context.framebuffer.msFbo, context.framebuffer.fbo, context.canvas.width, context.canvas.height, context.canvas.width, context.canvas.height, true, false, false);
            }

            const width = context.canvas.width * (args.blit ? 0.5 : 1);
            const height = context.canvas.height;
            const {width: dWidth, height: dHeight} = nativeWindow.getFramebufferSize(windowHandle);
            nativeWindow.blitFrameBuffer(context, context.framebuffer.fbo, 0, width, height, dWidth, dHeight, true, false, false);
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

        context.clearDirty();
      }
    }
  }

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
  const TIMESTAMP_FRAMES = 100;
  // let lastFrameTime = Date.now();

  const _renderLoop = async () => {
    if (args.performance) {
      if (timestamps.frames >= TIMESTAMP_FRAMES) {
        console.log(`${(TIMESTAMP_FRAMES/(timestamps.total/1000)).toFixed(0)} FPS | ${timestamps.idle/timestamps.total}ms idle | ${timestamps.wait}ms wait | ${timestamps.prepare}ms prepare | ${timestamps.events}ms events | ${timestamps.media}ms media | ${timestamps.user}ms user | ${timestamps.submit}ms submit`);

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

    if (vrPresentState.isPresenting) {
      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.wait += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }

      if (vrPresentState.oculusSystem) {
        // wait for frame
        await new Promise((accept, reject) => {
          vrPresentState.oculusSystem.GetPose(
            localPositionArray3,   // hmd position
            localQuaternionArray4, // hmd orientation
            localFloat32Array,     // left eye view matrix
            localFloat32Array2,    // left eye projection matrix
            localFloat32Array3,    // right eye view matrix
            localFloat32Array4,     // right eye projection matrix
            leftControllerPositionArray3, // left controller position.
            leftControllerQuaternionArray4, // left controller orientation.
            rightControllerPositionArray3, // right controller position.
            rightControllerQuaternionArray4, // right controller orientation.
            accept
          );
        });

        vrPresentState.hasPose = true;

        xrState.position = localPositionArray3;
        xrState.orientation = localQuaternionArray4;
        xrState.leftViewMatrix.set(localFloat32Array);
        xrState.leftProjectionMatrix.set(localFloat32Array2);
        xrState.rightViewMatrix.set(localFloat32Array3);
        xrState.rightProjectionMatrix.set(localFloat32Array4);

        localVector.toArray(xrState.position);
        localQuaternion.toArray(xrState.orientation);

        // Controllers.
        {
          const leftGamepad = xrState.gamepads[0];

          // Pose
          leftGamepad.position[0] = leftControllerPositionArray3[0];
          leftGamepad.position[1] = leftControllerPositionArray3[1];
          leftGamepad.position[2] = leftControllerPositionArray3[2];

          leftGamepad.orientation[0] = leftControllerQuaternionArray4[0];
          leftGamepad.orientation[1] = leftControllerQuaternionArray4[1];
          leftGamepad.orientation[2] = leftControllerQuaternionArray4[2];
          leftGamepad.orientation[3] = leftControllerQuaternionArray4[3];

          // Input
          vrPresentState.oculusSystem.GetControllersInputState(0, localGamepadArray);

          leftGamepad.connected[0] = localGamepadArray[0];

          // Pressed
          leftGamepad.buttons[0].pressed[0] = localGamepadArray[3]; // thumbstick
          leftGamepad.buttons[1].pressed[0] = localGamepadArray[5] >= 0.01; // trigger
          leftGamepad.buttons[2].pressed[0] = localGamepadArray[6] >= 0.01; // grip
          leftGamepad.buttons[3].pressed[0] = localGamepadArray[1] == 1; // xbutton
          leftGamepad.buttons[4].pressed[0] = localGamepadArray[2] == 1; // ybutton
          leftGamepad.buttons[5].pressed[0] = localGamepadArray[4] == 1; // menu

          // touched
          leftGamepad.buttons[0].touched[0] = localGamepadArray[9]; // thumbstick
          leftGamepad.buttons[1].touched[0] = localGamepadArray[10]; // trigger
          leftGamepad.buttons[3].touched[0] = localGamepadArray[7]; // xbutton
          leftGamepad.buttons[4].touched[0] = localGamepadArray[8]; // ybutton

          // thumbstick axis
          leftGamepad.axes[0] = localGamepadArray[11];
          leftGamepad.axes[1] = localGamepadArray[12];

          // values
          leftGamepad.buttons[1].value[0] = localGamepadArray[5]; // trigger
          leftGamepad.buttons[2].value[0] = localGamepadArray[6]; // grip
        }
        {
          const rightGamepad = xrState.gamepads[1];

          // Pose
          rightGamepad.position[0] = rightControllerPositionArray3[0];
          rightGamepad.position[1] = rightControllerPositionArray3[1];
          rightGamepad.position[2] = rightControllerPositionArray3[2];

          rightGamepad.orientation[0] = rightControllerQuaternionArray4[0];
          rightGamepad.orientation[1] = rightControllerQuaternionArray4[1];
          rightGamepad.orientation[2] = rightControllerQuaternionArray4[2];
          rightGamepad.orientation[3] = rightControllerQuaternionArray4[3];

          // Input
          vrPresentState.oculusSystem.GetControllersInputState(1, localGamepadArray);

          rightGamepad.connected[0] = localGamepadArray[0];

          // pressed
          rightGamepad.buttons[0].pressed[0] = localGamepadArray[3]; // thumbstick
          rightGamepad.buttons[1].pressed[0] = localGamepadArray[5] >= 0.1; // trigger
          rightGamepad.buttons[2].pressed[0] = localGamepadArray[6] >= 0.1; // grip
          rightGamepad.buttons[3].pressed[0] = localGamepadArray[1] == 1; // xbutton
          rightGamepad.buttons[4].pressed[0] = localGamepadArray[2] == 1; // ybutton
          rightGamepad.buttons[5].pressed[0] = localGamepadArray[4] == 1; // menu

          // touched
          rightGamepad.buttons[0].touched[0] = localGamepadArray[9]; // thumbstick
          rightGamepad.buttons[1].touched[0] = localGamepadArray[10]; // trigger
          rightGamepad.buttons[3].touched[0] = localGamepadArray[7]; // xbutton
          rightGamepad.buttons[4].touched[0] = localGamepadArray[8]; // ybutton

          // thumbstick axis
          rightGamepad.axes[0] = localGamepadArray[11];
          rightGamepad.axes[1] = localGamepadArray[12];

          // values
          rightGamepad.buttons[1].value[0] = localGamepadArray[5]; // trigger
          rightGamepad.buttons[2].value[0] = localGamepadArray[6]; // grip
        }
      } else if (vrPresentState.compositor) {
        // wait for frame
        await new Promise((accept, reject) => {
          vrPresentState.compositor.RequestGetPoses(
            vrPresentState.system,
            localFloat32PoseArray, // hmd, controllers, trackers
            accept
          );
        });
        if (!immediate) {
          return;
        }

        vrPresentState.hasPose = true;

        // build xr state
        const hmdMatrix = localMatrix.fromArray(localFloat32HmdPoseArray);

        hmdMatrix.decompose(localVector, localQuaternion, localVector2);
        localVector.toArray(xrState.position);
        localQuaternion.toArray(xrState.orientation);

        hmdMatrix.getInverse(hmdMatrix);

        vrPresentState.system.GetEyeToHeadTransform(0, localFloat32MatrixArray);
        localMatrix2.fromArray(localFloat32MatrixArray);
        localMatrix2.decompose(localVector, localQuaternion, localVector2);
        localVector.toArray(xrState.leftOffset);
        localMatrix2
          .getInverse(localMatrix2)
          .multiply(hmdMatrix);
        localMatrix2.toArray(xrState.leftViewMatrix);

        vrPresentState.system.GetProjectionMatrix(0, xrState.depthNear[0], xrState.depthFar[0], localFloat32MatrixArray);
        xrState.leftProjectionMatrix.set(localFloat32MatrixArray);

        vrPresentState.system.GetProjectionRaw(0, localFovArray);
        for (let i = 0; i < localFovArray.length; i++) {
          xrState.leftFov[i] = Math.atan(localFovArray[i]) / Math.PI * 180;
        }

        vrPresentState.system.GetEyeToHeadTransform(1, localFloat32MatrixArray);
        localMatrix2.fromArray(localFloat32MatrixArray);
        localMatrix2.decompose(localVector, localQuaternion, localVector2);
        localVector.toArray(xrState.rightOffset);
        localMatrix2
          .getInverse(localMatrix2)
          .multiply(hmdMatrix);
        localMatrix2.toArray(xrState.rightViewMatrix);

        vrPresentState.system.GetProjectionMatrix(1, xrState.depthNear[0], xrState.depthFar[0], localFloat32MatrixArray);
        xrState.rightProjectionMatrix.set(localFloat32MatrixArray);

        vrPresentState.system.GetProjectionRaw(1, localFovArray);
        for (let i = 0; i < localFovArray.length; i++) {
          xrState.rightFov[i] = Math.atan(localFovArray[i]) / Math.PI * 180;
        }

        // build stage parameters
        // vrPresentState.system.GetSeatedZeroPoseToStandingAbsoluteTrackingPose(localFloat32MatrixArray);
        // stageParameters.sittingToStandingTransform.set(localFloat32MatrixArray);

        // build gamepads data
        const _loadGamepad = i => {
          const gamepad = xrState.gamepads[i];
          if (vrPresentState.system.GetControllerState(i, localGamepadArray)) {
            gamepad.connected[0] = 1;

            localMatrix.fromArray(localFloat32GamepadPoseArrays[i]);
            localMatrix.decompose(localVector, localQuaternion, localVector2);
            localVector.toArray(gamepad.position);
            localQuaternion.toArray(gamepad.orientation);

            gamepad.buttons[0].pressed[0] = localGamepadArray[4]; // pad
            gamepad.buttons[1].pressed[0] = localGamepadArray[5]; // trigger
            gamepad.buttons[2].pressed[0] = localGamepadArray[3]; // grip
            gamepad.buttons[3].pressed[0] = localGamepadArray[2]; // menu
            gamepad.buttons[4].pressed[0] = localGamepadArray[1]; // system

            gamepad.buttons[0].touched[0] = localGamepadArray[9]; // pad
            gamepad.buttons[1].touched[0] = localGamepadArray[10]; // trigger
            gamepad.buttons[2].touched[0] = localGamepadArray[8]; // grip
            gamepad.buttons[3].touched[0] = localGamepadArray[7]; // menu
            gamepad.buttons[4].touched[0] = localGamepadArray[6]; // system

            for (let i = 0; i < 10; i++) {
              gamepad.axes[i] = localGamepadArray[11+i];
            }
            gamepad.buttons[1].value[0] = gamepad.axes[2]; // trigger
          } else {
            gamepad.connected[0] = 0;
          }
        };
        _loadGamepad(0);
        _loadGamepad(1);

        // build tracker data
        const _loadTracker = i => {
          const tracker = xrState.gamepads[2 + i];
          const trackerPoseArray = localFloat32TrackerPoseArrays[i];
          if (!isNaN(trackerPoseArray[0])) {
            tracker.connected[0] = 1;

            localMatrix.fromArray(trackerPoseArray);
            localMatrix.decompose(localVector, localQuaternion, localVector2);
            localVector.toArray(tracker.position);
            localQuaternion.toArray(tracker.orientation);
          } else {
            tracker.connected[0] = 0;
          }
        };
        for (let i = 0; i < maxNumTrackers; i++) {
          _loadTracker(i);
        }

        /* if (vrPresentState.lmContext) { // XXX remove this binding
          vrPresentState.lmContext.WaitGetPoses(handsArray);
        } */
      }

      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.prepare += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }
    } else if (oculusMobileVrPresentState.vrContext) {
      await new Promise((accept, reject) => {
        oculusMobileVrPresentState.hasPose = oculusMobileVrPresentState.vrContext.WaitGetPoses(
          oculusMobilePoseFloat32Array
        );

        accept();
      });

      // build hmd data
      let index = oculusMobilePoseFloat32Array.byteOffset;
      xrState.position.set(new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 3));
      index += 3*Float32Array.BYTES_PER_ELEMENT;
      xrState.orientation.set(new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 4));
      index += 4*Float32Array.BYTES_PER_ELEMENT;
      const ipd = new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 1)[0];
      xrState.leftOffset[0] = -ipd/2;
      xrState.rightOffset[0] = ipd/2;
      index += 1*Float32Array.BYTES_PER_ELEMENT;
      const fov = new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 4);
      xrState.leftFov.set(fov);
      xrState.rightFov.set(fov);
      index += 4*Float32Array.BYTES_PER_ELEMENT;
      xrState.leftViewMatrix.set(new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 16));
      index += 16*Float32Array.BYTES_PER_ELEMENT;
      xrState.rightViewMatrix.set(new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 16));
      index += 16*Float32Array.BYTES_PER_ELEMENT;
      xrState.leftProjectionMatrix.set(new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 16));
      index += 16*Float32Array.BYTES_PER_ELEMENT;
      xrState.rightProjectionMatrix.set(new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 16));
      index += 16*Float32Array.BYTES_PER_ELEMENT;

      // build gamepads data
      {
        const leftGamepad = xrState.gamepads[0];
        const gamepadFloat32Array = new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 16);
        index += 16*Float32Array.BYTES_PER_ELEMENT;
        const buttonsFloat32Array = new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 12);
        index += 12*Float32Array.BYTES_PER_ELEMENT;
        if (!isNaN(gamepadFloat32Array[0])) {
          leftGamepad.connected[0] = true;

          localMatrix.fromArray(gamepadFloat32Array);
          localMatrix.decompose(localVector, localQuaternion, localVector2);
          localVector.toArray(leftGamepad.position);
          localQuaternion.toArray(leftGamepad.orientation);

          // pressed
          leftGamepad.buttons[0].pressed[0] = buttonsFloat32Array[2]; // thumbstick
          leftGamepad.buttons[1].pressed[0] = buttonsFloat32Array[4] >= 0.1; // trigger
          leftGamepad.buttons[2].pressed[0] = buttonsFloat32Array[5] >= 0.1; // grip
          leftGamepad.buttons[3].pressed[0] = buttonsFloat32Array[0] == 1; // xbutton
          leftGamepad.buttons[4].pressed[0] = buttonsFloat32Array[1] == 1; // ybutton
          leftGamepad.buttons[5].pressed[0] = buttonsFloat32Array[3] == 1; // menu

          // touched
          leftGamepad.buttons[0].touched[0] = buttonsFloat32Array[8]; // thumbstick
          leftGamepad.buttons[1].touched[0] = buttonsFloat32Array[9]; // trigger
          leftGamepad.buttons[3].touched[0] = buttonsFloat32Array[6]; // xbutton
          leftGamepad.buttons[4].touched[0] = buttonsFloat32Array[7]; // ybutton

          // thumbstick axis
          leftGamepad.axes[0] = buttonsFloat32Array[10];
          leftGamepad.axes[1] = buttonsFloat32Array[11];

          // values
          leftGamepad.buttons[1].value[0] = buttonsFloat32Array[4]; // trigger
          leftGamepad.buttons[2].value[0] = buttonsFloat32Array[5]; // grip
        } else {
          leftGamepad.connected[0] = 0;
        }
      }
      {
        const rightGamepad = xrState.gamepads[1];
        const gamepadFloat32Array = new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 16);
        index += 16*Float32Array.BYTES_PER_ELEMENT;
        const buttonsFloat32Array = new Float32Array(oculusMobilePoseFloat32Array.buffer, index, 12);
        index += 12*Float32Array.BYTES_PER_ELEMENT;
        if (!isNaN(gamepadFloat32Array[0])) {
          rightGamepad.connected[0] = true;

          localMatrix.fromArray(gamepadFloat32Array);
          localMatrix.decompose(localVector, localQuaternion, localVector2);
          localVector.toArray(rightGamepad.position);
          localQuaternion.toArray(rightGamepad.orientation);

          // pressed
          rightGamepad.buttons[0].pressed[0] = buttonsFloat32Array[2]; // thumbstick
          rightGamepad.buttons[1].pressed[0] = buttonsFloat32Array[4] >= 0.1; // trigger
          rightGamepad.buttons[2].pressed[0] = buttonsFloat32Array[5] >= 0.1; // grip
          rightGamepad.buttons[3].pressed[0] = buttonsFloat32Array[0] == 1; // xbutton
          rightGamepad.buttons[4].pressed[0] = buttonsFloat32Array[1] == 1; // ybutton
          rightGamepad.buttons[5].pressed[0] = buttonsFloat32Array[3] == 1; // menu

          // touched
          rightGamepad.buttons[0].touched[0] = buttonsFloat32Array[8]; // thumbstick
          rightGamepad.buttons[1].touched[0] = buttonsFloat32Array[9]; // trigger
          rightGamepad.buttons[3].touched[0] = buttonsFloat32Array[6]; // xbutton
          rightGamepad.buttons[4].touched[0] = buttonsFloat32Array[7]; // ybutton

          // thumbstick axis
          rightGamepad.axes[0] = buttonsFloat32Array[10];
          rightGamepad.axes[1] = buttonsFloat32Array[11];

          // values
          rightGamepad.buttons[1].value[0] = buttonsFloat32Array[4]; // trigger
          rightGamepad.buttons[2].value[0] = buttonsFloat32Array[5]; // grip
        } else {
          rightGamepad.connected[0] = 0;
        }
      }

      /* vrPresentState.system.GetProjectionRaw(0, localFovArray);
      for (let i = 0; i < localFovArray.length; i++) {
        xrState.leftFov[i] = Math.atan(localFovArray[i]) / Math.PI * 180;
      } */
    } else if (mlPresentState.mlGlContext) {
      mlPresentState.mlHasPose = await new Promise((accept, reject) => {
        mlPresentState.mlContext.RequestGetPoses(
          transformArray,
          projectionArray,
          controllersArray,
          accept
        );
      });
      if (!immediate) {
        return;
      }

      mlPresentState.mlContext.PrepareFrame(
        mlPresentState.mlGlContext,
        mlPresentState.mlMsFbo,
        xrState.renderWidth[0]*2,
        xrState.renderHeight[0],
      );

      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.wait += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }

      if (mlPresentState.mlHasPose) {
        localVector.fromArray(transformArray, 0);
        localQuaternion.fromArray(transformArray, 3);
        localVector2.set(1, 1, 1);
        localMatrix.compose(localVector, localQuaternion, localVector2).getInverse(localMatrix);
        localVector.toArray(xrState.position);
        localQuaternion.toArray(xrState.orientation);
        localMatrix.toArray(xrState.leftViewMatrix);
        xrState.leftProjectionMatrix.set(projectionArray.slice(0, 16));

        localVector.fromArray(transformArray, 3 + 4);
        localQuaternion.fromArray(transformArray, 3 + 4 + 3);
        // localVector2.set(1, 1, 1);
        localMatrix.compose(localVector, localQuaternion, localVector2).getInverse(localMatrix);
        localMatrix.toArray(xrState.rightViewMatrix);
        xrState.rightProjectionMatrix.set(projectionArray.slice(16, 32));

        let controllersArrayIndex = 0;
        {
          const leftGamepad = xrState.gamepads[0];
          leftGamepad.connected[0] = controllersArray[controllersArrayIndex];
          controllersArrayIndex++;
          leftGamepad.position.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 3));
          controllersArrayIndex += 3;
          leftGamepad.orientation.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 4));
          controllersArrayIndex += 4;
          const leftTriggerValue = controllersArray[controllersArrayIndex];
          leftGamepad.buttons[1].value[0] = leftTriggerValue;
          const leftTriggerPushed = leftTriggerValue > 0.5 ? 1 : 0;
          leftGamepad.buttons[1].touched[0] = leftTriggerPushed;
          leftGamepad.buttons[1].pressed[0] = leftTriggerPushed;
          leftGamepad.axes[2] = leftTriggerValue;
          controllersArrayIndex++;
          const leftBumperValue = controllersArray[controllersArrayIndex];
          leftGamepad.buttons[2].value[0] = leftBumperValue;
          const leftBumperPushed = leftBumperValue > 0.5 ? 1 : 0;
          leftGamepad.buttons[2].touched[0] = leftBumperPushed;
          leftGamepad.buttons[2].pressed[0] = leftBumperPushed;
          controllersArrayIndex++;
          const leftHomeValue = controllersArray[controllersArrayIndex];
          leftGamepad.buttons[3].value[0] = leftHomeValue;
          const leftHomePushed = leftHomeValue > 0.5 ? 1 : 0;
          leftGamepad.buttons[3].touched[0] = leftHomePushed;
          leftGamepad.buttons[3].pressed[0] = leftHomePushed;
          controllersArrayIndex++;
          leftGamepad.axes[0] = controllersArray[controllersArrayIndex];
          leftGamepad.axes[1] = controllersArray[controllersArrayIndex + 1];
          const leftPadValue = controllersArray[controllersArrayIndex + 2];
          leftGamepad.buttons[0].value[0] = leftPadValue;
          const leftPadTouched = leftPadValue > 0 ? 1 : 0;
          const leftPadPushed = leftPadValue > 0.5 ? 1: 0;
          leftGamepad.buttons[0].touched[0] = leftPadTouched;
          leftGamepad.buttons[0].pressed[0] = leftPadPushed;
          controllersArrayIndex += 3;
        }
        {
          const rightGamepad = xrState.gamepads[1];
          rightGamepad.connected[0] = controllersArray[controllersArrayIndex];
          controllersArrayIndex++;
          rightGamepad.position.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 3));
          controllersArrayIndex += 3;
          rightGamepad.orientation.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 4));
          controllersArrayIndex += 4;
          const rightTriggerValue = controllersArray[controllersArrayIndex];
          rightGamepad.buttons[1].value[0] = rightTriggerValue;
          const rightTriggerPushed = rightTriggerValue > 0.5 ? 1 : 0;
          rightGamepad.buttons[1].touched[0] = rightTriggerPushed;
          rightGamepad.buttons[1].pressed[0] = rightTriggerPushed;
          rightGamepad.axes[2] = rightTriggerValue;
          controllersArrayIndex++;
          const rightBumperValue = controllersArray[controllersArrayIndex];
          rightGamepad.buttons[2].value[0] = rightBumperValue;
          const rightBumperPushed = rightBumperValue > 0.5 ? 1 : 0;
          rightGamepad.buttons[2].touched[0] = rightBumperPushed;
          rightGamepad.buttons[2].pressed[0] = rightBumperPushed;
          controllersArrayIndex++;
          const rightHomeValue = controllersArray[controllersArrayIndex];
          rightGamepad.buttons[3].value[0] = rightHomeValue;
          const rightHomePushed = rightHomeValue > 0.5 ? 1 : 0;
          rightGamepad.buttons[3].touched[0] = rightHomePushed;
          rightGamepad.buttons[3].pressed[0] = rightHomePushed;
          controllersArrayIndex++;
          rightGamepad.axes[0] = controllersArray[controllersArrayIndex];
          rightGamepad.axes[1] = controllersArray[controllersArrayIndex + 1];
          const rightPadValue = controllersArray[controllersArrayIndex + 2];
          rightGamepad.buttons[0].value[0] = rightPadValue;
          const rightPadTouched = rightPadValue > 0 ? 1 : 0;
          const rightPadPushed = rightPadValue > 0.5 ? 1 : 0;
          rightGamepad.buttons[0].touched[0] = rightPadTouched;
          rightGamepad.buttons[0].pressed[0] = rightPadPushed;
          controllersArrayIndex += 3;
        }

        /* window.top.updateVrFrame({
          // stageParameters,
          gamepads,
          context: mlPresentState.mlContext,
        }); */
      }

      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.prepare += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }
    } else {
      /* await new Promise((accept, reject) => {
        const now = Date.now();
        const timeDiff = now - lastFrameTime;
        const waitTime = Math.max(8 - timeDiff, 0);
        setTimeout(accept, waitTime);
      });
      if (!immediate) {
        return;
      } */

      if (args.performance) {
        const now = Date.now();
        const diff = now - timestamps.last;
        timestamps.wait += diff;
        timestamps.total += diff;
        timestamps.last = now;
      }
    }

    // XXX this only needs to happen at the top level
    // compute derived gamepads data
    for (let i = 0; i < xrState.gamepads.length; i++) {
      const gamepad = xrState.gamepads[i];
      localQuaternion.fromArray(gamepad.orientation);
      localVector
        .set(0, 0, -1)
        .applyQuaternion(localQuaternion)
        .toArray(gamepad.direction);
      localVector.fromArray(gamepad.position);
      localVector2.set(1, 1, 1);
      localMatrix
        .compose(localVector, localQuaternion, localVector2)
        .toArray(gamepad.transformMatrix);
    }

    // poll xr device events
    fakePresentState.fakeVrDisplay && fakePresentState.fakeVrDisplay.update();
    for (let i = 0; i < windows.length; i++) {
      const window = windows[i];
      window[symbols.mrDisplaysSymbol].oculusVRDevice.session && window[symbols.mrDisplaysSymbol].oculusVRDevice.session.update();
      window[symbols.mrDisplaysSymbol].openVRDevice.session && window[symbols.mrDisplaysSymbol].openVRDevice.session.update();
      window[symbols.mrDisplaysSymbol].oculusMobileVrDevice.session && window[symbols.mrDisplaysSymbol].oculusMobileVrDevice.session.update();
      window[symbols.mrDisplaysSymbol].magicLeapARDevice.session && window[symbols.mrDisplaysSymbol].magicLeapARDevice.session.update();
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
    // update magic leap state
    if (mlPresentState.mlGlContext) {
      nativeBindings.nativeMl.Update(mlPresentState.mlContext, mlPresentState.mlGlContext);
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
    for (let i = 0; i < windows.length; i++) {
      windows[i].tickAnimationFrame();
    }
    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.user += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    _blit();

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
    immediate = setImmediate(_renderLoop);
  };
  let immediate = setImmediate(_renderLoop);

  return {
    stop() {
      clearImmediate(immediate);
      immediate = null;
    },
  };
};
let currentRenderLoop = _startRenderLoop();

const _bindWindow = (window, newWindowCb) => {
  // window.innerWidth = innerWidth;
  // window.innerHeight = innerHeight;

  window.on('navigate', newWindowCb);
  window.document.on('paste', e => {
    e.clipboardData = new window.DataTransfer();
    const clipboardContents = nativeWindow.getClipboard().slice(0, 256);
    const dataTransferItem = new window.DataTransferItem('string', 'text/plain', clipboardContents);
    e.clipboardData.items.push(dataTransferItem);
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
  });
  window.addEventListener('error', err => {
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
    _bindReplWindow(core.make('', {
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
