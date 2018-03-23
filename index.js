#!/usr/bin/env node

process.chdir(__dirname); // needed for global bin to find libraries

const path = require('path');
const fs = require('fs');
const os = require('os');
const vm = require('vm');
const repl = require('repl');
const mkdirp = require('mkdirp');
const replHistory = require('repl.history');
const exokit = require('exokit-core');
const emojis = require('./assets/emojis');
const nativeBindingsModulePath = path.join(__dirname, 'native-bindings.js');
exokit.setNativeBindingsModule(nativeBindingsModulePath);
const {THREE} = exokit;
const nativeBindings = require(nativeBindingsModulePath);
const {nativeVideo, nativeVr, nativeWindow} = nativeBindings;

/* const {VERSION} = nativeGl;

nativeGl = {};
nativeGl.VERSION = VERSION; */
/* nativeGl.enable = () => {};
nativeGl.disable = () => {};
nativeGl.clear = () => {};
nativeGl.getExtension = () => null;
nativeGl.getParameter = id => {
  if (id === VERSION) {
    return 'WebGL 1';
  } else {
    return {};
  }
};
nativeGl.createTexture = () => {};
nativeGl.bindTexture = () => {};
nativeGl.texParameteri = () => {};
const _texImage2D = nativeGl.prototype.texImage2D;
nativeGl.prototype.texImage2D = function(a, b, c, d, e, f) {
  if (f.stack) {
    console.log('got teximage2d', f && f.constructor && f.constructor.name, f && f.stack);
  }
  try {
    return _texImage2D.apply(this, arguments);
  } catch(err) {
    console.log('failed teximage2d', f && f.constructor && f.constructor.name);

    throw err;
  }
};
nativeGl.clearColor = () => {};
nativeGl.clearDepth = () => {};
nativeGl.clearStencil = () => {};
nativeGl.depthFunc = () => {};
nativeGl.frontFace = () => {};
nativeGl.cullFace = () => {};
nativeGl.blendEquationSeparate = () => {};
nativeGl.blendFuncSeparate = () => {};
nativeGl.blendEquation = () => {};
nativeGl.blendFunc = () => {};
const _viewport = nativeGl.viewport;
nativeGl.viewport = function() {
  console.log('gl viewport', arguments, new Error().stack);
  _viewport.apply(this, arguments);
}; */

// CALLBACKS

const canvasSymbol = Symbol();
const contexts = [];
const _windowHandleEquals = (a, b) => a[0] === b[0] && a[1] === b[1];
const _isAttached = el => {
  if (el === el.ownerDocument.documentElement) {
    return true;
  } else if (el.parentNode) {
    return _isAttached(el.parentNode);
  } else {
    return false;
  }
};
nativeBindings.nativeGl.onconstruct = (gl, canvas) => {
  gl[canvasSymbol] = canvas;

  const windowHandle = nativeWindow.create(canvas.width || innerWidth, canvas.height || innerHeight, _isAttached(canvas));
  gl.setWindowHandle(windowHandle);

  const onparent = () => {
    if (_isAttached(canvas)) {
      nativeWindow.show(windowHandle);
    } else {
      nativeWindow.hide(windowHandle);
    }
  };
  canvas.on('parent', onparent);

  gl.destroy = (destroy => function() {
    nativeWindow.destroy(windowHandle);
    canvas._context = null;
    canvas.removeListener('parent', onparent);
  })(gl.destroy);

  contexts.push(gl);
};

