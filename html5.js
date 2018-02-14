const path = require('path');
const exokit = require('exokit');
const nativeBindingsModulePath = path.join(__dirname, 'native-bindings.js');
exokit.setNativeBindingsModule(nativeBindingsModulePath);
const {THREE} = exokit;
const nativeBindings = require(nativeBindingsModulePath);
const {nativeVr, nativeWindow} = nativeBindings;

// ENVIRONMENT

process.execArgv = []; // do not inherit node arguments in forked child processes

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

const nop = () => {};
nativeWindow.events.emit = (type, data) => {
  // console.log(type, data);

  switch (type) {
    case 'resize': {
      const {width, height} = data;
      innerWidth = width;
      innerHeight = height;

      if (window) {
        window.innerWidth = innerWidth;
        window.innerHeight = innerHeight;
        window.emit('resize');
      }
      break;
    }
    case 'keydown':
    case 'keyup':
    case 'keypress':
    case 'mousedown':
    case 'mouseup':
    case 'click': {
      data.preventDefault = nop;
      data.preventStopPropagation = nop;
      data.preventStopImmediatePropagation = nop;
      
      window.emit(type, data);
      break;
    }
    case 'mousemove': {
      data.preventDefault = nop;
      data.preventStopPropagation = nop;
      data.preventStopImmediatePropagation = nop;
      
      if (window.document.pointerLockElement) {
        data.movementX = data.pageX - (window.innerWidth / window.devicePixelRatio / 2);
        data.movementY = data.pageY - (window.innerHeight / window.devicePixelRatio / 2);

        nativeWindow.setCursorPosition(window.innerWidth / 2, window.innerHeight / 2);
      }
      
      window.emit(type, data);
      break;
    }
    case 'quit': {
      process.exit();
      break;
    }
  }
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

let system = null;
let compositor = null;
let msFbo = null;
let msTexture = null;
let fbo = null;
let texture = null;
let renderWidth = 0;
let renderHeight = 0;
const depthNear = 0.1;
const depthFar = 1000.0;
nativeVr.requestPresent = function() {
  // while booting we sometimes get transient errors
  const _requestSystem = () => new Promise((accept, reject) => {
    let err = null;
    const _recurse = (i = 0) => {
      if (i < 20) {
        const system = (() => {
          try {
            return nativeVr.system.VR_Init(nativeVr.EVRApplicationType.Scene);
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

  return _requestSystem()
    .then(newSystem => {
      system = newSystem;
      compositor = nativeVr.compositor.NewCompositor();

      const {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
      renderWidth = halfWidth;
      renderHeight = height;

      window.updateVrFrame({
        renderWidth,
        renderHeight,
      });

      const width = halfWidth * 2;
      const [msFb, msTex] = nativeWindow.getRenderTarget(width, height, 4);
      msFbo = msFb;
      msTexture = msTex;
      const [fb, tex] = nativeWindow.getRenderTarget(width, height, 1);
      fbo = fb;
      texture = tex;
    });
};
nativeVr.exitPresent = function() {
  nativeVr.system.VR_Shutdown();
  system = null;
  compositor = null;
  
  return Promise.resolve();
};

// EXPORTS

module.exports = exokit;

// MAIN

let window = null;
let innerWidth = 1280;
let innerHeight = 1024;
const FPS = 90;
const FRAME_TIME_MAX = 1000 / FPS;
const FRAME_TIME_MIN = FRAME_TIME_MAX / 5;
if (require.main === module) {
  const url = process.argv[2] || 'http://localhost:8000';
  
  nativeWindow.create(innerWidth, innerHeight);

  exokit.fetch(url)
    .then(site => {
      console.log('node site loaded');

      window = site.window;
      window.innerWidth = innerWidth;
      window.innerHeight = innerHeight;
      if (nativeVr.system.VR_IsHmdPresent()) { // XXX hook this up internally in exokit
        window.navigator.setVRMode('vr');
      }
      window.addEventListener('error', err => {
        console.warn('got error', err.error.stack);
      });

      let lastFrameTime = Date.now();
      const leftGamepad = new window.Gamepad('left', 0);
      const rightGamepad = new window.Gamepad('right', 1);
      const gamepads = [null, null];
      const frameData = new window.VRFrameData();
      const stageParameters = new window.VRStageParameters();
      const _recurse = () => {
        if (compositor) {
          // wait for frame
          compositor.WaitGetPoses(
            system,
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

          system.GetEyeToHeadTransform(0, localFloat32Array4);
          localMatrix2.fromArray(localFloat32Array4)
            .getInverse(localMatrix2)
            .multiply(hmdMatrix)
            .toArray(frameData.leftViewMatrix);

          system.GetProjectionMatrix(0, depthNear, depthFar, localFloat32Array4);
          _normalizeMatrixArray(localFloat32Array4);
          frameData.leftProjectionMatrix.set(localFloat32Array4);

          system.GetEyeToHeadTransform(1, localFloat32Array4);
          _normalizeMatrixArray(localFloat32Array4);
          localMatrix2.fromArray(localFloat32Array4)
            .getInverse(localMatrix2)
            .multiply(hmdMatrix)
            .toArray(frameData.rightViewMatrix);

          system.GetProjectionMatrix(1, depthNear, depthFar, localFloat32Array4);
          _normalizeMatrixArray(localFloat32Array4);
          frameData.rightProjectionMatrix.set(localFloat32Array4);

          // build stage parameters
          system.GetSeatedZeroPoseToStandingAbsoluteTrackingPose(localFloat32Array4);
          _normalizeMatrixArray(localFloat32Array4);
          stageParameters.sittingToStandingTransform.set(localFloat32Array4);
          
          // build gamepads data
          system.GetControllerState(0, localGamepadArray);
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
          
          system.GetControllerState(1, localGamepadArray);
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

          // bind framebuffer for rendering
          nativeWindow.bindFrameBuffer(msFbo);
        } else {
          // bind framebuffer for rendering
          nativeWindow.bindFrameBuffer(0);
        }

        // poll for window events
        nativeWindow.pollEvents();

        // trigger requestAnimationFrame
        window.tickAnimationFrame();

        // submit to compositor
        if (compositor) {
          nativeWindow.blitFrameBuffer(msFbo, fbo, renderWidth * 2, renderHeight, renderWidth * 2, renderHeight);
          compositor.Submit(texture);

          nativeWindow.blitFrameBuffer(fbo, 0, renderWidth * 2, renderHeight, window.innerWidth, window.innerHeight);
        }
        nativeWindow.swapBuffers();

        // wait for next frame
        const now = Date.now();
        setTimeout(_recurse, Math.min(Math.max(FRAME_TIME_MAX - (now - lastFrameTime), FRAME_TIME_MIN), FRAME_TIME_MAX));
        lastFrameTime = now;
      };
      _recurse();
    });
}

process.on('uncaughtException', err => {
  console.warn(err.stack);
});
process.on('unhandledRejection', err => {
  console.warn(err.stack);
});
