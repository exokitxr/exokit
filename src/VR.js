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
  constructor(id, hand, xrGamepad, hapticActuator) {
    this.id = id;
    this.hand = hand;
    this._xrGamepad = xrGamepad;

    this.mapping = 'standard';
    this.buttons = (() => {
      const result = Array(xrGamepad.buttons.length);
      for (let i = 0; i < result.length; i++) {
        result[i] = new GamepadButton(xrGamepad.buttons[i].value, xrGamepad.buttons[i].pressed, xrGamepad.buttons[i].touched);
      }
      return result;
    })();
    this.pose = new GamepadPose(xrGamepad.position, xrGamepad.orientation);
    this.axes = xrGamepad.axes;
    this.hapticActuators = hapticActuator ? [hapticActuator] : [];
    this.bones = xrGamepad.bones;
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
    const THREE = require('../lib/three.js'); // load full version

    for (let i = 0; i < this.meshes.length; i++) {
      const meshSpec = this.meshes[i];

      const geometry = new THREE.BufferGeometry();
      geometry.addAttribute('position', new THREE.BufferAttribute(meshSpec.positionArray, 3));
      geometry.setIndex(new THREE.BufferAttribute(meshSpec.indexArray, 1));
      const material = new THREE.MeshBasicMaterial({
        color: 0xFF0000,
      });
      const mesh = new THREE.Mesh(geometry, material);
      mesh.matrix.fromArray(meshSpec.transformMatrix);
      mesh.matrixWorld.copy(mesh.matrix);

      const raycaster = new THREE.Raycaster();
      raycaster.set(localVector.fromArray(origin), localVector2.fromArray(direction));
      const intersects = raycaster.intersectObject(mesh);
      if (intersects.length > 0) {
        const [intersect] = intersects;
        const {point} = intersect;
        return [{
          hitMatrix: localMatrix
            .compose(
              point,
              localQuaternion.set(0, 0, 0, 1),
              // localQuaternion.setFromUnitVectors(localVector2.set(0, 0, -1), normal),
              localVector2.set(1, 1, 1)
            )
            .toArray(new Float32Array(16)),
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

class FakeXRDisplay {
  constructor() {
    this.position = new THREE.Vector3();
    this.quaternion = new THREE.Quaternion();
  }

  pushHmdUpdate(position = this.position, quaternion = this.quaternion) {
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
  }
  pushGamepadsUpdate(position, quaternion) {
    if (!globalGamepads) {
      globalGamepads = _makeGlobalGamepads();
    }

    for (let i = 0; i < globalGamepads.main.length; i++) {
      const gamepad = globalGamepads.main[i];
      if (i === 1 && position && quaternion) {
        position.toArray(gamepad.pose.position);
        quaternion.toArray(gamepad.pose.orientation);
      } else {
        localVector.copy(this.position)
          .add(
            localVector2.set(-0.3 + i*0.6, -0.3, -0.35)
              .applyQuaternion(this.quaternion)
          ).toArray(gamepad.pose.position);
        this.quaternion.toArray(gamepad.pose.orientation); // XXX updates xrState
      }

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
  pushUpdate() {
    this.pushHmdUpdate();
    this.pushGamepadsUpdate();
  }

  enable() {
    GlobalContext.xrState.fakeVrDisplayEnabled[0] = 1;
  }
  disable() {
    GlobalContext.xrState.fakeVrDisplayEnabled[0] = 0;
  }

  get width() {
    return GlobalContext.xrState.renderWidth[0]*2;
  }
  set width(width) {
    GlobalContext.xrState.renderWidth[0] = width/2;
  }
  get height() {
    return GlobalContext.xrState.renderHeight[0];
  }
  set height(height) {
    GlobalContext.xrState.renderHeight[0] = height;
  }
  get projectionMatrix() {
    return GlobalContext.xrState.leftProjectionMatrix;
  }
  set projectionMatrix(projectionMatrix) {}
  get viewMatrix() {
    return GlobalContext.xrState.leftViewMatrix;
  }
  set viewMatrix(viewMatrix) {}
  get texture() {
    return {
      id: GlobalContext.xrState.tex[0],
    };
  }
  set texture(texture) {}
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
  openvrTracker: 'OpenVR Tracker',
  indexLeft: 'Valve Index (Left)',
  indexRight: 'Valve Index (Right)',
  oculusLeft: 'Oculus Touch (Left)',
  oculusRight: 'Oculus Touch (Right)',
  // oculusMobile: 'Oculus Go',
  oculusMobileLeft: 'Oculus Touch (Left)',
  oculusMobileRight: 'Oculus Touch (Right)',
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
    let hmdType = getHMDType();
    if (hmdType === 'openvr') {
      const vrSystem = nativeOpenVR.GetGlobalSystem();
      if (vrSystem && /^Knuckles/.test(vrSystem.GetModelName(0) || vrSystem.GetModelName(1))) {
        hmdType = 'index';
      }
    }

    if (!globalGamepads) {
      globalGamepads = _makeGlobalGamepads();
    }
    const gamepads = globalGamepads.main.slice();

    globalGamepads.main[0].id = getControllerID(hmdType, 'left');
    globalGamepads.main[1].id = getControllerID(hmdType, 'right');

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
function clearGamepads() {
  gamepads = null;
}
GlobalContext.clearGamepads = clearGamepads;

module.exports = {
  VRDisplay,
  VRFrameData,
  VRPose,
  VRStageParameters,
  Gamepad,
  GamepadButton,
  FakeXRDisplay,
  FakeMesher,
  FakePlaneTracker,
  getHMDType,
  lookupHMDTypeString,
  lookupHMDTypeIndex,
  createFakeXRDisplay,
  getGamepads
};
