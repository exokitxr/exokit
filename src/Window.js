/* const zeroMatrix = new THREE.Matrix4();
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

        const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeWindow.createRenderTarget(context, width, height, 0, 0, 0, 0);

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
          msTex,
          msDepthTex,
          tex: 0,
          depthTex: 0,
        };

        const _attribute = (name, value) => {
          if (name === 'width' || name === 'height') {
            nativeWindow.setCurrentWindowContext(windowHandle);

            nativeWindow.resizeRenderTarget(context, canvas.width, canvas.height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
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
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };
      } else {
        throw new Error('no HTMLCanvasElement source provided');
      }
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
if (nativeMl) {
  mlPresentState.mlContext = new nativeMl();
  nativeMl.requestPresent = function(layers) {
    if (!mlPresentState.mlGlContext) {
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
            msTex,
            msDepthTex,
            tex,
            depthTex,
          };

          const cleanups = [];
          mlPresentState.mlCleanups = cleanups;

          const _attribute = (name, value) => {
            if (name === 'width' || name === 'height') {
              nativeWindow.setCurrentWindowContext(windowHandle);

              nativeWindow.resizeRenderTarget(context, canvas.width, canvas.height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
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
          throw new Error('failed to present ml context');
        }
      } else {
        throw new Error('no HTMLCanvasElement source provided');
      }
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
  };
  nativeMl.exitPresent = function() {
    nativeWindow.destroyRenderTarget(mlPresentState.mlMsFbo, mlPresentState.mlMsTex, mlPresentState.mlMsDepthTex);
    nativeWindow.destroyRenderTarget(mlPresentState.mlFbo, mlPresentState.mlTex, mlPresentState.mlDepthTex);

    nativeWindow.setCurrentWindowContext(mlPresentState.mlGlContext.getWindowHandle());
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
        nativeMl.DeinitLifecycle();
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
  if (!nativeMl.IsSimulated()) {
    nativeMl.InitLifecycle(_mlLifecycleEvent, _mlKeyboardEvent);
  } else {
    // try to connect to MLSDK
    const s = net.connect(MLSDK_PORT, '127.0.0.1', () => {
      s.destroy();

      nativeMl.InitLifecycle(_mlLifecycleEvent, _mlKeyboardEvent);
    });
    s.on('error', () => {});
  }
} */

