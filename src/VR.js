const {EventEmitter} = require('events');
const {parentPort} = require('worker_threads');

const {Event} = require('./Event');
const symbols = require('./symbols');
const THREE = require('../lib/three-min.js');
const {
  nativeWindow,
  nativeOculusVR,
  nativeOpenVR,
  nativeOculusMobileVr,
  nativeMl,
} = require('./native-bindings.js');
const {defaultEyeSeparation, maxNumTrackers} = require('./constants.js');
const GlobalContext = require('./GlobalContext');

const localVector = new THREE.Vector3();
const localVector2 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();
const localViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);

class VRPose {
  constructor() {
    this.position = new Float32Array(3);
    this.orientation = new Float32Array(4);
  }

  /* set(position, orientation) {
    position.toArray(this.position);
    orientation.toArray(this.orientation);
  } */

  /* copy(vrPose) {
    if (this.position) {
      this.position.set(vrPose.position);
    }
    if (this.orientation) {
      this.orientation.set(vrPose.orientation);
    }
  } */
}
class VRFrameData {
  constructor() {
    this.leftProjectionMatrix = new Float32Array(16);
    this.leftViewMatrix = new Float32Array(16);
    this.rightProjectionMatrix = new Float32Array(16);
    this.rightViewMatrix = new Float32Array(16);

    this.pose = new VRPose();
  }
}
class GamepadButton {
  constructor(_value, _pressed, _touched) {
    this._value = _value;
    this._pressed = _pressed;
    this._touched = _touched;
  }

  get value() {
    return this._value[0];
  }
  set value(value) {
    this._value[0] = value;
  }
  get pressed() {
    return this._pressed[0] !== 0;
  }
  set pressed(pressed) {
    this._pressed[0] = pressed ? 1 : 0;
  }
  get touched() {
    return this._touched[0] !== 0;
  }
  set touched(touched) {
    this._touched[0] = touched ? 1 : 0;
  }

  /* copy(button) {
    this.value = button.value;
    this.pressed = button.pressed;
    this.touched = button.touched;
  } */
}
class GamepadPose {
  constructor(position, orientation) {
    this.hasPosition = true;
    this.hasOrientation = true;
    this.position = position;
    this.linearVelocity = new Float32Array(3);
    this.linearAcceleration = new Float32Array(3);
    this.orientation = orientation;
    this.angularVelocity = new Float32Array(3);
    this.angularAcceleration = new Float32Array(3);
  }

  /* copy(pose) {
    this.hasPosition = pose.hasPosition;
    this.hasOrientation = pose.hasOrientation;
    this.position.set(pose.position);
    this.linearVelocity.set(pose.linearVelocity);
    this.linearAcceleration.set(pose.linearAcceleration);
    this.orientation.set(pose.orientation);
    this.angularVelocity.set(pose.angularVelocity);
    this.angularAcceleration.set(pose.angularAcceleration);
  } */
}
class GamepadHapticActuator {
  constructor(index) {
    this.index = index;
  }
  get type() {
    return 'vibration';
  }
  set type(type) {}
  pulse(value, duration) {
    parentPort.postMessage({
      method: 'emit',
      type: 'hapticPulse',
      event: {
        index: this.index,
        value,
        duration,
      },
    });
  }
}
class Gamepad {
  constructor(hand, index, id) {
    this.id = id;
    this.hand = hand;
    this.index = index;

    const gamepad = GlobalContext.xrState.gamepads[index];

    this.mapping = 'standard';
    this.buttons = (() => {
      const result = Array(5);
      for (let i = 0; i < result.length; i++) {
        result[i] = new GamepadButton(gamepad.buttons[i].value, gamepad.buttons[i].pressed, gamepad.buttons[i].touched);
      }
      return result;
    })();
    this.pose = new GamepadPose(gamepad.position, gamepad.orientation);
    this.axes = gamepad.axes;
    this.hapticActuators = [new GamepadHapticActuator(index)];
  }

  get connected() {
    return GlobalContext.xrState.gamepads[this.index].connected[0] !== 0;
  }
  set connected(connected) {}

  /* copy(gamepad) {
    this.connected = gamepad.connected;
    for (let i = 0; i < this.buttons.length; i++) {
      this.buttons[i].copy(gamepad.buttons[i]);
    }
    this.pose.copy(gamepad.pose);
    this.axes.set(gamepad.axes);
  } */
}
class VRStageParameters {
  constructor() {
    // new THREE.Matrix4().compose(new THREE.Vector3(0, 0, 0), new THREE.Quaternion(), new THREE.Vector3(1, 1, 1)).toArray(new Float32Array(16))
    this.sittingToStandingTransform = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }

  /* copy(vrStageParameters) {
    this.sittingToStandingTransform.set(vrStageParameters.sittingToStandingTransform);
  } */
}

