const {EventEmitter} = require('events');
const {Event, EventTarget} = require('./Event');
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
const localVector3 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();
const localRay = new THREE.Ray();
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
  constructor(id, hand, xrGamepad, hapticActuator) {
    this.id = id;
    this.hand = hand;
    this._xrGamepad = xrGamepad;

    this.mapping = 'standard';
    this.buttons = (() => {
      const result = Array(5);
      for (let i = 0; i < result.length; i++) {
        result[i] = new GamepadButton(xrGamepad.buttons[i].value, xrGamepad.buttons[i].pressed, xrGamepad.buttons[i].touched);
      }
      return result;
    })();
    this.pose = new GamepadPose(xrGamepad.position, xrGamepad.orientation);
    this.axes = xrGamepad.axes;
    this.hapticActuators = hapticActuator ? [hapticActuator] : [];
  }

  get connected() {
    return this._xrGamepad.connected[0] !== 0;
  }
  set connected(connected) {
    this._xrGamepad.connected[0] = connected ? 1 : 0;
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

class VRDisplay extends EventEmitter {
  constructor(displayName, window) {
    super();

    this.displayName = displayName;
    this.window = window;

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

  async requestPresent(layers) {
    await this.onrequestpresent();
    
    const [{source: canvas}] = layers;
    const context = canvas._context || canvas.getContext('webgl');
    this.onmakeswapchain(context);

    if (this.onvrdisplaypresentchange && !this.isPresenting) {
      this.isPresenting = true;
      this.onvrdisplaypresentchange();
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
  get texture() {
    return {
      id: GlobalContext.xrState.tex[0],
    };
  }
  set texture(texture) {}
  get hidden() {
    return GlobalContext.xrState.hidden[0] != 0;
  }
  set hidden(hidden) {
    GlobalContext.xrState.hidden[0] = hidden ? 1 : 0;
  }

  destroy() {
    for (let i = 0; i < this._rafs.length; i++) {
      this.cancelAnimationFrame(this._rafs[i]);
    }
  }
}

class FakeMesher extends EventEmitter {
  constructor() {
    super();

    this.meshes = [];

    const boxBufferGeometry = {
      position: Float32Array.from([0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5]),
      normal: Float32Array.from([1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, -1, 0, 0, -1, 0, 0, -1, 0, 0, -1]),
      index: Uint16Array.from([0, 2, 1, 2, 3, 1, 4, 6, 5, 6, 7, 5, 8, 10, 9, 10, 11, 9, 12, 14, 13, 14, 15, 13, 16, 18, 17, 18, 19, 17, 20, 22, 21, 22, 23, 21]),
    };

    const lastMeshPosition = new THREE.Vector3();

    this.interval = setInterval(() => {
      localMatrix
        .fromArray(GlobalContext.xrState.leftViewMatrix)
        .getInverse(localMatrix)
        .decompose(localVector, localQuaternion, localVector2);
      const currentMeshPosition = new THREE.Vector3(Math.floor(localVector.x/10+0.5)*10, 0, Math.floor(localVector.z/10+0.5)*10);

      const updates = [];
      if (this.meshes.length > 0 && !currentMeshPosition.equals(lastMeshPosition)) {
        for (let i = 0; i < this.meshes.length; i++) {
          const mesh = this.meshes[i];
          updates.push({
            id: mesh.id,
            type: 'meshremove',
          });
        }
      }

      this.meshes = (this.meshes.length > 0 && currentMeshPosition.equals(lastMeshPosition)) ?
        this.meshes.map(({id}) => ({type: 'meshupdate', id, positionArray: null, normalArray: null, indexArray: null, transformMatrix: null}))
      :
        (() => {
          const result = Array(3);
          for (let i = 0; i < result.length; i++) {
            result[i] = {
              type: 'meshadd',
              id: Math.random() + '',
              positionArray: null,
              normalArray: null,
              indexArray: null,
              transformMatrix: null,
            };
          }
          return result;
        })();
      for (let i = 0; i < this.meshes.length; i++) {
        const mesh = this.meshes[i];

        localQuaternion.setFromUnitVectors(
          localVector.set(0, 1, 0),
          localVector2.set(-0.5+Math.random(), 1, -0.5+Math.random()).normalize()
        );
        localVector.set(0, 0.5+i, 0)
        localVector2.set(0.5+Math.random()*0.5, 1+Math.random(), 0.5+Math.random()*0.5);

        const box = new THREE.Box3().setFromCenterAndSize(localVector3.set(0, 0, 0), localVector2);
        box.quaternion = localQuaternion.clone();
        box.matrix = localMatrix
          .compose(
            localVector,
            localQuaternion,
            localVector3.set(1, 1, 1)
          )
          .clone();
        box.matrixInverse = localMatrix
          .getInverse(localMatrix)
          .clone();
        mesh.box = box;

        localMatrix.compose(
          localVector,
          localQuaternion,
          localVector2
        );

        const positionArray = new Float32Array(new SharedArrayBuffer(boxBufferGeometry.position.byteLength));
        positionArray.set(boxBufferGeometry.position);
        const normalArray = new Float32Array(new SharedArrayBuffer(boxBufferGeometry.normal.byteLength));
        normalArray.set(boxBufferGeometry.normal);
        for (let j = 0; j < positionArray.length; j += 3) {
          localVector.fromArray(positionArray, j);
          localVector.applyMatrix4(localMatrix);
          localVector.toArray(positionArray, j);

          localVector.fromArray(normalArray, j);
          localVector.applyMatrix4(localMatrix);
          localVector.toArray(normalArray, j);
        }

        const indexArray = new Uint16Array(new SharedArrayBuffer(boxBufferGeometry.index.byteLength));
        indexArray.set(boxBufferGeometry.index);

        mesh.positionArray = positionArray;
        mesh.normalArray = normalArray;
        mesh.indexArray = indexArray;
        mesh.transformMatrix = localMatrix
          .compose(
            localVector.set(currentMeshPosition.x, 0, currentMeshPosition.z),
            localQuaternion.set(0, 0, 0, 1),
            localVector2.set(1, 1, 1)
          )
          .toArray(new Float32Array(16));
      };
      updates.push.apply(updates, this.meshes);

      if (updates.length > 0) {
        this.emit('meshes', updates);
      }

      lastMeshPosition.copy(currentMeshPosition);
    }, 1000);
  }

  async requestHitTest(origin, direction, coordinateSystem) {
    for (let i = 0; i < this.meshes.length; i++) {
      const mesh = this.meshes[i];

      localVector.fromArray(origin).applyMatrix4(mesh.box.matrixInverse);
      localVector2.fromArray(origin).add(localVector3.fromArray(direction)).applyMatrix4(mesh.box.matrixInverse).sub(localVector);
      localRay.set(localVector, localVector2);
      const intersection = localRay.intersectBox(mesh.box, localVector3);
      if (intersection) {
        const normal = localVector;
        if (intersection.x >= mesh.box.max.x - 0.001) normal.set(1, 0, 0);
        else if (intersection.x <= mesh.box.min.x + 0.001) normal.set(-1, 0, 0);
        else if (intersection.y >= mesh.box.max.y - 0.001) normal.set(0, 1, 0);
        else if (intersection.y <= mesh.box.min.y + 0.001) normal.set(0, -1, 0);
        else if (intersection.z >= mesh.box.max.z - 0.001) normal.set(0, 0, 1);
        else if (intersection.z <= mesh.box.min.z + 0.001) normal.set(0, 0, -1);
        normal.applyQuaternion(mesh.box.quaternion);

        intersection.applyMatrix4(mesh.box.matrix);

        return [{
          hitMatrix: localMatrix
            .compose(
              intersection,
              localQuaternion.setFromUnitVectors(localVector2.set(0, 0, -1), normal),
              localVector2.set(1, 1, 1)
            )
            .toArray(new Float32Array(16))
        }];
      }
    }
    return [];
  }

  destroy() {
    clearInterval(this.interval);
  }
}

class FakePlaneTracker extends EventEmitter {
  constructor() {
    super();

    let planes = [];
    const lastMeshPosition = new THREE.Vector3();

    this.interval = setInterval(() => {
      localMatrix
        .fromArray(GlobalContext.xrState.leftViewMatrix)
        .getInverse(localMatrix)
        .decompose(localVector, localQuaternion, localVector2);
      const currentMeshPosition = new THREE.Vector3(Math.floor(localVector.x/10+0.5)*10, 0, Math.floor(localVector.z/10+0.5)*10);

      const updates = [];
      if (planes.length > 0 && !currentMeshPosition.equals(lastMeshPosition)) {
        for (let i = 0; i < planes.length; i++) {
          const plane = planes[i];
          updates.push({
            id: plane.id,
            type: 'planeremove',
          });
        }
      }

      if (planes.length === 0 || !currentMeshPosition.equals(lastMeshPosition)) {
        planes = Array(Math.floor(2 + Math.random()*5));

        for (let i = 0; i < planes.length; i++) {
          planes[i] = {
            id: Math.random() + '',
            type: 'planeadd',
            position: localVector.copy(currentMeshPosition)
              .add(localVector2.set(-5 + Math.random()*10, Math.random()*0.5, -5 + Math.random()*10))
              .toArray(new Float32Array(3)),
            normal: Float32Array.from([0, 1, 0]),
            scale: Float32Array.from([1, 1, 1]),
          };
        }
        updates.push.apply(updates, planes);
      }

      if (updates.length > 0) {
        this.emit('planes', updates);
      }

      lastMeshPosition.copy(currentMeshPosition);
    }, 1000);
  }

  destroy() {
    clearInterval(this.interval);
  }
}

class FakeVRDisplay extends VRDisplay {
  constructor(window) {
    super('FAKE');

    this.window = window;

    this.session = null;
    this.position = new THREE.Vector3();
    this.quaternion = new THREE.Quaternion();

    if (!globalGamepads) {
      globalGamepads = _makeGlobalGamepads();
    }
    const _decorateGamepad = (gamepad, targetRayMode) => {
      gamepad.handedness = gamepad.hand;
      gamepad.targetRayMode = targetRayMode;
      gamepad.pose.targetRay = {
        origin: new GlobalContext.DOMPoint(),
        direction: new GlobalContext.DOMPoint(0, 0, -1),
        transformMatrix: Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]),
      };
      gamepad.pose.targetRay.origin.values = gamepad._xrGamepad.position;
      gamepad.pose.targetRay.direction.values = gamepad._xrGamepad.direction;
      gamepad.pose._localPointerMatrix = gamepad._xrGamepad.transformMatrix;
    };
    for (let i = 0; i < globalGamepads.main.length; i++) {
      _decorateGamepad(globalGamepads.main[i], 'tracked-pointer');
    }
    for (let i = 0; i < globalGamepads.tracker.length; i++) {
      _decorateGamepad(globalGamepads.tracker[i], 'tracked-pointer');
    }
    for (let i = 0; i < globalGamepads.hand.length; i++) {
      _decorateGamepad(globalGamepads.hand[i], 'hand');
    }
    _decorateGamepad(globalGamepads.eye, 'gaze');

    for (let i = 0; i < globalGamepads.hand.length; i++) {
      const handGamepad = globalGamepads.hand[i];
      const hand = handGamepad._xrGamepad;

      /* handGamepad.wrist = hand.wrist;
      handGamepad.fingers = hand.fingers; */

      const offsetMatrix = new Float32Array(16);
      Object.defineProperty(handGamepad, 'offsetMatrix', {
        get: () => {
          localMatrix.compose(
            localVector.fromArray(hand.position),
            localQuaternion.fromArray(hand.orientation),
            localVector2.set(1, 1, 1)
          );

          const {xrOffset} = this.window.document;
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
          
          localMatrix.toArray(offsetMatrix);

          return offsetMatrix;
        },
      });
    }
    {
      const eye = globalGamepads.eye._xrGamepad;

      const offsetMatrix = new Float32Array(16);
      Object.defineProperty(globalGamepads.eye, 'offsetMatrix', {
        get: () => {
          /* localMatrix.compose(
            localVector.fromArray(eye.position),
            localQuaternion.fromArray(eye.orientation),
            localVector2.set(1, 1, 1)
          ); */

          localMatrix.compose(
            localVector.fromArray(GlobalContext.xrState.eye.position),
            localQuaternion.fromArray(eye.orientation),
            localVector2.set(1, 1, 1)
          );
          // localMatrix.identity();

          const {xrOffset} = this.window.document;
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
          
          localMatrix.toArray(offsetMatrix);

          return offsetMatrix;
        },
      });
    }

    this.onrequestanimationframe = fn => window.requestAnimationFrame(fn);
    this.onvrdisplaypresentchange = () => {
      setTimeout(() => {
        const e = new Event('vrdisplaypresentchange');
        e.display = this;
        window.dispatchEvent(e);
      });
    };

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
  } */

  /* setProjection(projectionMatrix) {
    GlobalContext.xrState.leftProjectionMatrix.set(projectionMatrix);
    GlobalContext.xrState.rightProjectionMatrix.set(projectionMatrix);
  } */

  async requestSession({exclusive = true, extensions = {}} = {}) {
    const self = this;

    const {xrState} = GlobalContext;

    if (extensions.meshing) {
      xrState.meshing[0] = 1;
    }
    if (extensions.planeTracking) {
      xrState.planeTracking[0] = 1;
    }
    if (extensions.handTracking) {
      xrState.handTracking[0] = 1;
    }
    if (extensions.eyeTracking) {
      xrState.eyeTracking[0] = 1;
    }

    await this.onrequestpresent();

    const session = {
      listeners: [],
      addEventListener(e, fn) {
        if (!this.listeners[e]) {
          this.listeners[e] = [];
        }
        this.listeners[e].push(fn);
      },
      removeEventListener(e, fn) {
        const listeners = this.listeners[e];
        if (listeners) {
          const index = listeners.indexOf(fn);
          if (index !== -1) {
            listeners.splice(index, 1);
          }
        }
      },
      dispatchEvent(e) {
        let listeners = this.listeners[e.type];
        if (listeners) {
          listeners = listeners.slice();
          for (let i = 0; i < listeners.length; i++) {
            listeners[i](e);
          }
        }
      },
      device: self,
      baseLayer: null,
      _frame: null, // defer
      getInputSources() {
        return getGamepads();
      },
      requestFrameOfReference() {
        return Promise.resolve({});
      },
      requestAnimationFrame(fn) {
        return self.requestAnimationFrame(timestamp => {
          fn(timestamp, this._frame);
        });
      },
      cancelAnimationFrame: fn => self.cancelAnimationFrame(fn),
      async end() {
        await self.exitPresent();

        xrState.fakeVrDisplayEnabled[0] = 1;
        self.session = null;

        const onends = self.listeners['end'].slice();
        for (let i = 0; i < onends.length; i++) {
          onends[i]();
        }
      },
      requestHitTest(origin, direction, coordinateSystem) {
        return self.window.runAsync({
          method: 'requestHitTest',
          origin,
          direction,
          coordinateSystem,
        });
        /* throw new Error('not implemented'); // XXX post this upwards
        if (!this._mesher) {
          self._mesher = new FakeMesher(session);
        }
        return self._mesher.requestHitTest(origin, direction, coordinateSystem); */
      },
    };
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

    this.session = session;

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
    for (let i = 0; i < globalGamepads.main.length; i++) {
      const gamepad = globalGamepads.main[i];
      localVector.copy(this.position)
        .add(
          localVector2.set(-0.3 + i*0.6, -0.3, -0.35)
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

      gamepad.connected = true;
    }
  }

  update() {
    const _updateGamepadEvents = () => {
      for (let i = 0; i < globalGamepads.main.length; i++) {
        const gamepad = globalGamepads.main[i];
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
    };
    _updateGamepadEvents();
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

let globalGamepads = null;
const _makeGlobalGamepads = () => ({
  main: [
    new Gamepad('gamepad', 'left', GlobalContext.xrState.gamepads[0], new GamepadHapticActuator(0)),
    new Gamepad('gamepad', 'right', GlobalContext.xrState.gamepads[1], new GamepadHapticActuator(1)),
  ],
  tracker: (() => {
    const result = Array(maxNumTrackers);
    for (let i = 0; i < result.length; i++) {
      result[i] = new Gamepad('tracker', '', GlobalContext.xrState.gamepads[2+i], null);
    }
    return result;
  })(),
  hand: (() => {
    const result = [
      new Gamepad('hand', 'left', GlobalContext.xrState.hands[0], null),
      new Gamepad('hand', 'right', GlobalContext.xrState.hands[1], null),
    ];
    for (let i = 0; i < result.length; i++) {
      const handGamepad = result[i];
      const hand = handGamepad._xrGamepad;

      handGamepad.wrist = hand.wrist;
      handGamepad.fingers = hand.fingers;
    }
    return result;
  })(),
  eye: new Gamepad('eye', '', GlobalContext.xrState.eye, null),
});

const controllerIDs = {
  fake: 'OpenVR Gamepad',
  openvr: 'OpenVR Gamepad',
  // oculusMobile: 'Oculus Go',
  // openvrTracker: 'Tracker',
  oculusGoLeft: 'Oculus Touch (Left)',
  oculusGoRight: 'Oculus Touch (Right)',
  oculusQuestLeft: 'Oculus Touch (Left)',
  oculusQuestRight: 'Oculus Touch (Right)',
};
function getControllerID(hmdType, hand) {
  return controllerIDs[hmdType] || controllerIDs[hmdType + hand.charAt(0).toUpperCase() + hand.slice(1)];
}
function getGamepads() {
  if (GlobalContext.xrState.isPresenting[0]) {
    const hmdType = getHMDType();
    if (!globalGamepads) {
      globalGamepads = _makeGlobalGamepads();
    }
    
    // XXX map oculusGo/oculusQuest
    // XXX can do these decorations at XR entry time
    globalGamepads.main[0].id = getControllerID(hmdType, 'left');
    globalGamepads.main[1].id = getControllerID(hmdType, 'right');

    const gamepads = globalGamepads.main.slice();

    if (hmdType === 'openvr') {
      for (let i = 0; i < globalGamepads.tracker.length; i++) {
        globalGamepads.tracker[i].id = getControllerID('openvr', 'tracker');
      }
      gamepads.push.apply(gamepads, globalGamepads.tracker);
    }
    
    if (GlobalContext.xrState.handTracking[0]) {
      gamepads.push.apply(gamepads, globalGamepads.hand);
    }
    if (GlobalContext.xrState.eyeTracking[0]) {
      gamepads.push(globalGamepads.eye);
    }
    
    return gamepads;
  } else {
    return [];
  }
};
GlobalContext.getGamepads = getGamepads;

module.exports = {
  VRDisplay,
  FakeMesher,
  FakePlaneTracker,
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