(() => {

const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const fs = require('fs');
const url = require('url');
const http = require('http');
const https = require('https');
const ws = require('ws');
const os = require('os');
const util = require('util');
const {URL} = url;
const {TextEncoder, TextDecoder} = util;
const {performance} = require('perf_hooks');

const {XMLHttpRequest: XMLHttpRequestBase, FormData} = require('window-xhr');

const fetch = require('window-fetch');
const {Request, Response, Headers, Blob} = fetch;

const WebSocket = require('ws/lib/websocket');

const {LocalStorage} = require('node-localstorage');
const indexedDB = require('fake-indexeddb');
const parseXml = require('@rgrove/parse-xml');

let {
  nativeWindow,
  nativeGl,
  nativeGl2,
  nativeCanvasRenderingContext2D,
  nativeImage,
  nativeImageData,
  nativeImageBitmap,
  nativeAudio,
  nativeVideo,
  nativePath2D,
  nativeCanvasGradient,
  nativeVr,
  nativeMl,
} = require('./native-bindings.js');

const {
  MRDisplay,
  VRDisplay,
  FakeVRDisplay,
  VRFrameData,
  VRPose,
  VRStageParameters,
  Gamepad,
  GamepadButton,
  getGamepads,
  getAllGamepads,
} = require('./VR.js');

const {defaultCanvasSize} = require('./constants');
const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');
const {urls} = require('./urls');

// Class imports.
const {Document, DocumentFragment, DocumentType, DOMImplementation, initDocument} = require('./Document');
const DOM = require('./DOM');
const {DOMRect, Node, NodeList} = require('./DOM');
const {CustomEvent, DragEvent, ErrorEvent, Event, EventTarget, KeyboardEvent, MessageEvent, MouseEvent, WheelEvent, PromiseRejectionEvent} = require('./Event');
const {History} = require('./History');
const {Location} = require('./Location');
const {XMLHttpRequest} = require('./Network');
const XR = require('./XR');
const utils = require('./utils');
const {_elementGetter, _elementSetter} = require('./utils');

const btoa = s => Buffer.from(s, 'binary').toString('base64');
const atob = s => Buffer.from(s, 'base64').toString('binary');

class CustomElementRegistry {
  constructor(window) {
    this._window = window;

    this.elements = {};
    this.elementPromises = {};
  }

  define(name, constructor, options) {
    name = name.toUpperCase();

    this.elements[name] = constructor;

    this._window.document.traverse(el => {
      if (el.tagName === name) {
        this.upgrade(el, constructor);
      }
    });

    const promises = this.elementPromises[name];
    if (promises) {
      for (let i = 0; i < promises.length; i++) {
        promises[i].accept();
      }
      this.elementPromises[name] = null;
    }
  }
  get(name) {
    name = name.toUpperCase();

    return this.elements[name];
  }
  whenDefined(name) {
    name = name.toUpperCase();

    if (this.elements[name]) {
      return Promise.resolve();
    } else {
      let promises = this.elementPromises[name];
      if (!promises) {
        promises = [];
        this.elementPromises[name] = promises;
      }
      const promise = new Promise((accept, reject) => {
        promise.accept = accept;
        promise.reject = reject;
      });
      promises.push(promise);
      return promise;
    }
  }

  upgrade(el, constructor) {
    Object.setPrototypeOf(el, constructor.prototype);
    constructor.call(el);
  }
}

let nativeVm = GlobalContext.nativeVm = null;

class MonitorManager {
  getList() {
    return nativeWindow.getMonitors();
  }

  select(index) {
    nativeWindow.setMonitor(index);
  }
}

class Screen {
  constructor(window) {
    this._window = window;
  }

  get top() {
    return 0;
  }
  set top(top) {}
  get left() {
    return 0;
  }
  set left(left) {}
  get width() {
    return this._window.innerWidth;
  }
  set width(width) {}
  get height() {
    return this._window.innerHeight;
  }
  set height(height) {}
  get colorDepth() {
    return 24;
  }
  set colorDepth(colorDepth) {}
  get orientation() {
    return {
      angle: 0,
      type: 'landscape-primary',
      onchange: null,
    };
  }
  set orientation(orientation) {}

  get pixelDepth() {
    return this.colorDepth;
  }
  set pixelDepth(pixelDepth) {}
  get availTop() {
    return this.top;
  }
  set availTop(availTop) {}
  get availLeft() {
    return this.left;
  }
  set availLeft(availLeft) {}
  get availWidth() {
    return this.width;
  }
  set availWidth(availWidth) {}
  get availHeight() {
    return this.height;
  }
  set availHeight(availHeight) {}
}

const handEntrySize = (1 + (5 * 5)) * (3 + 3);
const maxNumPlanes = 32 * 3;
const planeEntrySize = 3 + 4 + 2 + 1;
VRFrameData.nonstandard = {
  init() {
    this.hands = [
      new Float32Array(handEntrySize),
      new Float32Array(handEntrySize),
    ];
    this.planes = new Float32Array(maxNumPlanes * planeEntrySize);
    this.numPlanes = 0;
  },
  copy(frameData) {
    for (let i = 0; i < this.hands.length; i++) {
      this.hands[i].set(frameData.hands[i]);
    }
    this.planes.set(frameData.planes);
    this.numPlanes = frameData.numPlanes;
  },
};
class GamepadGesture {
  constructor() {
    this.position = new Float32Array(3);
    this.gesture = null;
  }

  copy(gesture) {
    this.position.set(gesture.position);
    this.gesture = gesture.gesture;
  }
}
Gamepad.nonstandard = {
  init() {
    this.gesture = new GamepadGesture();
  },
  copy(gamepad) {
    this.gesture.copy(gamepad.gesture);
  },
};

class MLDisplay extends MRDisplay {
  constructor() {
    super('ML');

    this._frameData = new VRFrameData();
    this._planesArray = new Float32Array(maxNumPlanes * planeEntrySize);
    this._numPlanes = 0;
    this._context = null;
  }

  requestPresent(layers) {
    if (this.onrequestpresent) {
      this.onrequestpresent(layers);
    }

    this.isPresenting = true;

    if (this.onvrdisplaypresentchange) {
      this.onvrdisplaypresentchange();
    }

    return Promise.resolve();
  }

  exitPresent() {
    return (this.onexitpresent ? this.onexitpresent() : Promise.resolve())
      .then(() => {
        this.isPresenting = false;

        for (let i = 0; i < this._rafs.length; i++) {
          this.cancelAnimationFrame(this._rafs[i]);
        }
        this._rafs.length = 0;

        if (this.onvrdisplaypresentchange) {
          this.onvrdisplaypresentchange();
        }
      });
  }

  getFrameData(frameData) {
    frameData.copy(this._frameData);

    if (frameData.planes) {
      frameData.planes.set(this._planesArray);
      frameData.numPlanes = this._numPlanes;
    }
  }

  getGeometry(positions, normals, indices, metrics) {
    if (this._context) {
      return this._context.stageGeometry.getGeometry(positions, normals, indices, metrics);
    } else {
      return 0;
    }
  }

  update(update) {
    const {
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
      handsArray,
    } = update;

    if (depthNear !== undefined) {
      this.depthNear = depthNear;
    }
    if (depthFar !== undefined) {
      this.depthFar = depthFar;
    }
    if (renderWidth !== undefined) {
      this._width = renderWidth;
    }
    if (renderHeight !== undefined) {
      this._height = renderHeight;
    }
    if (leftOffset !== undefined) {
      this._leftOffset.set(leftOffset);
    }
    if (leftFov !== undefined) {
      this._leftFov.set(leftFov);
    }
    if (rightOffset !== undefined) {
      this._rightOffset.set(rightOffset);
    }
    if (rightFov !== undefined) {
      this._rightFov.set(rightFov);
    }
    if (frameData !== undefined) {
      this._frameData.copy(frameData);
    }
    if (update.planesArray !== undefined) {
      this._planesArray.set(update.planesArray);
    }
    if (update.numPlanes !== undefined) {
      this._numPlanes = update.numPlanes;
    }
    if (update.context !== undefined) {
      this._context = update.context;
    }
  }
}

if (global.args.frame || global.args.minimalFrame) {
  nativeGl = (OldWebGLRenderingContext => {
    function WebGLRenderingContext() {
      const result = Reflect.construct(OldWebGLRenderingContext, arguments);
      for (const k in result) {
        if (typeof result[k] === 'function') {
          result[k] = (old => function() {
            if (global.args.frame) {
              console.log(k, arguments);
            } else if (global.args.minimalFrame) {
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
  })(nativeGl);
}
const _onGlContextConstruct = (gl, canvas) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  const windowSpec = (() => {
    try {
      const visible = !args.image && canvas.ownerDocument.documentElement.contains(canvas);
      const {hidden} = canvas.ownerDocument;
      const firstWindowHandle = contexts.length > 0 ? contexts[0].getWindowHandle() : null;
      return nativeWindow.create3d(canvasWidth, canvasHeight, visible && !hidden, firstWindowHandle, gl);
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
        msTex,
        msDepthTex,
        tex,
        depthTex,
      };
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

    gl.destroy = (destroy => function() {
      destroy.call(this);

      nativeWindow.setCurrentWindowContext(windowHandle);

      if (gl === vrPresentState.glContext) {
        nativeVr.VR_Shutdown();

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

    contexts.push(gl);
    fps = nativeWindow.getRefreshRate();

    canvas.ownerDocument.defaultView.on('unload', () => {
      gl.destroy();
    });
  } else {
    throw new Error('WebGLRenderingContext failed to create backing context');
  }
};
class WebGLRenderingContext extends nativeGl {
  constructor(canvas) {
    super();
    _onGlContextConstruct(this, canvas);
  }
}
class WebGL2RenderingContext extends nativeGl2 {
  constructor(canvas) {
    super();
    _onGlContextConstruct(this, canvas);
  }
}

const _onCanvasContextConstruct = (ctx, canvas) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  const windowSpec = (() => {
    try {
      const firstWindowHandle = contexts.length > 0 ? contexts[0].getWindowHandle() : null;
      return nativeWindow.create2d(canvasWidth, canvasHeight, firstWindowHandle);
    } catch (err) {
      console.warn(err.message);
      return null;
    }
  })();

  if (windowSpec) {
    const [windowHandle, tex] = windowSpec;

    ctx.setWindowHandle(windowHandle);
    ctx.setTexture(tex, canvasWidth, canvasHeight);
    
    ctx.canvas = canvas;
    
    contexts.push(ctx);
    
    ctx.destroy = (destroy => function() {
      destroy.call(this);
      
      nativeWindow.destroy(windowHandle);
      canvas._context = null;
      
      contexts.splice(contexts.indexOf(ctx), 1);
    })(ctx.destroy);
  } else {
    throw new Error('CanvasRenderingContext2D failed to create backing context');
  }
};
class CanvasRenderingContext2D extends nativeCanvasRenderingContext2D {
  constructor(canvas) {
    super();
    _onCanvasContextConstruct(this, canvas);
  }
}

class AudioContext extends nativeAudio.AudioContext {
  /**
   * Wrap AudioContext.DecodeAudioDataSync binding with promises and callbacks.
   */
  decodeAudioData(arrayBuffer, successCallback, errorCallback) {
    return new Promise((resolve, reject) => {
      try {
        let audioBuffer = this._decodeAudioDataSync(arrayBuffer);
        if (successCallback) {
          process.nextTick(() => {
            try {
              successCallback(audioBuffer);
            } catch(err) {
              console.warn(err);
            }
          });
        }
        resolve(audioBuffer);
      } catch(err) {
        console.warn(err);
        if (errorCallback) {
          process.nextTick(() => {
            try {
              errorCallback(err);
            } catch(err) {
              console.warn(err);
            }
          });
        }
        reject(err);
      }
    });
  }
}

class MediaRecorder extends EventEmitter {
  constructor() {
    super();
  }

  start() {}

  stop() {}

  requestData() {}
}

class DataTransfer {
  constructor({items = [], files = []} = {}) {
    this.items = items;
    this.files = files;
  }
}
class DataTransferItem {
  constructor(kind = 'string', type = 'text/plain', data = null) {
    this.kind = kind;
    this.type = type;
    this.data = data;
  }

  getAsFile() {
    return new Blob([this.data], {
      type: this.type,
    });
  }

  getAsString(callback) {
    const {data} = this;
    setImmediate(() => {
      callback(data);
    });
  }
}

class FileReader extends EventTarget {
  constructor() {
    super();

    this.result = null;
  }

  get onload() {
    return _elementGetter(this, 'load');
  }
  set onload(onload) {
    _elementSetter(this, 'load', onload);
  }

  get onerror() {
    return _elementGetter(this, 'error');
  }
  set onerror(onerror) {
    _elementSetter(this, 'error', onerror);
  }

  readAsArrayBuffer(file) {
    this.result = file.buffer.buffer.slice(file.buffer.byteOffset, file.buffer.byteOffset + file.buffer.byteLength);

    process.nextTick(() => {
      this.dispatchEvent(new Event('load', {
        target: this,
      }));
    });
  }

  readAsDataURL(file) {
    this.result = 'data:' + file.type + ';base64,' + file.buffer.toString('base64');

    process.nextTick(() => {
      this.dispatchEvent(new Event('load', {
        target: this,
      }));
    });
  }

  readAsText(file) {
    this.result = file.buffer.toString('utf8');

    process.nextTick(() => {
      this.dispatchEvent(new Event('load', {
        target: this,
      }));
    });
  }
}

class Worker {
  constructor(src) {
    if (src instanceof Blob) {
      src = 'data:application/javascript,' + src.buffer.toString('utf8');
    } else {
      const blob = urls.get(src);

      if (blob) {
        src = 'data:application/octet-stream;base64,' + blob.buffer.toString('base64');
      } else {
        src = _normalizeUrl(src);
      }
    }

    this.worker = vmOne.make({
      initModule: path.join(__dirname, 'Worker.js'),
      args: {
        src,
      },
    });
  }

  postMessage(message, transferList) {
    this.worker.postMessage(message, transferList);
  }

  get onmessage() {
    return this.worker.onmessage;
  }
  set onmessage(onmessage) {
    this.worker.onmessage = onmessage;
  }
  get onerror() {
    return this.worker.onerror;
  }
  set onerror(onerror) {
    this.worker.onerror = onerror;
  }
}

let rafCbs = [];
let rafIndex = 0;
function tickAnimationFrame(timestamp) {
  for (let i = 0; i < rafCbs.length; i++) {
    const rafCb = rafCbs[i];

    if (rafCb) {
      rafCb(timestamp);
    }
  }
}

const _updateGamepads = newGamepads => {
  if (newGamepads !== undefined) {
    const gamepads = getGamepads();
    const allGamepads = getAllGamepads();

    if (newGamepads[0]) {
      gamepads[0] = allGamepads[0];
      gamepads[0].copy(newGamepads[0]);
    } else {
      gamepads[0] = null;
    }
    if (newGamepads[1]) {
      gamepads[1] = allGamepads[1];
      gamepads[1].copy(newGamepads[1]);
    } else {
      gamepads[1] = null;
    }
  }
};
function updateXrFrame(update) {
  let updatedHmd = false;
  if (vrDisplay.isPresenting || update.force) {
    vrDisplay.update(update);
    updatedHmd = true;
  }
  if (xrDisplay.session || update.force) {
    xrDisplay.update(update);
    updatedHmd = true;
  }
  if (mlDisplay.isPresenting || update.force) {
    mlDisplay.update(update);
    updatedHmd = true;
  }
  if (xmDisplay.session || update.force) {
    xmDisplay.update(update);
    updatedHmd = true;
  }
  if (updatedHmd) {
    _updateGamepads(update.gamepads);
  }
}

global.on('message', m => {
  switch (m.method) {
    case 'tickAnimationFrame': {
      tickAnimationFrame(m.timestamp);
      break;
    }
    case 'updateXrFrame': {
      updateXrFrame(m.update);
      break;
    }
    default: throw new Error(`Window: unknown messsage method ${JSON.stringify(method)}`);
  }
});

const _findFreeSlot = a => {
  let i;
  for (i = 0; i < a.length; i++) {
    if (a[i] === null) {
      break;
    }
  }
  return i;
};

const _getFakeVrDisplay = window => {
  const {fakeVrDisplay} = window[symbols.mrDisplaysSymbol];
  return fakeVrDisplay.isActive ? fakeVrDisplay : null;
};
const _getVrDisplay = window => window[symbols.mrDisplaysSymbol].vrDisplay;
const _getMlDisplay = window => window[symbols.mrDisplaysSymbol].mlDisplay;
const _cloneMrDisplays = (mrDisplays, window) => {
  const result = {};
  for (const k in mrDisplays) {
    const mrDisplayClone = mrDisplays[k].clone();
    mrDisplayClone.onrequestanimationframe = _makeRequestAnimationFrame(window);
    mrDisplayClone.window = window;
    result[k] = mrDisplayClone;
  }
  return result;
};

function _makeWindow(window = {}, htmlString = '', options = {}) {
  const _normalizeUrl = utils._makeNormalizeUrl(options.baseUrl);

  const HTMLImageElementBound = (Old => class HTMLImageElement extends Old {
    constructor() {
      super(...arguments);

      // need to set owner document here because HTMLImageElement can be manually constructed via new Image()
      this.ownerDocument = window.document;
    }
  })(DOM.HTMLImageElement);

  const HTMLAudioElementBound = (Old => class HTMLAudioElement extends Old {
    constructor(src) {
      if (typeof src === 'string') {
        const audio = new HTMLAudioElementBound();
        audio.setAttribute('src', src);
        return audio;
      } else {
        super(...arguments);

        // need to set owner document here because HTMLAudioElement can be manually constructed via new Audio()
        this.ownerDocument = window.document;
      }
    }
  })(DOM.HTMLAudioElement);

  function createImageBitmap(src, x, y, w, h, options) {
    let image;
    if (src.constructor.name === 'HTMLImageElement') {
      image = src.image;
    } else if (src.constructor.name === 'Blob') {
      image = new nativeImage();
      try {
        image.load(src.buffer);
      } catch (err) {
        return Promise.reject(new Error('failed to load image'));
      }
    } else {
      return Promise.reject(new Error('invalid arguments. Unknown constructor type: ' + src.constructor.name));
    }

    if (typeof x === 'object') {
      options = x;
      x = undefined;
    }
    
    x = x || 0;
    y = y || 0;
    w = w || image.width;
    h = h || image.height;
    const flipY = !!options && options.imageOrientation === 'flipY';
    const imageBitmap = new nativeImageBitmap(
      image,
      x,
      y,
      w,
      h,
      flipY,
    );
    return Promise.resolve(imageBitmap);
  }

  function windowStartFn(global) {
    if (!global.args.require) {
      global.require = undefined;
    }
    const _logStack = err => {
      console.warn(err);
    };
    process.on('uncaughtException', _logStack);
    process.on('unhandledRejection', _logStack);
    global.process = undefined;
    global.setImmediate = undefined;
  }
  windowStartFn(global);

  for (const k in EventEmitter.prototype) {
    window[k] = EventEmitter.prototype[k];
  }
  EventEmitter.call(window);

  window.window = window;
  window.self = window;
  window.parent = window.top = {};

  window.innerWidth = defaultCanvasSize[0];
  window.innerHeight = defaultCanvasSize[1];
  window.devicePixelRatio = 1;
  const location = new Location(options.url);
  Object.defineProperty(window, 'location', {
    get() {
      return location;
    },
    set(href) {
      href = href + '';
      location.href = href;
    },
  });
  window.history = new History(location.href);
  window.navigator = {
    userAgent: `MixedReality (Exokit ${GlobalContext.version})`,
    vendor: 'MixedReality',
    platform: os.platform(),
    hardwareConcurrency: os.cpus().length,
    appCodeName: 'Mozilla',
    appName: 'Netscape',
    appVersion: '5.0',
    mediaDevices: {
      getUserMedia(constraints) {
        if (constraints.audio) {
          return Promise.resolve(new nativeAudio.MicrophoneMediaStream());
        } else if (constraints.video) {
          const dev = new VideoDevice();
          dev.constraints = constraints.video;
          return Promise.resolve(dev);
        } else {
          return Promise.reject(new Error('constraints not met'));
        }
      },
    },
    getVRDisplaysSync() {
      const result = [];
      const fakeVrDisplay = _getFakeVrDisplay(window);
      if (fakeVrDisplay) {
        result.push(fakeVrDisplay);
      }
      if (nativeMl && nativeMl.IsPresent()) {
        result.push(_getMlDisplay(window));
      }
      if (nativeVr && nativeVr.VR_IsHmdPresent()) {
        result.push(_getVrDisplay(window));
      }
      result.sort((a, b) => +b.isPresenting - +a.isPresenting);
      return result;
    },
    createVRDisplay() {
      const {fakeVrDisplay} = window[symbols.mrDisplaysSymbol];
      fakeVrDisplay.isActive = true;
      return fakeVrDisplay;
    },
    getGamepads,
  };

  // WebVR enabled.
  if (['all', 'webvr'].includes(global.args.xr)) {
    window.navigator.getVRDisplays = function() {
      return Promise.resolve(this.getVRDisplaysSync());
    }
  }

  // WebXR enabled.
  if (['all', 'webxr'].includes(global.args.xr)) {
    window.navigator.xr = new XR.XR(window);
  }

  window.destroy = function() {
    this._emit('destroy', {window: this});
  };
  window.URL = URL;
  window.fetch = (url, options) => {
    if (typeof url === 'string') {
      const blob = urls.get(url);
      if (blob) {
        return Promise.resolve(new Response(blob));
      } else {
        url = _normalizeUrl(url);
        return fetch(url, options);
      }
    } else {
      return fetch(url, options);
    }
  };
  window.Request = Request;
  window.Response = Response;
  window.Headers = Headers;
  window.Blob = Blob;
  window.FormData = FormData;
  window.XMLHttpRequest = (Old => {
    class XMLHttpRequest extends Old {
      open(method, url, async, username, password) {
        url = _normalizeUrl(url);
        return super.open(method, url, async, username, password);
      }
    }
    for (const k in XMLHttpRequestBase) {
      XMLHttpRequest[k] = XMLHttpRequestBase[k];
    }
    return XMLHttpRequest;
  })(XMLHttpRequest);
  window.WebSocket = (Old => {
    class WebSocket extends Old {
      constructor(url, protocols) {
        url = _normalizeUrl(url);
        super(url, protocols);
      }
    }
    for (const k in Old) {
      WebSocket[k] = Old[k];
    }
    return WebSocket;
  })(WebSocket);
  window.event = new Event(); // XXX this needs to track the current event
  window.localStorage = new LocalStorage(path.join(options.dataPath, '.localStorage'));
  window.indexedDB = indexedDB;
  window.performance = performance;
  window.screen = new Screen(window);
  window.urls = urls; // XXX non-standard
  window.scrollTo = function(x = 0, y = 0) {
    this.scrollX = x;
    this.scrollY = y;
  };
  window.scrollX = 0;
  window.scrollY = 0;

  window.btoa = btoa;
  window.atob = atob;

  window[symbols.htmlTagsSymbol] = {
    DOCUMENT: Document,
    BODY: DOM.HTMLBodyElement,
    A: DOM.HTMLAnchorElement,
    STYLE: DOM.HTMLStyleElement,
    SCRIPT: DOM.HTMLScriptElement,
    LINK: DOM.HTMLLinkElement,
    IMG: HTMLImageElementBound,
    AUDIO: HTMLAudioElementBound,
    VIDEO: DOM.HTMLVideoElement,
    SOURCE: DOM.HTMLSourceElement,
    IFRAME: DOM.HTMLIFrameElement,
    CANVAS: DOM.HTMLCanvasElement,
    TEMPLATE: DOM.HTMLTemplateElement,
  };
  window[symbols.optionsSymbol] = options;
  window.DocumentFragment = DocumentFragment;

  // DOM.
  window.Element = DOM.Element;
  window.HTMLElement = DOM.HTMLElement;
  window.HTMLAnchorElement = DOM.HTMLAnchorElement;
  window.HTMLStyleElement = DOM.HTMLStyleElement;
  window.HTMLLinkElement = DOM.HTMLLinkElement;
  window.HTMLScriptElement = DOM.HTMLScriptElement;
  window.HTMLImageElement = HTMLImageElementBound,
  window.HTMLAudioElement = HTMLAudioElementBound;
  window.HTMLVideoElement = DOM.HTMLVideoElement;
  window.SVGElement = DOM.SVGElement;
  window.HTMLIFrameElement = DOM.HTMLIFrameElement;
  window.HTMLCanvasElement = DOM.HTMLCanvasElement;
  window.HTMLTemplateElement = DOM.HTMLTemplateElement;
  window.Node = Node;
  window.Text = DOM.Text;
  window.Comment = DOM.Comment;
  window.NodeList = NodeList;
  window.HTMLCollection = DOM.HTMLCollection;

  window.customElements = new CustomElementRegistry(window);
  window.CustomElementRegistry = CustomElementRegistry;
  window.MutationObserver = require('./MutationObserver').MutationObserver;
  window.DOMRect = DOMRect;
  window[symbols.styleEpochSymbol] = 0;
  window.getComputedStyle = el => {
    let styleSpec = el[symbols.computedStyleSymbol];
    if (!styleSpec || styleSpec.epoch !== window[symbols.styleEpochSymbol]) {
      const style = el.style.clone();
      const stylesheetEls = el.ownerDocument.documentElement.getElementsByTagName('style')
        .concat(el.ownerDocument.documentElement.getElementsByTagName('link'));
      for (let i = 0; i < stylesheetEls.length; i++) {
        const {stylesheet} = stylesheetEls[i];
        if (stylesheet) {
          const {rules} = stylesheet;
          for (let j = 0; j < rules.length; j++) {
            const rule = rules[j];
            const {selectors} = rule;
            if (selectors && selectors.some(selector => el.matches(selector))) {
              const {declarations} = rule;
              for (let k = 0; k < declarations.length; k++) {
                const {property, value} = declarations[k];
                style[property] = value;
              }
            }
          }
        }
      }
      styleSpec = {
        style,
        epoch: window[symbols.styleEpochSymbol],
      };
      el[symbols.computedStyleSymbol] = styleSpec;
    }
    return styleSpec.style;
  };
  window.browser = {
    http,
    // https,
    ws,
    createRenderTarget: nativeWindow.createRenderTarget, // XXX needed for reality tabs fakeDisplay
    magicleap: nativeMl ? {
      RequestMeshing: nativeMl.RequestMeshing,
      RequestPlaneTracking: nativeMl.RequestPlaneTracking,
      RequestHandTracking: nativeMl.RequestHandTracking,
      RequestEyeTracking: nativeMl.RequestEyeTracking,
      RequestDepthPopulation: nativeMl.RequestDepthPopulation,
      RequestCamera: nativeMl.RequestCamera,
    } : null,
    monitors: new MonitorManager(),
  };
  window.DOMParser = class DOMParser {
    parseFromString(htmlString, type) {
      const _recurse = node => {
        let nodeName = null;
        let value = null;
        if (node.type === 'text') {
          nodeName = '#text';
          value = node.text;
        } else if (node.type === 'comment') {
          nodeName = '#comment';
          value = node.content;
        }

        const tagName = node.name || null;

        const attrs = [];
        if (node.attributes) {
          for (const name in node.attributes) {
            attrs.push({
              name,
              value: node.attributes[name],
            });
          }
        }

        const childNodes = node.children ? node.children.map(childNode => _recurse(childNode)) : [];

        return {
          nodeName,
          tagName,
          attrs,
          value,
          childNodes,
        };
      };
      const xmlAst = parseXml(htmlString, {
        // preserveComments: true,
      });
      const htmlAst = _recurse(xmlAst);
      return utils._parseDocumentAst(htmlAst, window, false);
    }
  };
  window.TextEncoder = TextEncoder;
  window.TextDecoder = TextDecoder;

  window.Event = Event;
  window.KeyboardEvent = KeyboardEvent;
  window.MouseEvent = MouseEvent;
  window.WheelEvent = WheelEvent;
  window.DragEvent = DragEvent;
  window.MessageEvent = MessageEvent;
  window.PromiseRejectionEvent = PromiseRejectionEvent;
  window.CustomEvent = CustomEvent;
  window.addEventListener = EventTarget.prototype.addEventListener.bind(window);
  window.removeEventListener = EventTarget.prototype.removeEventListener.bind(window);
  window.dispatchEvent = EventTarget.prototype.dispatchEvent.bind(window);

  window.requestAnimationFrame = fn => {
    fn = fn.bind(window);
    const id = ++rafIndex;
    fn[symbols.idSymbol] = id;
    rafCbs[_findFreeSlot(rafCbs)] = fn;
    return id;
  };
  window.cancelAnimationFrame = id => {
    const index = rafCbs.findIndex(r => r && r[symbols.idSymbol] === id);
    if (index !== -1) {
      rafCbs[index] = null;
    }
  };
  // window.postMessage = global.postMessage;

  window.Image = HTMLImageElementBound;
  window.ImageData = nativeImageData;
  window.ImageBitmap = nativeImageBitmap;
  window.createImageBitmap = createImageBitmap;

  window.Path2D = nativePath2D;
  window.CanvasGradient = nativeCanvasGradient;
  window.CanvasRenderingContext2D = CanvasRenderingContext2D;
  window.WebGLRenderingContext = WebGLRenderingContext;
  if (global.args.webgl !== '1') {
    window.WebGL2RenderingContext = WebGL2RenderingContext;
  }
  window.Audio = HTMLAudioElementBound;
  window.MediaRecorder = MediaRecorder;
  window.Document = Document;
  window.DocumentType = DocumentType;
  window.DOMImplementation = DOMImplementation;
  window.DataTransfer = DataTransfer;
  window.DataTransferItem = DataTransferItem;
  window.FileReader = FileReader;
  window.Screen = Screen;
  window.Gamepad = Gamepad;
  window.VRStageParameters = VRStageParameters;
  window.VRDisplay = VRDisplay;
  window.MLDisplay = MLDisplay;
  window.FakeVRDisplay = FakeVRDisplay;
  // window.ARDisplay = ARDisplay;
  window.VRFrameData = VRFrameData;
  if (window.navigator.xr) {
    window.XR = XR.XR;
    window.XRDevice = XR.XRDevice;
    window.XRSession = XR.XRSession;
    window.XRWebGLLayer = XR.XRWebGLLayer;
    window.XRPresentationFrame = XR.XRPresentationFrame;
    window.XRView = XR.XRView;
    window.XRViewport = XR.XRViewport;
    window.XRDevicePose = XR.XRDevicePose;
    window.XRInputSource = XR.XRInputSource;
    window.XRInputPose = XR.XRInputPose;
    window.XRInputSourceEvent = XR.XRInputSourceEvent;
    window.XRCoordinateSystem = XR.XRCoordinateSystem;
    window.XRFrameOfReference = XR.XRFrameOfReference;
    window.XRStageBounds = XR.XRStageBounds;
    window.XRStageBoundsPoint = XR.XRStageBoundsPoint;
  }

  window.AudioContext = AudioContext;
  window.AudioNode = nativeAudio.AudioNode;
  window.AudioBufferSourceNode = nativeAudio.AudioBufferSourceNode;
  window.OscillatorNode = nativeAudio.OscillatorNode;
  window.AudioDestinationNode = nativeAudio.AudioDestinationNode;
  window.AudioParam = nativeAudio.AudioParam;
  window.AudioListener = nativeAudio.AudioListener;
  window.GainNode = nativeAudio.GainNode;
  window.AnalyserNode = nativeAudio.AnalyserNode;
  window.PannerNode = nativeAudio.PannerNode;
  window.StereoPannerNode = nativeAudio.StereoPannerNode;

  window.Video = nativeVideo.Video;
  window.VideoDevice = nativeVideo.VideoDevice;

  window.Worker =  Worker;

  /*
    Treat function onload() as a special case that disables automatic event attach for onload, because this is how browsers work. E.g.
      <!doctype html><html><head><script>
        function onload() {
          console.log ('onload'); // NOT called; presence of top-level function onload() makes all the difference
        }
        window.onload = onload;
      </script></head></html>
  */
  window[symbols.disabledEventsSymbol] = {
    load: undefined,
    error: undefined,
  };
  window._emit = function(type) {
    if (!this[symbols.disabledEventsSymbol][type]) {
      Node.prototype._emit.apply(this, arguments);
    }
  };
  Object.defineProperty(window, 'onload', {
    get() {
      return window[symbols.disabledEventsSymbol]['load'] !== undefined ? window[symbols.disabledEventsSymbol]['load'] : _elementGetter(window, 'load');
    },
    set(onload) {
      if (nativeVm.isCompiling()) {
        this[symbols.disabledEventsSymbol]['load'] = onload;
      } else {
        if (window[symbols.disabledEventsSymbol]['load'] !== undefined) {
          this[symbols.disabledEventsSymbol]['load'] = onload;
        } else {
          _elementSetter(window, 'load', onload);
        }
      }
    },
  });
  Object.defineProperty(window, 'onerror', {
    get() {
      return window[symbols.disabledEventsSymbol]['error'] !== undefined ? window[symbols.disabledEventsSymbol]['error'] : _elementGetter(window, 'error');
    },
    set(onerror) {
      if (nativeVm.isCompiling()) {
        window[symbols.disabledEventsSymbol]['error'] = onerror;
      } else {
        if (window[symbols.disabledEventsSymbol]['error'] !== undefined) {
          window[symbols.disabledEventsSymbol]['error'] = onerror;
        } else {
          _elementSetter(window, 'error', onerror);
        }
      }
    },
  });
  Object.defineProperty(window, 'onmessage', {
    get() {
      return _elementGetter(window, 'message');
    },
    set(onmessage) {
      _elementSetter(window, 'message', onmessage);
    },
  });
  Object.defineProperty(window, 'onpopstate', {
    get() {
      return _elementGetter(window, 'popstate');
    },
    set(onpopstate) {
      _elementSetter(window, 'popstate', onpopstate);
    },
  });

  window.history.on('popstate', (u, state) => {
    window.location.set(u);

    const event = new Event('popstate');
    event.state = state;
    window.dispatchEvent(event);
  });
  let loading = false;
  window.location.on('update', href => {
    if (!loading) {
      exokit.load(href, {
        dataPath: options.dataPath,
      })
        .then(newWindow => {
          window._emit('beforeunload');
          window._emit('unload');
          window._emit('navigate', newWindow);
        })
        .catch(err => {
          loading = false;

          const e = new ErrorEvent('error', {target: this});
          e.message = err.message;
          e.stack = err.stack;
          this.dispatchEvent(e);
        });
      loading = true;
    }
  });

  if (!parent) {
    const _bindMRDisplay = display => {
      display.onrequestanimationframe = window.requestAnimationFrame;
      display.oncancelanimationframe = window.cancelAnimationFrame;
      display.onvrdisplaypresentchange = () => {
        process.nextTick(() => {
          const e = new Event('vrdisplaypresentchange');
          e.display = display;
          window.dispatchEvent(e);
        });
      };
    };

    const fakeVrDisplay = new FakeVRDisplay();
    fakeVrDisplay.isActive = false;
    fakeVrDisplay.onlayers = layers => {
      GlobalContext.fakePresentState.layers = layers;
    };

    const vrDisplay = new VRDisplay();
    _bindMRDisplay(vrDisplay);
    vrDisplay.onrequestpresent = layers => nativeVr.requestPresent(layers);
    vrDisplay.onexitpresent = () => nativeVr.exitPresent();
    vrDisplay.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const xrDisplay = new XR.XRDevice('VR');
    xrDisplay.onrequestpresent = layers => nativeVr.requestPresent(layers);
    xrDisplay.onexitpresent = () => nativeVr.exitPresent();
    xrDisplay.onrequestanimationframe = window.requestAnimationFrame;
    xrDisplay.oncancelanimationframe = window.cancelAnimationFrame;
    xrDisplay.requestSession = (requestSession => function() {
      return requestSession.apply(this, arguments)
        .then(session => {
          vrDisplay.isPresenting = true;
          session.once('end', () => {
            vrDisplay.isPresenting = false;
          });
          return session;
        });
    })(xrDisplay.requestSession);
    xrDisplay.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const mlDisplay = new MLDisplay();
    _bindMRDisplay(mlDisplay);
    mlDisplay.onrequestpresent = layers => nativeMl.requestPresent(layers);
    mlDisplay.onexitpresent = () => nativeMl.exitPresent();
    mlDisplay.onlayers = layers => {
      GlobalContext.mlPresentState.layers = layers;
    };

    const xmDisplay = new XR.XRDevice('AR');
    xmDisplay.onrequestpresent = layers => nativeMl.requestPresent(layers);
    xmDisplay.onexitpresent = () => nativeMl.exitPresent();
    xmDisplay.onrequestanimationframe = window.requestAnimationFrame;
    xmDisplay.oncancelanimationframe = window.cancelAnimationFrame;
    xmDisplay.requestSession = (requestSession => function() {
      return requestSession.apply(this, arguments)
        .then(session => {
          mlDisplay.isPresenting = true;
          session.once('end', () => {
            mlDisplay.isPresenting = false;
          });
          return session;
        });
    })(xmDisplay.requestSession);
    xmDisplay.onlayers = layers => {
      GlobalContext.mlPresentState.layers = layers;
    };

    window[symbols.mrDisplaysSymbol] = {
      fakeVrDisplay,
      vrDisplay,
      xrDisplay,
      mlDisplay,
      xmDisplay,
    };

    if (nativeMl) {
      let lastPresent = nativeMl.IsPresent();

      nativeMl.OnPresentChange(isPresent => {
        if (isPresent && !lastPresent) {
          const e = new Event('vrdisplayconnect');
          e.display = _getMlDisplay(window);
          window.dispatchEvent(e);
        } else if (!isPresent && lastPresent) {
          const e = new Event('vrdisplaydisconnect');
          e.display = _getMlDisplay(window);
          window.dispatchEvent(e);
        }
        lastPresent = isPresent;
      });
    }
  } else {
    window[symbols.mrDisplaysSymbol] = _cloneMrDisplays(top[symbols.mrDisplaysSymbol], window);

    top.on('vrdisplaypresentchange', e => {
      window._emit('vrdisplaypresentchange', e);
    });
  }

  window.document = utils._parseDocument(htmlString, window);

  return window;
}
_makeWindowVm(global, global.args.htmlString, global.args.options);

})();
