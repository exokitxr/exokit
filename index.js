#!/usr/bin/env node

process.chdir(__dirname); // needed for global bin to find libraries

const path = require('path');
const fs = require('fs');
const os = require('os');
const repl = require('repl');
const core = require('./core.js');
const mkdirp = require('mkdirp');
const replHistory = require('repl.history');
const minimist = require('minimist');
const UPNG = require('upng-js');
const {version} = require('./package.json');
const emojis = require('./assets/emojis');
const nativeBindingsModulePath = path.join(__dirname, 'native-bindings.js');
const {THREE} = core;
const nativeBindings = require(nativeBindingsModulePath);
const {nativeVideo, nativeVr, nativeLm, nativeMl, nativeWindow} = nativeBindings;

const dataPath = path.join(os.homedir() || __dirname, '.exokit');

const canvasSymbol = Symbol();
const contexts = [];
const _windowHandleEquals = (a, b) => a[0] === b[0] && a[1] === b[1];

const args = (() => {
  if (require.main === module) {
    const minimistArgs = minimist(process.argv.slice(2), {
      boolean: [
        'home',
        'perf',
        'performance',
        'frame',
        'minimalFrame',
        'blit',
	'version',
      ],
      string: [
        'size',
        'image',
        'depth-image',
      ],
      alias: {
        h: 'home',
        p: 'performance',
        perf: 'performance',
        s: 'size',
        f: 'frame',
        m: 'minimalFrame',
        b: 'blit',
        i: 'image',
        d: 'depth-image',
        v: 'version',
      },
    });
    return {
      url: minimistArgs._[0] || '',
      home: minimistArgs.home,
      performance: !!minimistArgs.performance,
      size: minimistArgs.size,
      frame: minimistArgs.frame,
      minimalFrame: minimistArgs.minimalFrame,
      blit: minimistArgs.blit,
      image: minimistArgs.image,
      depthImage: minimistArgs['depth-image'],
      version: minimistArgs.version,
    };
  } else {
    return {};
  }
})();

core.setArgs(args);
const agent = core.setVersion(version);
core.setNativeBindingsModule(nativeBindingsModulePath);

