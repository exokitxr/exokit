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
// const replHistory = require('repl.history');
const minimist = require('minimist');

const {version} = require('../package.json');
const {defaultEyeSeparation, maxNumTrackers} = require('./constants.js');
const symbols = require('./symbols');
const THREE = require('../lib/three-min.js');

const {getHMDType} = require('./VR.js');

const nativeBindings = require(path.join(__dirname, 'native-bindings.js'));

const GlobalContext = require('./GlobalContext');
GlobalContext.args = {};
GlobalContext.version = '';
GlobalContext.commands = [];

const localVector = new THREE.Vector3();
const localVector2 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();

// openvr
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
const zeroMatrix = new THREE.Matrix4();
const localFloat32Array = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array2 = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array3 = zeroMatrix.toArray(new Float32Array(16));
const localFloat32Array4 = zeroMatrix.toArray(new Float32Array(16));

const localPositionArray3 = new Float32Array(3);
const localQuaternionArray4 = new Float32Array(4);

const leftControllerPositionArray3 = new Float32Array(3);
const leftControllerQuaternionArray4 = new Float32Array(4);

const rightControllerPositionArray3 = new Float32Array(3);
const rightControllerQuaternionArray4 = new Float32Array(4);

// oculus mobile
const oculusMobilePoseFloat32Array = new Float32Array(3 + 4 + 1 + 4 + (16*2) + (16*2) + (16+12) + (16+12));

// magic leap
const transformArray = new Float32Array(7 * 2);
const projectionArray = new Float32Array(16 * 2);
const controllersArray = new Float32Array((1 + 3 + 4 + 6) * 2);

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
        'nogl',
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
        n: 'nogl',
        e: 'headless',
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
      nogl: minimistArgs.nogl,
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
  const _makeTypedArray = _makeSab(4*1024);

  const result = {};
  result.isPresenting = _makeTypedArray(Uint32Array, 1);
  result.renderWidth = _makeTypedArray(Float32Array, 1);
  result.renderWidth[0] = 1920/2;
  result.renderHeight = _makeTypedArray(Float32Array, 1);
  result.renderHeight[0] = 1080;
  result.metrics = _makeTypedArray(Uint32Array, 2);
  result.devicePixelRatio = _makeTypedArray(Uint32Array, 1);
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
  result.vrRequest = _makeTypedArray(Uint32Array, 2);
  result.tex = _makeTypedArray(Uint32Array, 1);
  result.depthTex = _makeTypedArray(Uint32Array, 1);
  result.fakeVrDisplayEnabled = _makeTypedArray(Uint32Array, 1);

  return result;
})();
GlobalContext.xrState = xrState;

