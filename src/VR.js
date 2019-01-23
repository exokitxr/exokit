const {Event} = require('./Event');
const THREE = require('../lib/three-min.js');
const {defaultCanvasSize, defaultEyeSeparation} = require('./constants.js');
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
    return this._value[0] !== 0;
  }
  set value(value) {
    this._value[0] = value ? 1 : 0;
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
  constructor(hand, index) {
    this.id = 'OpenVR Gamepad';
    this.hand = hand;
    this.index = index;

    this.connected = false;
    this.mapping = 'standard';
    this.buttons = Array(16);
    for (let i = 0; i < this.buttons.length; i++) {
      this.buttons[i] = new GamepadButton();
    }
    this.pose = new GamepadPose();
    this.axes = new Float32Array(10);
    this.hapticActuators = [new GamepadHapticActuator(index)];
  }

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

class MRDisplay {
  constructor(displayName) {
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
    this.onexitpresent = null;
    this.onrequestanimationframe = null;
    this.onvrdisplaypresentchange = null;

    this._rafs = [];
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

  requestAnimationFrame(fn) {
    if (this.onrequestanimationframe) {
      const animationFrame = this.onrequestanimationframe(timestamp => {
        this._rafs.splice(animationFrame, 1);
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

  /* clone() {
    const o = new this.constructor();
    for (const k in this) {
      o[k] = this[k];
    }
    if (o.session) {
      o.session = o.session.clone();
      o.session.device = o;
    }
    return o;
  } */

  destroy() {
    for (let i = 0; i < this._rafs.length; i++) {
      this.cancelAnimationFrame(this._rafs[i]);
    }
  }
}
class VRDisplay extends MRDisplay {
  constructor(displayName = 'VR') {
    super(displayName);

    this._frameData = new VRFrameData();
  }

  getFrameData(frameData) {
    frameData.leftProjectionMatrix.set(GlobalContext.xrState.leftProjectionMatrix);
    frameData.leftViewMatrix.set(GlobalContext.xrState.leftViewMatrix);
    frameData.rightViewMatrix.set(GlobalContext.xrState.rightViewMatrix);
    frameData.rightProjectionMatrix.set(GlobalContext.xrState.rightProjectionMatrix);
    frameData.pose.position.set(GlobalContext.xrState.position);
    frameData.pose.orientation.set(GlobalContext.xrState.orientation);
  }

  /* update(update) {
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
    if (stageParameters !== undefined) {
      this.stageParameters.copy(stageParameters);
    }
  } */
}

class FakeVRDisplay extends MRDisplay {
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
      const {pose} = gamepad;
      if (!pose.targetRay) {
        pose.targetRay = {
          transformMatrix: new Float32Array(16),
        };
      }
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

    this._layers = [];
    this._onends = [];

    // this._frameData = new VRFrameData();
  }

  get depthNear() {
    return GlobalContext.xrState.depthNear[0];
  }
  set depthNear(depthNear) {}
  get depthFar() {
    return GlobalContext.xrState.depthFar[0];
  }
  set depthFar(depthFar) {}

  getState() {
    return GlobalContext.xrState;
  }

  setSize(width, height) {
    GlobalContext.xrState.renderWidth[0] = width;
    GlobalContext.xrState.renderHeight[0] = height;
  }
  
  setProjection(projectionMatrix) {
    GlobalContext.xrState.leftProjectionMatrix.set(projectionMatrix);
    GlobalContext.xrState.rightProjectionMatrix.set(projectionMatrix);
  }

  requestPresent() {
    this.isPresenting = true;

    if (this.onvrdisplaypresentchange) {
      this.onvrdisplaypresentchange();
    }

    return Promise.resolve();
  }

  exitPresent() {
    this.isPresenting = false;

    if (this.onvrdisplaypresentchange) {
      this.onvrdisplaypresentchange();
    }

    return Promise.resolve();
  }
  
  requestSession({exclusive = true} = {}) {
    const self = this;

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
      /* clone() {
        const o = new this.constructor();
        for (const k in this) {
          o[k] = this[k];
        }
        o._frame = o._frame.clone();
        o._frame.session = o;
        return o;
      }, */
    };

    const xrState = this.getState();
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
      /* clone() {
        const o = new this.constructor();
        for (const k in this) {
          o[k] = this[k];
        }
        o._pose = o._pose.clone();
        o._pose.frame = o;
        return o;
      }, */
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
      /* clone() {
        const o = new this.constructor();
        for (const k in this) {
          o[k] = this[k];
        }
        return o;
      }, */
    };
    _frame._pose = _pose;

    return Promise.resolve(session);
  }
  
  get layers() {
    return this._layers;
  }
  set layers(layers) {
    this._layers = layers;

    if (this.onlayers) {
      this.onlayers(layers);
    }
  }

  /* getFrameData(frameData) {
    frameData.copy(this._frameData);
  } */

  getLayers() {
    if (!this._stereo) {
      return [
        {
          leftBounds: [0, 0, 1, 1],
          rightBounds: [1, 0, 1, 1],
          source: null,
        }
      ];
    } else {
      return [
        {
          leftBounds: [0, 0, 0.5, 1],
          rightBounds: [0.5, 0, 1, 1],
          source: null,
        }
      ];
    }
  }
  
  getEyeParameters(eye) {
    const result = super.getEyeParameters(eye);
    if (eye === 'right') {
      result.renderWidth = 0;
    }
    return result;
  }
  
  waitGetPoses() {
    // update hmd
    this.position.toArray(GlobalContext.xrState.position);
    this.quaternion.toArray(GlobalContext.xrState.orientation);
    
    localMatrix.compose(
      localVector.copy(this.position)
        .add(localVector2.set(-0.1, 0, 0).applyQuaternion(this.quaternion)),
      this.quaternion,
      localVector2.set(1, 1, 1)
    )
     .getInverse(localMatrix)
     .toArray(GlobalContext.xrState.leftViewMatrix);
     
    localMatrix.compose(
      localVector.copy(this.position)
        .add(localVector2.set(0.1, 0, 0).applyQuaternion(this.quaternion)),
      this.quaternion,
      localVector2.set(1, 1, 1)
    )
     .getInverse(localMatrix)
     .toArray(GlobalContext.xrState.rightViewMatrix);

    for (let i = 0; i < this.gamepads.length; i++) {
      const gamepad = this.gamepads[i];
      localVector.copy(this.position)
        .add(
          localVector2.set(-0.3 + i*0.6, -0.3, 0)
            .applyQuaternion(this.quaternion)
        ).toArray(gamepad.pose.position);
      this.quaternion.toArray(gamepad.pose.orientation);

      if (!gamepad.handedness) {
        gamepad.handedness = gamepad.hand;
      }
      if (!gamepad.pose._localPointerMatrix) {
        gamepad.pose._localPointerMatrix = new Float32Array(32);
      }
      if (!gamepad.pose.targetRay) {
        gamepad.pose.targetRay = {
          transformMatrix: new Float32Array(32),
        };
      }

      localMatrix2
        .compose(
          localVector.fromArray(gamepad.pose.position),
          localQuaternion.fromArray(gamepad.pose.orientation),
          localVector2.set(1, 1, 1)
        )
        .toArray(gamepad.pose._localPointerMatrix);
    }
  }
}

const createVRDisplay = () => new FakeVRDisplay();

const getGamepads = (() => {
  let gamepads = null;
  
  return () => {
    if (!gamepads) {
      gamepads = [
        new Gamepad('left', 0),
        new Gamepad('right', 1),
      ];
    }
    return gamepads;
  };
})();
GlobalContext.getGamepads = getGamepads;

module.exports = {
  MRDisplay,
  VRDisplay,
  FakeVRDisplay,
  VRFrameData,
  VRPose,
  VRStageParameters,
  Gamepad,
  GamepadButton,
  createVRDisplay,
  getGamepads,
};