if (args.version) {
  console.log(agent);
  process.exit(0);
}

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
    }
  })();
  if (windowSpec) {
    const [windowHandle, sharedFramebuffer, sharedColorTexture, sharedDepthStencilTexture] = windowSpec;

    gl.setWindowHandle(windowHandle);

    gl[canvasSymbol] = canvas;

    const document = canvas.ownerDocument;
    const window = document.defaultView;
    const framebufferWidth = nativeWindow.getFramebufferSize(windowHandle).width;
    window.devicePixelRatio = framebufferWidth / canvasWidth;

    const title = `${agent.name} ${agent.version}`
    nativeWindow.setWindowTitle(windowHandle, title);

    if (document.hidden) {
      const [framebuffer, colorTexture, depthStencilTexture] = nativeWindow.createRenderTarget(gl, canvasWidth, canvasHeight, 1, sharedColorTexture, sharedDepthStencilTexture);
      gl.setDefaultFramebuffer(framebuffer);

      document._emit('framebuffer', {
        framebuffer: sharedFramebuffer,
        colorTexture,
        depthStencilTexture,
      });
    }

    const ondomchange = () => {
      process.nextTick(() => { // show/hide synchronously emits events
        const {hidden} = canvas.ownerDocument;
        if (!hidden) {
          const visible = canvas.ownerDocument.documentElement.contains(canvas);
          if (visible && !hidden) {
            nativeWindow.show(windowHandle);
          } else {
            nativeWindow.hide(windowHandle);
          }
        }
      });
    };
    canvas.ownerDocument.on('domchange', ondomchange);

    gl.destroy = (destroy => function() {
      nativeWindow.destroy(windowHandle);
      canvas._context = null;
      canvas.ownerDocument.removeListener('domchange', ondomchange);

      contexts.splice(contexts.indexOf(gl), 1);
    })(gl.destroy);

    contexts.push(gl);

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
const localFovArray = new Float32Array(4);
const localFovArray2 = new Float32Array(4);
const localGamepadArray = new Float32Array(16);

const handEntrySize = (1 + (5 * 5)) * (3 + 3);
const maxNumPlanes = 32 * 3;
const planeEntrySize = 3 + 4 + 2 + 1;
const framebufferArray = new Uint32Array(2);
const transformArray = new Float32Array(7 * 2);
const projectionArray = new Float32Array(16 * 2);
const viewportArray = new Uint32Array(4);
const handsArray = [
  new Float32Array(handEntrySize),
  new Float32Array(handEntrySize),
];
const planesArray = new Float32Array(planeEntrySize * maxNumPlanes);
const numPlanesArray = new Uint32Array(1);
const controllersArray = new Float32Array((3 + 4 + 1) * 2);
const gesturesArray = new Float32Array(4 * 2);
const meshArray = [null, null, null];

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
  fbo: null,
  tex: null,
  lmContext: null,
};
let renderWidth = 0;
let renderHeight = 0;
const depthNear = 0.1;
const depthFar = 10000.0;
const _requestContext = () => {
  if (!vrPresentState.vrContext) {
    vrPresentState.vrContext = nativeVr.getContext();
  }
  return Promise.resolve(vrPresentState.vrContext);
};
const _requestSystem = vrContext => new Promise((accept, reject) => {
  let err = null;
  const _recurse = (i = 0) => { // while booting we sometimes get transient errors
    if (i < 20) {
      const system = (() => {
        try {
          return nativeVr.VR_Init(nativeVr.EVRApplicationType.Scene);
        } catch (newErr) {
          err = newErr;
          return null;
        }
      })();
      if (system) {
        accept(system);
      } else {
        setTimeout(() => {
          _recurse(i + 1);
        }, 100);
      }
    } else {
      reject(err);
    };
  };
  _recurse();
});
nativeVr.requestPresent = function(layers) {
  if (!vrPresentState.isPresenting) {
    const layer = layers.find(layer => layer && layer.source && layer.source.constructor && layer.source.constructor.name === 'HTMLCanvasElement' && layer.source._context && layer.source._context.constructor && layer.source._context.constructor.name === 'WebGLRenderingContext');
    if (layer) {
      const canvas = layer.source;
      const context = canvas._context;
      const window = canvas.ownerDocument.defaultView;

      return _requestContext()
        .then(vrContext =>
          _requestSystem()
            .then(newSystem => {
              const {width: halfWidth, height} = newSystem.GetRecommendedRenderTargetSize();
              renderWidth = halfWidth;
              renderHeight = height;

              window.top.updateVrFrame({
                renderWidth,
                renderHeight,
              });

              nativeWindow.setCurrentWindowContext(context.getWindowHandle());

              const width = halfWidth * 2;
              const [msFbo, msTex, msDepthStencilTex] = nativeWindow.createRenderTarget(context, width, height, 4, 0, 0);
              const [fbo, tex, depthStencilTex] = nativeWindow.createRenderTarget(context, width, height, 1, 0, 0);

              context.setDefaultFramebuffer(msFbo);

              vrPresentState.isPresenting = true;
              vrPresentState.system = newSystem;
              vrPresentState.compositor = vrContext.compositor.NewCompositor();
              vrPresentState.glContext = context;
              vrPresentState.msFbo = msFbo;
              vrPresentState.msTex = msTex;
              vrPresentState.fbo = fbo;
              vrPresentState.tex = tex;

              vrPresentState.lmContext = nativeLm && new nativeLm();
            })
        );
    } else {
      return Promise.reject(new Error('no HTMLCanvasElement source with WebGLRenderingContext provided'));
    }
  } else {
    return Promise.reject(new Error('already presenting'));
  }
};
nativeVr.exitPresent = function() {
  if (vrPresentState.isPresenting) {
    nativeVr.VR_Shutdown();

    nativeWindow.destroyRenderTarget(vrPresentState.msFbo, vrPresentState.msTex);
    nativeWindow.destroyRenderTarget(vrPresentState.fbo, vrPresentState.tex);

    const context = vrPresentState.glContext;
    nativeWindow.setCurrentWindowContext(context.getWindowHandle());
    context.setDefaultFramebuffer(0);

    vrPresentState.isPresenting = false;
    vrPresentState.system = null;
    vrPresentState.compositor = null;
    vrPresentState.glContext = null;
    vrPresentState.msFbo = null;
    vrPresentState.msTex = null;
    vrPresentState.fbo = null;
    vrPresentState.tex = null;
  }

  return Promise.resolve();
};
let mlContext = null;
let isMlPresenting = false;
let mlFbo = null;
let mlTex = null;
let mlGlContext = null;
if (nativeMl) {
  mlContext = new nativeMl();
  nativeMl.requestPresent = function(layers) {
    if (!isMlPresenting) {
      const layer = layers.find(layer => layer && layer.source && layer.source.constructor && layer.source.constructor.name === 'HTMLCanvasElement' && layer.source._context && layer.source._context.constructor && layer.source._context.constructor.name === 'WebGLRenderingContext');
      if (layer) {
        const canvas = layer.source;
        const context = canvas._context;
        const window = canvas.ownerDocument.defaultView;

        const windowHandle = context.getWindowHandle();
        nativeWindow.setCurrentWindowContext(windowHandle);

        const initResult = mlContext.Init(windowHandle);
        if (initResult) {
          isMlPresenting = true;

          const [fbo, tex, depthStencilTex] = nativeWindow.createRenderTarget(context, window.innerWidth, window.innerHeight, 1, 0, 0);
          mlFbo = fbo;
          mlTex = tex;

          mlContext.WaitGetPoses(framebufferArray, transformArray, projectionArray, viewportArray, planesArray, numPlanesArray, controllersArray, gesturesArray, meshArray);

          /* for (let i = 0; i < 2; i++) {
            nativeWindow.framebufferTextureLayer(framebufferArray[0], framebufferArray[1], i);
          } */

          window.top.updateMlFrame({
            transformArray,
            projectionArray,
            viewportArray,
            planesArray,
            numPlanes: numPlanesArray[0],
            gamepads: [null, null],
            meshArray,
          });
          mlContext.SubmitFrame(mlFbo, window.innerWidth, window.innerHeight);

          context.setDefaultFramebuffer(mlFbo);

          mlGlContext = context;

          return Promise.resolve();
        } else {
          return Promise.reject(new Error('simulator not attached'));
        }
      } else {
        return Promise.reject(new Error('no HTMLCanvasElement source with WebGLRenderingContext provided'));
      }
    } else {
      return Promise.reject(new Error('already presenting'));
    }
  };
  nativeMl.exitPresent = function() {
    if (isMlPresenting) {
      throw new Error('not implemented'); // XXX
    }

    return Promise.resolve();
  };
}

