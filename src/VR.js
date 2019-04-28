const {EventEmitter} = require('events');
const {Event} = require('./Event');
const symbols = require('./symbols');
const THREE = require('../lib/three-min.js');
const {defaultCanvasSize, defaultEyeSeparation, maxNumTrackers} = require('./constants.js');
const {
  nativeOculusVR,
  nativeOpenVR,
  nativeOculusMobileVr,
  nativeMl,
} = require('./native-bindings.js');
const GlobalContext = require('./GlobalContext');

const localVector = new THREE.Vector3();
const localVector2 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();
const localViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);

class VRPose {
  constructor() {
    this.position = GlobalContext.xrState.position;
    this.orientation = GlobalContext.xrState.orientation;
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
    this.leftProjectionMatrix = GlobalContext.xrState.leftProjectionMatrix;
    /* this.leftProjectionMatrix.set(Float32Array.from([
      1.0000000000000002, 0, 0, 0,
      0, 1.0000000000000002, 0, 0,
      0, 0, -1.00010000500025, -1,
      0, 0, -0.200010000500025, 0,
    ])); */
    // c = new THREE.PerspectiveCamera(); c.fov = 90; c.updateProjectionMatrix(); c.projectionMatrix.elements
    this.leftViewMatrix = GlobalContext.xrState.leftViewMatrix;
    // this.leftViewMatrix.set(Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]));
    // new THREE.Matrix4().toArray()

    this.rightProjectionMatrix = GlobalContext.xrState.rightProjectionMatrix;
    this.rightViewMatrix = GlobalContext.xrState.rightViewMatrix;

    this.pose = new VRPose();
  }