const zeroMatrix = new THREE.Matrix4();
const localFloat32Array = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array2 = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array3 = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array4 = new Float32Array(16);
const localGamepadArray = new Float32Array(16);
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
};
let renderWidth = 0;
let renderHeight = 0;
const depthNear = 0.1;
const depthFar = 1000.0;
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

            window.updateVrFrame({
              renderWidth,
              renderHeight,
            });

            nativeWindow.setCurrentWindowContext(context.getWindowHandle());

            const width = halfWidth * 2;
            const [msFbo, msTex] = nativeWindow.createRenderTarget(width, height, 4);
            const [fbo, tex] = nativeWindow.createRenderTarget(width, height, 1);

            nativeWindow.bindFrameBuffer(msFbo);

            vrPresentState.isPresenting = true;
            vrPresentState.system = newSystem;
            vrPresentState.compositor = vrContext.compositor.NewCompositor();
            vrPresentState.glContext = context;
            vrPresentState.msFbo = msFbo;
            vrPresentState.msTex = msTex;
            vrPresentState.fbo = fbo;
            vrPresentState.tex = tex;
          })
      );
  } else {
    return Promise.reject(new Error('no HTMLCanvasElement source with WebGLRenderingContext provided'));
  }
};
nativeVr.exitPresent = function() {
  if (vrPresentState.isPresenting) {
    nativeVr.VR_Shutdown();

    nativeWindow.destroyRenderTarget(vrPresentState.msFbo, vrPresentState.msTex);
    nativeWindow.destroyRenderTarget(vrPresentState.fbo, vrPresentState.tex);

    nativeWindow.setCurrentWindowContext(vrPresentState.glContext.getWindowHandle());
    nativeWindow.bindFrameBuffer(0);

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

const _dispatchCanvasEvent = (canvas, event) => {
  [canvas, canvas.ownerDocument.defaultView].every(target => {
    target.dispatchEvent(event);
    return !event.propagationStopped;
  });
};
nativeWindow.setEventHandler((type, data) => {
  // console.log(type, data);

  const {windowHandle} = data;
  const context = contexts.find(context => _windowHandleEquals(context.getWindowHandle(), windowHandle));
  const canvas = context[canvasSymbol];
  const window = canvas.ownerDocument.defaultView;

  if (context) {
    switch (type) {
      case 'framebufferResize': {
        const {width, height} = data;
        innerWidth = width;
        innerHeight = height;

        window.innerWidth = innerWidth;
        window.innerHeight = innerHeight;
        _dispatchCanvasEvent(canvas, new window.Event('resize'));
        break;
      }
      case 'keydown': {
        if (data.keyCode === 27 && window.top.document.pointerLockElement) {
          window.top.document.exitPointerLock();
        }

        _dispatchCanvasEvent(canvas, new window.KeyboardEvent(type, data));
        break;
      }
      case 'keyup':
      case 'keypress': {
        _dispatchCanvasEvent(canvas, new window.KeyboardEvent(type, data));
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

        _dispatchCanvasEvent(canvas, e);
        break;
      }
      case 'wheel': {
        _dispatchCanvasEvent(canvas, new window.WheelEvent(type, data));
        break;
      }
      case 'mousemove': {
        if (window.document.pointerLockElement) {
          data.movementX = data.pageX - (window.innerWidth / window.devicePixelRatio / 2);
          data.movementY = data.pageY - (window.innerHeight / window.devicePixelRatio / 2);

          nativeWindow.setCursorPosition(context.getWindowHandle(), window.innerWidth / 2, window.innerHeight / 2);
        }

        _dispatchCanvasEvent(canvas, new window.MouseEvent(type, data));
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

module.exports = exokit;

// MAIN

let window = null;
let innerWidth = 1280; // XXX do not track this globally
let innerHeight = 1024;
const FPS = 90;
const FRAME_TIME_MAX = 1000 / FPS;
const FRAME_TIME_MIN = FRAME_TIME_MAX / 5;
if (require.main === module) {
  const _prepare = () => {
    if (!process.env['DISPLAY']) {
      process.env['DISPLAY'] = ':0.0';
    }

    let rootPath = null;
    let runtimePath = null;
    const platform = os.platform();
    if (platform === 'linux') {
      rootPath = path.join(os.userInfo().homedir, '.config/openvr');
      runtimePath = path.join(__dirname, 'node_modules', 'native-openvr-deps/bin/linux64');
    } else if (platform === 'darwin') {
      rootPath = path.join('/Users/', os.userInfo().username, '/Library/Application Support/OpenVR/.openvr');
      runtimePath = path.join(__dirname, '/node_modules/native-openvr-deps/bin/osx64');
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
  };
  const _start = () => {
    const _bindWindow = (window, newWindowCb) => {
      window.innerWidth = innerWidth;
      window.innerHeight = innerHeight;
      if (nativeVr.VR_IsHmdPresent()) {
        window.navigator.setVRMode('vr');
      }
      window.addEventListener('error', err => {
        console.warn('got error', err.error.stack);
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

      let lastFrameTime = Date.now();
      const leftGamepad = new window.Gamepad('left', 0);
      const rightGamepad = new window.Gamepad('right', 1);
      const gamepads = [null, null];
      const frameData = new window.VRFrameData();
      const stageParameters = new window.VRStageParameters();
      let timeout = null;
      const _recurse = () => {
        if (vrPresentState.isPresenting) {
          // wait for frame
          vrPresentState.compositor.WaitGetPoses(
            vrPresentState.system,
            localFloat32Array, // hmd
            localFloat32Array2, // left controller
            localFloat32Array3 // right controller
          );
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
            .multiply(hmdMatrix)
            .toArray(frameData.leftViewMatrix);

          vrPresentState.system.GetProjectionMatrix(0, depthNear, depthFar, localFloat32Array4);
          _normalizeMatrixArray(localFloat32Array4);
          frameData.leftProjectionMatrix.set(localFloat32Array4);

          vrPresentState.system.GetEyeToHeadTransform(1, localFloat32Array4);
          _normalizeMatrixArray(localFloat32Array4);
          localMatrix2.fromArray(localFloat32Array4)
            .getInverse(localMatrix2)
            .multiply(hmdMatrix)
            .toArray(frameData.rightViewMatrix);

          vrPresentState.system.GetProjectionMatrix(1, depthNear, depthFar, localFloat32Array4);
          _normalizeMatrixArray(localFloat32Array4);
          frameData.rightProjectionMatrix.set(localFloat32Array4);

          // build stage parameters
          vrPresentState.system.GetSeatedZeroPoseToStandingAbsoluteTrackingPose(localFloat32Array4);
          _normalizeMatrixArray(localFloat32Array4);
          stageParameters.sittingToStandingTransform.set(localFloat32Array4);

          // build gamepads data
          vrPresentState.system.GetControllerState(0, localGamepadArray);
          if (!isNaN(localGamepadArray[0])) {
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

          // update vr frame
          window.updateVrFrame({
            depthNear: 0.1,
            depthFar: 1000.0,
            renderWidth,
            renderHeight,
            frameData,
            stageParameters,
            gamepads,
          });
        }

        // poll for window events
        nativeWindow.pollEvents();

        // update media frames
        nativeVideo.Video.updateAll();

        // trigger requestAnimationFrame
        window.tickAnimationFrame();

        // submit frame
        for (let i = 0; i < contexts.length; i++) {
          const context = contexts[i];
          if (context.isDirty()) {
            if (vrPresentState.glContext === context) {
              nativeWindow.setCurrentWindowContext(context.getWindowHandle());

              nativeWindow.blitFrameBuffer(vrPresentState.msFbo, vrPresentState.fbo, renderWidth * 2, renderHeight, renderWidth * 2, renderHeight);
              vrPresentState.compositor.Submit(vrPresentState.tex);

              nativeWindow.blitFrameBuffer(vrPresentState.fbo, 0, renderWidth * 2, renderHeight, window.innerWidth, window.innerHeight);

              nativeWindow.bindFrameBuffer(vrPresentState.msFbo);
            }
            nativeWindow.swapBuffers(context.getWindowHandle());

            context.clearDirty();
          }
        }

        // wait for next frame
        const now = Date.now();
        timeout = setTimeout(_recurse, Math.min(Math.max(FRAME_TIME_MAX - (now - lastFrameTime), FRAME_TIME_MIN), FRAME_TIME_MAX));
        lastFrameTime = now;
      };
      _recurse();

      window.on('unload', () => {
        clearTimeout(timeout);
      });
      window.on('navigate', newWindowCb);
    };

    const url = process.argv[2];
    if (url) {
      return exokit.load(url)
        .then(window => {
          const _bindHeadlessWindow = newWindow => {
            _bindWindow(newWindow, _bindHeadlessWindow);
          };
          _bindHeadlessWindow(window);
        });
    } else {
      let window;
      const _bindReplWindow = newWindow => {
        window = newWindow;
        _bindWindow(window, _bindReplWindow);
        if (!vm.isContext(window)) {
          vm.createContext(window);
        }
      };
      _bindReplWindow(exokit());

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
            result = vm.runInContext(cmd, window, {filename});
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
      r.defineCommand('redirect', {
        help: 'Redirect <url1> to <url2>',
        action(url) {
          const [url1, url2] = url.split(' ');
          if (url1 && url2) {
            window.redirect(url1, url2);
          } else {
            console.warn('invalid arguments');
          }
          this.clearBufferedCommand();
          this.displayPrompt();
        }
      });
      replHistory(r, path.join(process.env.HOME || process.cwd(), '.exokit_history'));
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