nativeWindow.setEventHandler((type, data) => {
  // console.log(type, data);

  const {windowHandle} = data;
  const context = contexts.find(context => _windowHandleEquals(context.getWindowHandle(), windowHandle));
  const canvas = context[canvasSymbol];
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

        window.innerWidth = innerWidth;
        window.innerHeight = innerHeight;
        canvas.dispatchEvent(new window.Event('resize'));
        break;
      }
      case 'keydown': {
        if (data.keyCode === 27 && window.top.document.pointerLockElement) {
          window.top.document.exitPointerLock();
        } else {
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
        const e = new window.MouseEvent(type, data);

        // XXX The overlay detection here is a hack.
        // It's only needed because sibling overlay elements sometimes expect to capture events instead of the canvas.
        // The correct way to handle this is to compute actual layout with something like reworkcss + Yoga.
        let dispatchEl = null;
        if (!window.document.pointerLockElement && !vrPresentState.isPresenting && (dispatchEl = window.document.documentElement.traverse(el => {
          if (el.nodeType === window.Node.ELEMENT_NODE && window.getComputedStyle(el).cursor === 'pointer') {
            return el;
          }
        }))) {
          dispatchEl.dispatchEvent(e);
        }

        canvas.dispatchEvent(e);
        break;
      }
      case 'wheel': {
        canvas.dispatchEvent(new window.WheelEvent(type, data));
        break;
      }
      case 'mousemove': {
        if (window.document.pointerLockElement) {
          data.movementX = data.pageX - (window.innerWidth / 2);
          data.movementY = data.pageY - (window.innerHeight / 2);

          nativeWindow.setCursorPosition(context.getWindowHandle(), window.innerWidth / 2, window.innerHeight / 2);
        }

        canvas.dispatchEvent(new window.MouseEvent(type, data));
        break;
      }
      case 'drop': {
        Promise.all(data.paths.map(p => new Promise((accept, reject) => {
          fs.readFile(p, (err, data) => {
            if (!err) {
              accept(new window.Blob([data]));
            } else {
              reject(err);
            }
          });
        })))
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
        contexts.splice(contexts.indexOf(context), 1);
        break;
      }
    }
  } else {
    console.warn('got native window event with no matching context', {type, data});
  }
});

// EXPORTS

module.exports = core;

// MAIN