class VRDisplay extends EventEmitter {
  constructor(displayName, window) {
    super();

    this.displayName = displayName;
    this.window = window;

    this.isConnected = true;
    this.isPresenting = false;
    this.capabilities = {
      canPresent: true,
      hasExternalDisplay: true,
      hasPosition: true,
      maxLayers: 1,
    };
    this.stageParameters = new VRStageParameters();

    this.onrequestpresent = null;
    this.onmakeswapchain = null;
    this.onexitpresent = null;
    this.onrequestanimationframe = null;
    this.onvrdisplaypresentchange = null;

    this._frameData = new VRFrameData();
    this._rafs = [];
    this._layers = [];
  }

  getFrameData(frameData) {
    const {xrOffset} = this.window.document;
    if (xrOffset) {
      localMatrix2.compose(
        localVector.fromArray(xrOffset.position),
        localQuaternion.fromArray(xrOffset.orientation),
        localVector2.fromArray(xrOffset.scale)
      )
      // left
      localMatrix
        .fromArray(GlobalContext.xrState.leftViewMatrix)
        .multiply(
          localMatrix2
        )
        .toArray(frameData.leftViewMatrix);
      // right
      localMatrix
        .fromArray(GlobalContext.xrState.rightViewMatrix)
        .multiply(
          localMatrix2
        )
        .toArray(frameData.rightViewMatrix);
      // pose
      localMatrix
        .compose(localVector.fromArray(GlobalContext.xrState.position), localQuaternion.fromArray(GlobalContext.xrState.orientation), localVector2.set(1, 1, 1))
        .multiply(
          localMatrix2
        )
        .decompose(localVector, localQuaternion, localVector2);
      localVector.toArray(frameData.pose.position);
      localQuaternion.toArray(frameData.pose.orientation);
    } else {
      frameData.leftViewMatrix.set(GlobalContext.xrState.leftViewMatrix);
      frameData.rightViewMatrix.set(GlobalContext.xrState.rightViewMatrix);
      frameData.pose.position.set(GlobalContext.xrState.position);
      frameData.pose.orientation.set(GlobalContext.xrState.orientation);
    }

    frameData.leftProjectionMatrix.set(GlobalContext.xrState.leftProjectionMatrix);
    frameData.rightProjectionMatrix.set(GlobalContext.xrState.rightProjectionMatrix);
  }

  getLayers() {
    return [
      {
        leftBounds: [0, 0, 0.5, 1],
        rightBounds: [0.5, 0, 0.5, 1],
        source: null,
      }
    ];
  }

  getEyeParameters(eye) {
    const leftEye = eye === 'left';
    const _fovArrayToVRFieldOfView = fovArray => ({
      leftDegrees: fovArray[0],
      rightDegrees: fovArray[1],
      upDegrees: fovArray[2],
      downDegrees: fovArray[3],
    });
    return {
      renderWidth: GlobalContext.xrState.renderWidth[0],
      renderHeight:  GlobalContext.xrState.renderHeight[0],
      offset: leftEye ? GlobalContext.xrState.leftOffset : GlobalContext.xrState.rightOffset,
      fieldOfView: _fovArrayToVRFieldOfView(leftEye ? GlobalContext.xrState.leftFov : GlobalContext.xrState.rightFov),
    };
  }

  get depthNear() {
    return GlobalContext.xrState.depthNear[0];
  }
  set depthNear(depthNear) {
    GlobalContext.xrState.depthNear[0] = depthNear;
  }
  get depthFar() {
    return GlobalContext.xrState.depthFar[0];
  }
  set depthFar(depthFar) {
    GlobalContext.xrState.depthFar[0] = depthFar;
  }

  async requestPresent(layers) {
    await this.onrequestpresent();

    const [{source: canvas}] = layers;
    const context = canvas._context || canvas.getContext('webgl');
    this.onmakeswapchain(context);

    if (this.onvrdisplaypresentchange && !this.isPresenting) {
      this.isPresenting = true;
      setImmediate(() => {
        this.onvrdisplaypresentchange();
      });
    } else {
      this.isPresenting = true;
    }
  }

  async exitPresent() {
    for (let i = 0; i < this._rafs.length; i++) {
      this.cancelAnimationFrame(this._rafs[i]);
    }
    this._rafs.length = 0;
    
    await this.onexitpresent();

    if (this.onvrdisplaypresentchange && this.isPresenting) {
      this.isPresenting = false;
      setImmediate(() => {
        this.onvrdisplaypresentchange();
      });
    } else {
      this.isPresenting = false;
    }
  }

  requestAnimationFrame(fn) {
    if (this.onrequestanimationframe) {
      const animationFrame = this.onrequestanimationframe(timestamp => {
        this._rafs.splice(this._rafs.indexOf(animationFrame), 1);
        fn(timestamp);
      });
      this._rafs.push(animationFrame);
      return animationFrame;
    }
  }

  cancelAnimationFrame(animationFrame) {
    if (this.oncancelanimationframe) {
      const result = this.oncancelanimationframe(animationFrame);
      const index = this._rafs.indexOf(animationFrame);
      if (index !== -1) {
        this._rafs.splice(index, 1);
      }
      return result;
    }
  }

  submitFrame() {}