const topVrPresentState = {
  hmdType: null,
  windowHandle: null,
  fbo: 0,
  tex: 0,
  depthTex: 0,
  vrContext: null,
  vrSystem: null,
  vrCompositor: null,
  hasPose: false,
};

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

  if (nativeBindings.nativeWindow.pollEvents) {
    setInterval(() => {
      nativeBindings.nativeWindow.pollEvents();
    }, 1000/60); // XXX make this run at the native frame rate
  }

  const _handleRequestPresent = async () => {
    const vrRequestMethod = xrState.vrRequest[0];

    if (vrRequestMethod === 1) { // requestPresent
      const hmdType = getHMDType();
      console.log('request present', hmdType); // XXX

      if (!topVrPresentState.windowHandle) {
        topVrPresentState.windowHandle = nativeBindings.nativeWindow.createWindowHandle(1, 1, false);
      }
      nativeBindings.nativeWindow.setCurrentWindowContext(topVrPresentState.windowHandle);

      if (hmdType === 'fake') {
        const width = xrState.renderWidth[0]*2;
        const height = xrState.renderHeight[0];

        const [fbo, tex, depthTex] = nativeBindings.nativeWindow.createVrTopRenderTarget(width, height);

        xrState.tex[0] = tex;
        xrState.depthTex[0] = depthTex;
        // xrState.renderWidth[0] = halfWidth;
        // xrState.renderHeight[0] = height;
      } else if (hmdType === 'oculus') {
        const system = topVrPresentState.oculusSystem || nativeBindings.nativeOculusVR.Oculus_Init();
        // const lmContext = topVrPresentState.lmContext || (nativeBindings.nativeLm && new nativeBindings.nativeLm());

        topVrPresentState.vrContext = system;

        const {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2;

        const [fbo, tex, depthTex] = system.CreateSwapChain(width, height);

        xrState.tex[0] = tex;
        xrState.depthTex[0] = depthTex;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;
      } else if (hmdType === 'openvr') {
        const vrContext = nativeBindings.nativeOpenVR.getContext();
        const system = nativeBindings.nativeOpenVR.VR_Init(nativeBindings.nativeOpenVR.EVRApplicationType.Scene);
        const compositor = vrContext.compositor.NewCompositor();

        // const lmContext = topVrPresentState.lmContext || (nativeLm && new nativeLm());

        topVrPresentState.vrContext = vrContext;
        topVrPresentState.vrSystem = system;
        topVrPresentState.vrCompositor = compositor;

        const {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2;

        const [fbo, tex, depthTex] = nativeBindings.nativeWindow.createVrTopRenderTarget(width, height);

        xrState.tex[0] = tex;
        xrState.depthTex[0] = depthTex;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;
      } else if (hmdType === 'oculusMobile') {
        const vrContext = nativeBindings.nativeOculusMobileVr.OculusMobile_Init(topVrPresentState.windowHandle);

        topVrPresentState.vrContext = vrContext;

        const {width: halfWidth, height} = vrContext.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2;

        const [fbo, tex, depthTex] = nativeBindings.nativeWindow.createVrTopRenderTarget(width, height);

        topVrPresentState.fbo = fbo;
        xrState.tex[0] = tex;
        xrState.depthTex[0] = depthTex;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;
      } else if (hmdType === 'magicleap') {
        topVrPresentState.vrContext = new nativeBindings.nativeMl();
        topVrPresentState.vrContext.Present(topVrPresentState.windowHandle);

        const {width: halfWidth, height} = topVrPresentState.vrContext.GetSize();
        const width = halfWidth * 2;

        const [fbo, tex, depthTex] = nativeBindings.nativeWindow.createVrTopRenderTarget(width, height);

        topVrPresentState.fbo = fbo;
        xrState.tex[0] = tex;
        xrState.depthTex[0] = depthTex;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;
      } else {
        throw new Error('unknown hmd type');
      }

      topVrPresentState.hmdType = hmdType;

      const windowId = xrState.vrRequest[1];
      const window = windows.find(window => window.id === windowId);

      xrState.isPresenting[0] = 1;
      xrState.vrRequest.fill(0);

      if (window) {
        window.runAsync(JSON.stringify({
          method: 'response',
          hmdType,
        }));
      } else {
        console.warn(`no top level window to respond to for request present: ${hmdType} ${windowId} ${JSON.stringify(windows.map(window => window.id))}`);
      }
    } else if (vrRequestMethod === 2) { // exitPresent
      if (topVrPresentState.hmdType === 'fake') {
        // XXX destroy fbo
      } else {
        throw new Error(`fail to exit present for hmd type ${topVrPresentState.hmdType}`);
      }

      topVrPresentState.hmdType = null;

      const windowId = xrState.vrRequest[1];
      const window = windows.find(window => window.id === windowId);

      xrState.isPresenting[0] = 0;
      xrState.vrRequest.fill(0);

      window.runAsync(JSON.stringify({
        method: 'response',
      }));
    }
  };
  const _waitGetPoses = async () => {
    if (topVrPresentState.hmdType === 'oculus') {
      // wait for frame
      await new Promise((accept, reject) => {
        topVrPresentState.vrContext.GetPose(
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
      if (!immediate) {
        return;
      }

      topVrPresentState.hasPose = true;

      xrState.position.set(localPositionArray3);
      xrState.orientation.set(localQuaternionArray4);
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
        topVrPresentState.vrContext.GetControllersInputState(0, localGamepadArray);

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
        topVrPresentState.vrContext.GetControllersInputState(1, localGamepadArray);

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
    } else if (topVrPresentState.hmdType === 'openvr') {
      // wait for frame
      await new Promise((accept, reject) => {
        topVrPresentState.vrCompositor.RequestGetPoses(
          topVrPresentState.vrSystem,
          localFloat32PoseArray, // hmd, controllers, trackers
          accept
        );
      });
      if (!immediate) {
        return;
      }

      topVrPresentState.hasPose = true;

      // hmd pose
      const hmdMatrix = localMatrix.fromArray(localFloat32HmdPoseArray);

      hmdMatrix.decompose(localVector, localQuaternion, localVector2);
      localVector.toArray(xrState.position);
      localQuaternion.toArray(xrState.orientation);

      hmdMatrix.getInverse(hmdMatrix);

      // left eye pose
      topVrPresentState.vrSystem.GetEyeToHeadTransform(0, localFloat32MatrixArray);
      localMatrix2.fromArray(localFloat32MatrixArray);
      localMatrix2.decompose(localVector, localQuaternion, localVector2);
      localVector.toArray(xrState.leftOffset);
      localMatrix2
        .getInverse(localMatrix2)
        .multiply(hmdMatrix);
      localMatrix2.toArray(xrState.leftViewMatrix);

      topVrPresentState.vrSystem.GetProjectionMatrix(0, xrState.depthNear[0], xrState.depthFar[0], localFloat32MatrixArray);
      xrState.leftProjectionMatrix.set(localFloat32MatrixArray);

      topVrPresentState.vrSystem.GetProjectionRaw(0, localFovArray);
      for (let i = 0; i < localFovArray.length; i++) {
        xrState.leftFov[i] = Math.atan(localFovArray[i]) / Math.PI * 180;
      }

      // right eye pose
      topVrPresentState.vrSystem.GetEyeToHeadTransform(1, localFloat32MatrixArray);
      localMatrix2.fromArray(localFloat32MatrixArray);
      localMatrix2.decompose(localVector, localQuaternion, localVector2);
      localVector.toArray(xrState.rightOffset);
      localMatrix2
        .getInverse(localMatrix2)
        .multiply(hmdMatrix);
      localMatrix2.toArray(xrState.rightViewMatrix);

      topVrPresentState.vrSystem.GetProjectionMatrix(1, xrState.depthNear[0], xrState.depthFar[0], localFloat32MatrixArray);
      xrState.rightProjectionMatrix.set(localFloat32MatrixArray);

      topVrPresentState.vrSystem.GetProjectionRaw(1, localFovArray);
      for (let i = 0; i < localFovArray.length; i++) {
        xrState.rightFov[i] = Math.atan(localFovArray[i]) / Math.PI * 180;
      }

      // build stage parameters
      // topVrPresentState.vrSystem.GetSeatedZeroPoseToStandingAbsoluteTrackingPose(localFloat32MatrixArray);
      // stageParameters.sittingToStandingTransform.set(localFloat32MatrixArray);

      // build gamepads data
      const _loadGamepad = i => {
        const gamepad = xrState.gamepads[i];
        if (topVrPresentState.vrSystem.GetControllerState(i, localGamepadArray)) {
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
    } else if (topVrPresentState.hmdType === 'oculusMobile') {
      topVrPresentState.hasPose = await new Promise((accept, reject) => {
        const hasPose = topVrPresentState.vrContext.WaitGetPoses(
          oculusMobilePoseFloat32Array
        );

        accept(hasPose);
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
    } else if (topVrPresentState.hmdType === 'magicleap') {
      topVrPresentState.hasPose = await new Promise((accept, reject) => {
        const hasPose = topVrPresentState.vrContext.WaitGetPoses(
          transformArray,
          projectionArray,
          controllersArray
        );

        accept(hasPose);
      });
      if (!immediate) {
        return;
      }

      if (topVrPresentState.hasPose) {
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
      }

      // queue magic leap state updates
      nativeBindings.nativeMl.Update(topVrPresentState.vrContext);

      /* // prepare magic leap frame
      topVrPresentState.vrContext.PrepareFrame(
        mlGlContext, // gl context for depth population
        topVrPresentState.mlFbo,
        xrState.renderWidth[0]*2,
        xrState.renderHeight[0],
      ); */
    } else {
      /* await new Promise((accept, reject) => {
        const now = Date.now();
        const timeDiff = now - lastFrameTime;
        const waitTime = Math.max(8 - timeDiff, 0);
        setTimeout(accept, waitTime);
      }); */
    }
  };
  const _submitFrame = async () => {
    if (topVrPresentState.hasPose) {
      if (topVrPresentState.hmdType === 'oculus') {
        const [fbo, tex, depthTex] = topVrPresentState.vrContext.Submit();
        xrState.tex[0] = tex;
        xrState.depthTex[0] = depthTex;
      } else if (topVrPresentState.hmdType === 'openvr') {
        topVrPresentState.vrCompositor.Submit(xrState.tex[0]);
      } else if (topVrPresentState.hmdType === 'oculusMobile') {
        topVrPresentState.vrContext.Submit(topVrPresentState.fbo, xrState.renderWidth[0]*2, xrState.renderHeight[0]);
      } else if (topVrPresentState.hmdType === 'magicleap') {
        topVrPresentState.vrContext.SubmitFrame(topVrPresentState.fbo, xrState.renderWidth[0]*2, xrState.renderHeight[0]);
      }

      topVrPresentState.hasPose = false;
    }
  };
  const _topRenderLoop = async () => {
    if (args.performance) {
      if (timestamps.frames >= TIMESTAMP_FRAMES) {
        console.log(`${(TIMESTAMP_FRAMES/(timestamps.total/1000)).toFixed(0)} FPS | ${timestamps.idle/TIMESTAMP_FRAMES}ms idle | ${timestamps.wait/TIMESTAMP_FRAMES}ms wait | ${timestamps.prepare/TIMESTAMP_FRAMES}ms prepare | ${timestamps.events/TIMESTAMP_FRAMES}ms events | ${timestamps.media/TIMESTAMP_FRAMES}ms media | ${timestamps.user/TIMESTAMP_FRAMES}ms user | ${timestamps.submit/TIMESTAMP_FRAMES}ms submit`);

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

    await _handleRequestPresent();
    await _waitGetPoses();

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

    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.wait += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    // poll operating system events
    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.events += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    // update events
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
    await Promise.all(windows.map(window => window.runAsync('tickAnimationFrame')));

    if (args.performance) {
      const now = Date.now();
      const diff = now - timestamps.last;
      timestamps.user += diff;
      timestamps.total += diff;
      timestamps.last = now;
    }

    await _submitFrame();

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
      // u = u.replace(/\/?$/, '/');
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
    const _onnavigate = href => {
      core.load(href, {
        dataPath,
        args,
        replacements,
        onnavigate: _onnavigate,
      });
    };
    _onnavigate(u);
  } else {
    const _onnavigate = href => {
      window = null;

      core.load(href, {
        dataPath,
      }, {
        onnavigate: _onnavigate,
      })
        .then(newWindow => {
          window = newWindow;
        })
        .catch(err => {
          console.warn(err.stack);
        });
    };
    let window = core.make('', {
      dataPath,
      onnavigate: _onnavigate,
    });

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
    // replHistory(r, path.join(dataPath, '.repl_history'));
    r.on('exit', () => {
      process.exit();
    });
  }
};

if (require.main === module) {
  /* if (!nativeBindings.nativePlatform) { // not a mobile platform
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
  } */
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
        xrState.metrics[0] = w;
        xrState.metrics[1] = h;
      }
    }
  }
  if (args.frame || args.minimalFrame) {
    nativeBindings.nativeGl = (OldWebGLRenderingContext => {
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
    })(nativeBindings.nativeGl);
  }

  _prepare()
    .then(() => _start())
    .catch(err => {
      console.warn(err.stack);
      process.exit(1);
    });
}

module.exports = core;
