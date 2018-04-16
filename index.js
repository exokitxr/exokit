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
const minimist = require('minimist');
const emojis = require('./assets/emojis');
const nativeBindingsModulePath = path.join(__dirname, 'native-bindings.js');
const {THREE} = exokit;
const nativeBindings = require(nativeBindingsModulePath);
const {nativeVideo, nativeVr, nativeMl, nativeWindow} = nativeBindings;

const dataPath = __dirname;

const canvasSymbol = Symbol();
const contexts = [];
const _windowHandleEquals = (a, b) => a[0] === b[0] && a[1] === b[1];
const _isAttached = el => {
  for (;;) {
    if (el === el.ownerDocument.documentElement) {
      return true;
    } else if (el.parentNode) {
      el = el.parentNode;
    } else {
      return false;
    }
  }
};

const args = (() => {
  if (require.main === module) {
    const minimistArgs = minimist(process.argv.slice(2), {
      boolean: [
        'p',
        'perf',
        'performance',
        'f',
        'frame',
        'm',
        'minimalFrame',
      ],
      string: [
        's',
        'size',
      ],
      alias: {
        p: 'performance',
        perf: 'performance',
        s: 'size',
        f: 'frame',
        m: 'minimalFrame',
      },
    });
    return {
      url: minimistArgs._[0] || '',
      performance: !!minimistArgs.performance,
      size: minimistArgs.size,
      frame: minimistArgs.frame,
      minimalFrame: minimistArgs.minimalFrame,
    };
  } else {
    return args;
  }
})();
exokit.setArgs(args);
console.log('args', args);
exokit.setNativeBindingsModule(nativeBindingsModulePath);

nativeBindings.nativeGl.onconstruct = (gl, canvas) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;
  const windowHandle = (() => {
    try {
      return nativeWindow.create(canvasWidth, canvasHeight, _isAttached(canvas));
    } catch (err) {
      console.warn(err.message);
    }
  })();
  if (windowHandle) {
    gl.setWindowHandle(windowHandle);

    gl[canvasSymbol] = canvas;

    const window = canvas.ownerDocument.defaultView;
    const framebufferWidth = nativeWindow.getFramebufferSize(windowHandle).width;
    window.devicePixelRatio = framebufferWidth / canvasWidth;

    const title = `Exokit v${require('./package.json').version}`
    nativeWindow.setWindowTitle(windowHandle, title);

    const ondomchange = () => {
      if (_isAttached(canvas)) {
        nativeWindow.show(windowHandle);
      } else {
        nativeWindow.hide(windowHandle);
      }
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

const maxNumPlanes = 32 * 3;
const planeEntrySize = 3 + 4 + 2 + 1;
const framebufferArray = new Uint32Array(2);
const transformArray = new Float32Array(7 * 2);
const projectionArray = new Float32Array(16 * 2);
const viewportArray = new Uint32Array(4);
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
              const [msFbo, msTex] = nativeWindow.createRenderTarget(width, height, 4);
              const [fbo, tex] = nativeWindow.createRenderTarget(width, height, 1);

              context.setDefaultFramebuffer(msFbo);
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

          const [fbo, tex] = nativeWindow.createRenderTarget(window.innerWidth, window.innerHeight, 1);
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
          nativeWindow.bindFrameBuffer(mlFbo);

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
      // nativeWindow.bindFrameBuffer(0);
    }

    return Promise.resolve();
  };
}

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
        } else {
          _dispatchCanvasEvent(canvas, new window.KeyboardEvent(type, data));
        }
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
          data.movementX = data.pageX - (window.innerWidth / 2);
          data.movementY = data.pageY - (window.innerHeight / 2);

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
          });

          vrPresentState.glContext.setDefaultFramebuffer(vrPresentState.msFbo);
          nativeWindow.bindFrameBuffer(vrPresentState.msFbo);

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

          mlGlContext.setDefaultFramebuffer(mlFbo);
          nativeWindow.bindFrameBuffer(mlFbo);

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
        window.tickAnimationFrame();
        if (args.frame) {
          console.log('-'.repeat(80) + 'end frame');
        }
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

              nativeWindow.blitFrameBuffer(vrPresentState.msFbo, vrPresentState.fbo, renderWidth * 2, renderHeight, renderWidth * 2, renderHeight, true, false, false);
              vrPresentState.compositor.Submit(vrPresentState.tex);

              nativeWindow.blitFrameBuffer(vrPresentState.fbo, 0, renderWidth, renderHeight, window.innerWidth, window.innerHeight, true, false, false);

              context.setDefaultFramebuffer(0);
              nativeWindow.bindFrameBuffer(0);
            } else if (mlGlContext === context) {
              nativeWindow.setCurrentWindowContext(context.getWindowHandle());

              mlContext.SubmitFrame(mlFbo, window.innerWidth, window.innerHeight);

              nativeWindow.blitFrameBuffer(mlFbo, 0, window.innerWidth, window.innerHeight, window.innerWidth, window.innerHeight, true, false, false);

              context.setDefaultFramebuffer(mlFbo);
              nativeWindow.bindFrameBuffer(mlFbo);
            }
            nativeWindow.swapBuffers(context.getWindowHandle());

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

      window.on('unload', () => {
        clearTimeout(timeout);
      });
      window.on('navigate', newWindowCb);
    };

    let {url} = args;
    if (url) {
      if (url === '.') {
        console.warn('NOTE: You ran `exokit . <url>`\n(Did you mean to run `node . <url>` or `exokit <url>` instead?)')
      }
      if (url.indexOf('://') < 0) {
        url = 'http://' + url;
      }
      return exokit.load(url, {
        dataPath,
      })
        .then(window => {
          const _bindHeadlessWindow = newWindow => {
            _bindWindow(newWindow, _bindHeadlessWindow);
          };
          _bindHeadlessWindow(window);
        });
    } else {
      let window = null;
      const _bindReplWindow = newWindow => {
        _bindWindow(newWindow, _bindReplWindow);
        if (!vm.isContext(newWindow)) {
          vm.createContext(newWindow);
        }
        window = newWindow;
      };
      _bindReplWindow(exokit('', {
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