  get layers() {
    return this._layers;
  }
  set layers(layers) {
    this._layers = layers;

    if (this.onlayers) {
      this.onlayers(layers);
    }
  }

  destroy() {
    for (let i = 0; i < this._rafs.length; i++) {
      this.cancelAnimationFrame(this._rafs[i]);
    }
  }
}

class FakeXRDisplay {
  constructor() {
    this.position = new THREE.Vector3();
    this.quaternion = new THREE.Quaternion();
  }

  pushUpdate() {
    // update hmd
    this.position.toArray(GlobalContext.xrState.position);
    this.quaternion.toArray(GlobalContext.xrState.orientation);

    localMatrix.compose(
      this.position,
      this.quaternion,
      localVector2.set(1, 1, 1)
    )
      .getInverse(localMatrix)
      .toArray(GlobalContext.xrState.leftViewMatrix);
    GlobalContext.xrState.rightViewMatrix.set(GlobalContext.xrState.leftViewMatrix);

    // update gamepads
    const gamepads = navigator.getGamepads();
    for (let i = 0; i < 2 && i < gamepads.length; i++) {
      const gamepad = gamepads[i];
      localVector.copy(this.position)
        .add(
          localVector2.set(-0.3 + i*0.6, -0.3, 0)
            .applyQuaternion(this.quaternion)
        ).toArray(gamepad.pose.position);
      this.quaternion.toArray(gamepad.pose.orientation); // XXX updates xrState

      localMatrix2
        .compose(
          localVector.fromArray(gamepad.pose.position),
          localQuaternion.fromArray(gamepad.pose.orientation),
          localVector2.set(1, 1, 1)
        )
        .toArray(gamepad.pose._localPointerMatrix);

      GlobalContext.xrState.gamepads[i].connected[0] = 1;
    }
  }
}

const getHMDType = () => {
  if (GlobalContext.xrState.fakeVrDisplayEnabled[0]) {
    return 'fake';
  } else if (nativeOculusVR && nativeOculusVR.Oculus_IsHmdPresent()) {
    return 'oculus';
  } else if (nativeOpenVR && nativeOpenVR.VR_IsHmdPresent()) {
    return 'openvr';
  } else if (nativeOculusMobileVr && nativeOculusMobileVr.OculusMobile_IsHmdPresent()) {
    return 'oculusMobile';
  } else if (nativeMl && nativeMl.IsPresent()) {
    return 'magicleap';
  } else {
    return null;
  }
};
const hmdTypes = [
  'fake',
  'oculus',
  'openvr',
  'oculusMobile',
  'magicleap',
];
const hmdTypeIndexMap = (() => {
  const result = {};
  hmdTypes.forEach((t, i) => {
    result[t] = i;
  });
  return result;
})();
const lookupHMDTypeIndex = s => hmdTypeIndexMap[s];
const hmdTypeStringMap = (() => {
  const result = {};
  hmdTypes.forEach((t, i) => {
    result[i] = t;
  });
  return result;
})();
const lookupHMDTypeString = i => hmdTypeStringMap[i];

const createFakeXRDisplay = () => new FakeXRDisplay();

const controllerIDs = {
  fake: 'OpenVR Gamepad',
  openvr: 'OpenVR Gamepad',
  // oculusMobile: 'Oculus Go',
  openvrTracker: 'OpenVR Tracker',
  oculusLeft: 'Oculus Touch (Left)',
  oculusRight: 'Oculus Touch (Right)',
  oculusMobileLeft: 'Oculus Touch (Left)',
  oculusMobileRight: 'Oculus Touch (Right)',
};

function getControllerID(hmdType, hand) {
  return controllerIDs[hmdType] || controllerIDs[hmdType + hand.charAt(0).toUpperCase() + hand.slice(1)];
}

let gamepads = null;
function getGamepads(window) {
  if (GlobalContext.xrState.isPresenting[0]) {
    if (!gamepads) {
      const hmdType = getHMDType();

      let numGamepads = 2;
      if (hmdType === 'openvr') {
        numGamepads += maxNumTrackers;
      }
      gamepads = Array(numGamepads);
      for (let i = 0; i < gamepads.length; i++) {
        let hand, id;
        if (i === 0) {
          hand = 'left';
          id = getControllerID(hmdType, hand);
        } else if (i === 1) {
          hand = 'right';
          id = getControllerID(hmdType, hand);
        } else {
          hand = null;
          id = controllerIDs['openvrTracker'];
        }
        gamepads[i] = new Gamepad(hand, i, id);
      }
    }
    return gamepads;
  } else {
    return [];
  }
};
GlobalContext.getGamepads = getGamepads;
function clearGamepads() {
  gamepads = null;
}
GlobalContext.clearGamepads = clearGamepads;

module.exports = {
  VRDisplay,
  FakeXRDisplay,
  VRFrameData,
  VRPose,
  VRStageParameters,
  Gamepad,
  GamepadButton,
  getHMDType,
  lookupHMDTypeString,
  lookupHMDTypeIndex,
  createFakeXRDisplay,
  getGamepads
};