let innerWidth = 1280; // XXX do not track this globally
let innerHeight = 1024;
const FPS = 90;
const FRAME_TIME_MAX = ~~(1000 / FPS);
const FRAME_TIME_MIN = 0;
if (require.main === module) {
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

  const _prepare = () => {
    if (!process.env['DISPLAY']) {
      process.env['DISPLAY'] = ':0.0';
    }

    return Promise.all([
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
  };
  const _start = () => {
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
            nativeWindow.setCursorMode(contexts[i].getWindowHandle(), false);
          }
        } else {
          for (let i = 0; i < contexts.length; i++) {
            nativeWindow.setCursorMode(contexts[i].getWindowHandle(), true);
          }
        }
      });
      window.addEventListener('destroy', e => {
        const {window} = e;
        for (let i = 0; i < contexts.length; i++) {
          const context = contexts[i];
          if (context[canvasSymbol].ownerDocument.defaultView === window) {
            context.destroy();
          }
        }
      });
      window.addEventListener('error', err => {
        console.warn('got error', err);
      });

      let lastFrameTime = Date.now();
      const timestamps = {
        frames: 0,
        last: Date.now(),
        wait: 0,
        pose: 0,
        prepare: 0,
        events: 0,
        media: 0,
        user: 0,
        submit: 0,
        total: 0,
      };
      const TIMESTAMP_FRAMES = 90;
      const leftGamepad = new window.Gamepad('left', 0);
      const rightGamepad = new window.Gamepad('right', 1);
      const gamepads = [null, null];
      const frameData = new window.VRFrameData();
      const stageParameters = new window.VRStageParameters();
      let timeout = null;
      let numFrames = 0;
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

      window.document.addEventListener('pointerlockchange', () => {
        const {pointerLockElement} = window.document;
        if (pointerLockElement) {
          for (let i = 0; i < contexts.length; i++) {
            nativeWindow.setCursorMode(contexts[i].getWindowHandle(), false);
          }
        } else {
          for (let i = 0; i < contexts.length; i++) {
            nativeWindow.setCursorMode(contexts[i].getWindowHandle(), true);
          }
        }
      });

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

      window.addEventListener('error', err => {
        console.warn('got error', err);
      });

      const _recurse = () => {
        if (args.performance) {
          if (timestamps.frames >= TIMESTAMP_FRAMES) {
            console.log(`${(TIMESTAMP_FRAMES/(timestamps.total/1000)).toFixed(0)} FPS | ${timestamps.wait}ms wait | ${timestamps.pose}ms pose | ${timestamps.prepare}ms prepare | ${timestamps.events}ms events | ${timestamps.media}ms media | ${timestamps.user}ms user | ${timestamps.submit}ms submit`);

            timestamps.frames = 0;
            timestamps.wait = 0;
            timestamps.pose = 0;
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
          timestamps.wait += diff;
          timestamps.total += diff;
          timestamps.last = now;
        }

        if (vrPresentState.isPresenting) {
          // wait for frame
          vrPresentState.compositor.WaitGetPoses(
            vrPresentState.system,
            localFloat32Array, // hmd
            localFloat32Array2, // left controller
            localFloat32Array3 // right controller
          );
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
          localMatrix2.fromArray(localFloat32Array4)
            .getInverse(localMatrix2)
            .multiply(hmdMatrix);
          localMatrix2.toArray(frameData.leftViewMatrix);
          localMatrix2.decompose(localVector, localQuaternion, localVector2);
          const leftOffset = localVector.length();

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
          localMatrix2.fromArray(localFloat32Array4)
            .getInverse(localMatrix2)
            .multiply(hmdMatrix);
          localMatrix2.toArray(frameData.rightViewMatrix);
          localMatrix2.decompose(localVector, localQuaternion, localVector2);
          const rightOffset = localVector.length();

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

            leftGamepad.buttons[0].touched = localGamepadArray[9] !== 0; // pad
            leftGamepad.buttons[1].touched = localGamepadArray[10] !== 0; // trigger
            leftGamepad.buttons[2].touched = localGamepadArray[8] !== 0; // grip
            leftGamepad.buttons[3].touched = localGamepadArray[7] !== 0; // menu

            leftGamepad.axes[0] = localGamepadArray[11];
            leftGamepad.axes[1] = localGamepadArray[12];

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

            rightGamepad.buttons[0].touched = localGamepadArray[9] !== 0; // pad
            rightGamepad.buttons[1].touched = localGamepadArray[10] !== 0; // trigger
            rightGamepad.buttons[2].touched = localGamepadArray[8] !== 0; // grip
            rightGamepad.buttons[3].touched = localGamepadArray[7] !== 0; // menu

            rightGamepad.axes[0] = localGamepadArray[11];
            rightGamepad.axes[1] = localGamepadArray[12];

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
        } else if (isMlPresenting) {
          mlContext.WaitGetPoses(framebufferArray, transformArray, projectionArray, viewportArray, planesArray, numPlanesArray, controllersArray, gesturesArray, meshArray);
          if (args.performance) {
            const now = Date.now();
            const diff = now - timestamps.last;
            timestamps.pose += diff;
            timestamps.total += diff;
            timestamps.last = now;
          }

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
          controllersArrayIndex++;

          let gesturesArrayIndex = 0;
          leftGamepad.gesture.position.set(gesturesArray.slice(gesturesArrayIndex, gesturesArrayIndex + 3));
          gesturesArrayIndex += 3;
          leftGamepad.gesture.gesture = gesturesArray[gesturesArrayIndex];
          gesturesArrayIndex++;

          gamepads[0] = leftGamepad;

          rightGamepad.pose.position.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 3));
          controllersArrayIndex += 3;
          rightGamepad.pose.orientation.set(controllersArray.slice(controllersArrayIndex, controllersArrayIndex + 4));
          controllersArrayIndex += 4;
          const rightTriggerValue = controllersArray[controllersArrayIndex];
          rightGamepad.buttons[1].value = leftTriggerValue;
          const rightTriggerPushed = rightTriggerValue > 0.5;
          rightGamepad.buttons[1].touched = rightTriggerPushed;
          rightGamepad.buttons[1].pressed = rightTriggerPushed;
          controllersArrayIndex++;

          rightGamepad.gesture.position.set(gesturesArray.slice(gesturesArrayIndex, gesturesArrayIndex + 3));
          gesturesArrayIndex += 3;
          rightGamepad.gesture.gesture = gesturesArray[gesturesArrayIndex];
          gesturesArrayIndex++;

          gamepads[1] = rightGamepad;

          window.top.updateMlFrame({
            transformArray,
            projectionArray,
            viewportArray,
            planesArray,
            numPlanes: numPlanesArray[0],
            gamepads,
            meshArray,
          });

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
            timestamps.pose += diff;
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
        if ((numFrames % FPS) === 0) {
          const displays = window.navigator.getVRDisplaysSync();
          if (!displays.some(display => display.isPresenting)) {
            for (let i = 0; i < displays.length; i++) {
              const e = new window.Event('vrdisplayactivate');
              e.display = displays[i];
              window.dispatchEvent(e);
            }
          }
        }
        window.tickAnimationFrame();
        if (args.frame || args.minimalFrame) {
          console.log('-'.repeat(80) + 'end frame');
        }
        numFrames++;
        if (args.performance) {
          const now = Date.now();
          const diff = now - timestamps.last;
          timestamps.user += diff;
          timestamps.total += diff;
          timestamps.last = now;
        }

        // submit frame
        for (let i = 0; i < contexts.length; i++) {
          const context = contexts[i];
          if (context.isDirty()) {
            if (vrPresentState.glContext === context) {
              nativeWindow.setCurrentWindowContext(context.getWindowHandle());

              nativeWindow.blitFrameBuffer(context, vrPresentState.msFbo, vrPresentState.fbo, renderWidth * 2, renderHeight, renderWidth * 2, renderHeight, true, false, false);
              vrPresentState.compositor.Submit(context, vrPresentState.tex);

              nativeWindow.blitFrameBuffer(context, vrPresentState.fbo, 0, renderWidth * (args.blit ? 1 : 2), renderHeight, window.innerWidth, window.innerHeight, true, false, false);
            } else if (mlGlContext === context) {
              nativeWindow.setCurrentWindowContext(context.getWindowHandle());

              mlContext.SubmitFrame(mlFbo, window.innerWidth, window.innerHeight);

              nativeWindow.blitFrameBuffer(context, mlFbo, 0, window.innerWidth, window.innerHeight, window.innerWidth, window.innerHeight, true, false, false);
            }
            nativeWindow.swapBuffers(context.getWindowHandle());

            numDirtyFrames++;
            _checkDirtyFrameTimeout();

            context.clearDirty();
          }
        }
        if (args.performance) {
          const now = Date.now();
          const diff = now - timestamps.last;
          timestamps.submit += diff;
          timestamps.total += diff;
          timestamps.last = now;
        }

        // wait for next frame
        const now = Date.now();
        timeout = setTimeout(_recurse, Math.min(Math.max(FRAME_TIME_MAX - ~~(now - lastFrameTime), FRAME_TIME_MIN), FRAME_TIME_MAX));
        lastFrameTime = now;
      };
      _recurse();
    };

    let {url} = args;
    if (!url && args.home) {
      url = 'file://' + path.join(path.dirname(require.resolve('exokit-home')), 'index.html');
    }
    if (url) {
      if (url === '.') {
        console.warn('NOTE: You ran `exokit . <url>`\n(Did you mean to run `node . <url>` or `exokit <url>` instead?)')
      }
      if (url.indexOf('://') === -1) {
        url = 'http://' + url;
      }
      return core.load(url, {
        dataPath,
      })
        .then(window => {
          const _bindHeadlessWindow = newWindow => {
            _bindWindow(newWindow, _bindHeadlessWindow);
          };
          _bindHeadlessWindow(window);

          const _flipImage = (width, height, stride, arrayBuffer) => {
            const uint8Array = new Uint8Array(arrayBuffer);

            const arrayBuffer2 = new ArrayBuffer(arrayBuffer.byteLength);
            const uint8Array2 = new Uint8Array(arrayBuffer2);
            for (let y = 0; y < height; y++) {
              const yBottom = height - y - 1;
              uint8Array2.set(uint8Array.slice(yBottom * width * stride, (yBottom + 1) * width * stride), y * width * stride);
            }
            return arrayBuffer2;
          };

          if (args.image) {
            window.setDirtyFrameTimeout({
              dirtyFrames: 100,
              timeout: 5000,
            }, (err, gl) => {
              if (!err) {
                const {[canvasSymbol]: canvas} = gl;
                const {width, height} = canvas;

                const arrayBuffer = new ArrayBuffer(width * height * 4);
                const uint8Array = new Uint8Array(arrayBuffer);
                gl.readPixels(0, 0, width, height, gl.RGBA, gl.UNSIGNED_BYTE, uint8Array);
                const result = new Buffer(UPNG.encode([
                  _flipImage(width, height, 4, arrayBuffer),
                ], width, height, 0));
                fs.writeFileSync(args.image, result);

                process.exit(0);
              } else {
                throw err;
              }
            });
          } else if (args.depthImage) {
            window.setDirtyFrameTimeout({
              dirtyFrames: 100,
              timeout: 5000,
            }, (err, gl) => {
              if (!err) {
                const {[canvasSymbol]: canvas} = gl;
                const {width, height} = canvas;

                const arrayBuffer = new ArrayBuffer(width * height * 4);
                const uint8Array = new Uint8Array(arrayBuffer);
                gl.readPixels(0, 0, width, height, gl.RGBA, gl.UNSIGNED_BYTE, uint8Array);
                const diffuseImage = new Buffer(UPNG.encode([
                  _flipImage(width, height, 4, arrayBuffer),
                ], width, height, 0));
                fs.writeFileSync(args.depthImage + '.png', diffuseImage);

                const float32Array = new Float32Array(arrayBuffer);
                gl.readPixels(0, 0, width, height, gl.DEPTH_COMPONENT, gl.FLOAT, float32Array);
                const depthImage = new Buffer(_flipImage(width, height, 4, arrayBuffer));
                fs.writeFileSync(args.depthImage + '.depth', depthImage);

                process.exit(0);
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

      const _getPrompt = os.platform() !== 'win32' ?
        () => `[${emojis[Math.floor(Math.random() * emojis.length)]}] `
      :
        () => '[x] ';

      let lastUnderscore = window._;
      const replEval = (cmd, context, filename, callback) => {
        cmd = cmd.slice(0, -1); // remove trailing \n

        let result, err = null, match;

        if (/^\s*<(?:\!\-*)?[a-z]/i.test(cmd)) {
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
            r.setPrompt(_getPrompt());
          }
        } else {
          if (err.name === 'SyntaxError') {
            err = new repl.Recoverable(err);
          }
        }
        callback(err, result);
      };
      const r = repl.start({
        prompt: _getPrompt(),
        eval: replEval,
      });
      r.defineCommand('go', {
        help: 'Navigate to <url>',
        action(url) {
          window.location.href = url;
          this.clearBufferedCommand();
          this.displayPrompt();
        }
      });
      replHistory(r, path.join(dataPath, '.repl_history'));
      r.on('exit', () => {
        process.exit();
      });
    }
  };

  _prepare()
    .then(() => _start());
}

process.on('uncaughtException', err => {
  console.warn(err.stack);
});
process.on('unhandledRejection', err => {
  console.warn(err.stack);
});