  /* copy(frameData) {
    this.leftProjectionMatrix.set(frameData.leftProjectionMatrix);
    this.leftViewMatrix.set(frameData.leftViewMatrix);
    this.rightProjectionMatrix.set(frameData.rightProjectionMatrix);
    this.rightViewMatrix.set(frameData.rightViewMatrix);
    this.pose.copy(frameData.pose);
  } */
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
    if (GlobalContext.vrPresentState.isPresenting) {
      value = Math.min(Math.max(value, 0), 1);
      const deviceIndex = GlobalContext.vrPresentState.system.GetTrackedDeviceIndexForControllerRole(this.index + 1);

      const startTime = Date.now();
      const _recurse = () => {
        if ((Date.now() - startTime) < duration) {
          GlobalContext.vrPresentState.system.TriggerHapticPulse(deviceIndex, 0, value * 4000);
          setTimeout(_recurse, 50);
        }
      };
      setTimeout(_recurse, 50);
    }
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
  constructor(displayName) {
    super();

    this.displayName = displayName;

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
    frameData.leftProjectionMatrix.set(GlobalContext.xrState.leftProjectionMatrix);
    frameData.leftViewMatrix.set(GlobalContext.xrState.leftViewMatrix);
    frameData.rightViewMatrix.set(GlobalContext.xrState.rightViewMatrix);
    frameData.rightProjectionMatrix.set(GlobalContext.xrState.rightProjectionMatrix);
    frameData.pose.position.set(GlobalContext.xrState.position);
    frameData.pose.orientation.set(GlobalContext.xrState.orientation);
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

  async requestPresent(layers) {
    await this.onrequestpresent();
    
    const [{source: canvas}] = layers;
    const {_context: context} = canvas;
    this.session.device.onmakeswapchain(context);

    if (this.onvrdisplaypresentchange && !this.isPresenting) {
      this.isPresenting = true;
      this.onvrdisplaypresentchange();
    } else {
      this.isPresenting = true;
    }
  }

  async exitPresent() {
    if (this.onexitpresent) {
      await this.onexitpresent();
    }

    for (let i = 0; i < this._rafs.length; i++) {
      this.cancelAnimationFrame(this._rafs[i]);
    }
    this._rafs.length = 0;

    if (this.onvrdisplaypresentchange && this.isPresenting) {
      this.isPresenting = false;
      this.onvrdisplaypresentchange();
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

class FakeVRDisplay extends VRDisplay {
  constructor(window) {
    super('FAKE');

    this.window = window;
    this.position = new THREE.Vector3();
    this.quaternion = new THREE.Quaternion();
    this.gamepads = [
      new Gamepad('left', 0),
      new Gamepad('right', 1),
    ];
    for (let i = 0; i < this.gamepads.length; i++) {
      const gamepad = this.gamepads[i];
      gamepad.handedness = gamepad.hand;
      gamepad.pose.targetRay = {
        transformMatrix: new Float32Array(16),
      };
      gamepad.pose._localPointerMatrix = new Float32Array(16);
    }
    this.isPresenting = false;
    this.stageParameters = new VRStageParameters();

    this.onrequestanimationframe = fn => window.requestAnimationFrame(fn);
    this.onvrdisplaypresentchange = () => {
      setTimeout(() => {
        const e = new Event('vrdisplaypresentchange');
        e.display = this;
        window.dispatchEvent(e);
      });
    };

    this._onends = [];
    this._lastPresseds = [false, false];

    // this._frameData = new VRFrameData();
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

  /* setSize(width, height) {
    GlobalContext.xrState.renderWidth[0] = width;
    GlobalContext.xrState.renderHeight[0] = height;
  } *?

  /* setProjection(projectionMatrix) {
    GlobalContext.xrState.leftProjectionMatrix.set(projectionMatrix);
    GlobalContext.xrState.rightProjectionMatrix.set(projectionMatrix);
  } */

  async requestPresent(layers) {
    GlobalContext.xrState.renderWidth[0] = this.window.innerWidth / 2;
    GlobalContext.xrState.renderHeight[0] = this.window.innerHeight;

    await this.onrequestpresent();
    
    const [{source: canvas}] = layers;
    const {_context: context} = canvas;
    this.onmakeswapchain(context);
    
    const [fbo, msFbo, msTex, msDepthTex] = nativeWindow.createVrChildRenderTarget(context, xrState.renderWidth[0]*2, xrState.renderHeight[0]);
    context.setDefaultFramebuffer(msFbo);
    nativeWindow.bindVrChildFbo(context, fbo, xrState.tex[0], xrState.depthTex[0]);

    if (this.onvrdisplaypresentchange && !this.isPresenting) {
      this.isPresenting = true;
      this.onvrdisplaypresentchange();
    } else {
      this.isPresenting = true;
    }
  }

  exitPresent() {
    return Promise.resolve().then(() => {
      if (this.onvrdisplaypresentchange && this.isPresenting) {
        this.isPresenting = false;
        this.onvrdisplaypresentchange();
      } else {
        this.isPresenting = false;
      }
    });
  }

  async requestSession({exclusive = true} = {}) {
    const self = this;

    GlobalContext.xrState.renderWidth[0] = this.window.innerWidth / 2;
    GlobalContext.xrState.renderHeight[0] = this.window.innerHeight;

    await this.onrequestpresent();
    
    const session = {
      addEventListener(e, fn) {
        if (e === 'end') {
          self._onends.push(fn);
        }
      },
      device: self,
      baseLayer: null,
      // layers,
      _frame: null, // defer
      getInputSources() {
        return this.device.gamepads;
      },
      requestFrameOfReference() {
        return Promise.resolve({});
      },
      requestAnimationFrame(fn) {
        return this.device.onrequestanimationframe(timestamp => {
          fn(timestamp, this._frame);
        });
      },
      end() {
        for (let i = 0; i < self._onends.length; i++) {
          self._onends[i]();
        }
        return self.exitPresent();
      },
    };

    const {xrState} = GlobalContext;
    const _frame = {
      session,
      views: [{
        eye: 'left',
        projectionMatrix: xrState.leftProjectionMatrix,
        _viewport: {
          x: 0,
          y: 0,
          width: xrState.renderWidth[0],
          height: xrState.renderHeight[0],
        },
      }, {
        eye: 'right',
        projectionMatrix: xrState.rightProjectionMatrix,
        _viewport: {
          x: xrState.renderWidth[0],
          y: 0,
          width: xrState.renderWidth[0],
          height: xrState.renderHeight[0],
        },
      }],
      _pose: null, // defer
      getDevicePose() {
        return this._pose;
      },
      getInputPose(inputSource, coordinateSystem) {
        localMatrix.fromArray(inputSource.pose._localPointerMatrix);

        const {xrOffset} = self.window.document;
        if (xrOffset) {
          localMatrix
            .premultiply(
              localMatrix2.compose(
                localVector.fromArray(xrOffset.position),
                localQuaternion.fromArray(xrOffset.orientation),
                localVector2.fromArray(xrOffset.scale)
              )
              .getInverse(localMatrix2)
            );
        }

        localMatrix.toArray(inputSource.pose.targetRay.transformMatrix);

        return inputSource.pose; // XXX or _pose
      },
    };
    session._frame = _frame;
    const _pose = {
      frame: _frame,
      getViewMatrix(view) {
        const viewMatrix = view.eye === 'left' ? xrState.leftViewMatrix : xrState.rightViewMatrix;

        const {xrOffset} = self.window.document;
        if (xrOffset) {
          localMatrix
            .fromArray(viewMatrix)
            .multiply(
              localMatrix2.compose(
                localVector.fromArray(xrOffset.position),
                localQuaternion.fromArray(xrOffset.orientation),
                localVector2.fromArray(xrOffset.scale)
              )
            )
            .toArray(localViewMatrix);
        } else {
          localViewMatrix.set(viewMatrix);
        }
        return localViewMatrix;
      },
    };
    _frame._pose = _pose;

    return session;
  }

  supportsSession() {
    return Promise.resolve(null);
  }

  /* getFrameData(frameData) {
    frameData.copy(this._frameData);
  } */

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
    const result = super.getEyeParameters(eye);
    if (eye === 'right') {
      result.renderWidth = 0;
    }
    return result;
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
    for (let i = 0; i < this.gamepads.length; i++) {
      const gamepad = this.gamepads[i];
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

  update() {
    // emit gamepad events
    for (let i = 0; i < this.gamepads.length; i++) {
      const gamepad = this.gamepads[i];
      const pressed = gamepad.buttons[1].pressed;
      const lastPressed = this._lastPresseds[i];
      if (pressed && !lastPressed) {
        this.emit('selectstart', new GlobalContext.XRInputSourceEvent('selectstart', {
          frame: this._frame,
          inputSource: gamepad,
        }));
        this.emit('select', new GlobalContext.XRInputSourceEvent('select', {
          frame: this._frame,
          inputSource: gamepad,
        }));
      } else if (lastPressed && !pressed) {
        this.emit('selectend', new GlobalContext.XRInputSourceEvent('selectend', {
          frame: this._frame,
          inputSource: gamepad,
        }));
      }
      this._lastPresseds[i] = pressed;
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

const createVRDisplay = () => new FakeVRDisplay();

const controllerIDs = {
  openvr: 'OpenVR Gamepad',
  oculusMobile: 'Oculus Go',
  openvrTracker: 'Tracker'
  oculusGoLeft: 'Oculus Touch (Left)',
  oculusGoRight: 'Oculus Touch (Right)',
  oculusQuestLeft: 'Oculus Touch (Left)',
  oculusQuestRight: 'Oculus Touch (Right)',
};

function getControllerID(hmdType, hand) {
  return controllerIDs[hmdType] || controllerIDs[vrDisplay.id + hand.charAt(0).toUpperCase() + hand.slice(1)];
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

module.exports = {
  VRDisplay,
  FakeVRDisplay,
  VRFrameData,
  VRPose,
  VRStageParameters,
  Gamepad,
  GamepadButton,
  getHMDType,
  createVRDisplay,
  getGamepads
};
