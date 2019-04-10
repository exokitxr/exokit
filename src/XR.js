const {EventEmitter} = require('events');
const {Event, EventTarget} = require('./Event');
const {defaultCanvasSize} = require('./constants');
const GlobalContext = require('./GlobalContext');
const THREE = require('../lib/three-min.js');
const symbols = require('./symbols');
const {_elementGetter, _elementSetter} = require('./utils');

const localVector = new THREE.Vector3();
const localVector2 = new THREE.Vector3();
const localVector3 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();

class XR extends EventEmitter {
  constructor(window) {
    super();

    this._window = window;
  }
  requestDevice(name = null) {
    if (GlobalContext.fakeVrDisplayEnabled) {
      return Promise.resolve(this._window[symbols.mrDisplaysSymbol].fakeVrDisplay);
    } else if ((name === 'VR' || name === null) && GlobalContext.nativeVr && GlobalContext.nativeVr.VR_IsHmdPresent()) {
      return Promise.resolve(this._window[symbols.mrDisplaysSymbol].xrDisplay);
    } else if ((name === 'VR' || name === null) && GlobalContext.nativeOculusMobileVr && GlobalContext.nativeOculusMobileVr.OculusMobile_IsHmdPresent()) {
      return Promise.resolve(this._window[symbols.mrDisplaysSymbol].oculusMobileVrDevice);
    } else if ((name === 'AR' || name === null) && GlobalContext.nativeMl && GlobalContext.nativeMl.IsPresent()) {
      return Promise.resolve(this._window[symbols.mrDisplaysSymbol].xmDisplay);
    } else {
      return Promise.resolve(null);
    }
  }
  get onvrdevicechange() {
    return _elementGetter(this, 'vrdevicechange');
  }
  set onvrdevicechange(onvrdevicechange) {
    _elementSetter(this, 'vrdevicechange', onvrdevicechange);
  }
};
module.exports.XR = XR;

class XRDevice {
  constructor(name = 'VR') {
    this.name = name; // non-standard
    this.session = null; // non-standard
    
    this._layers = [];
  }
  supportsSession() {
    return Promise.resolve(null);
  }
  requestSession({exclusive = false, outputContext = null} = {}) {
    if (!this.session) {
      const session = new XRSession({
        device: this,
        exclusive,
        outputContext,
      });
      session.once('end', () => {
        this.session = null;
      });
      this.session = session;
    }
    return Promise.resolve(this.session);
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
}
module.exports.XRDevice = XRDevice;

class XRSession extends EventTarget {
  constructor({device = null, exclusive = false, outputContext = null} = {}) {
    super();

    this.device = device;
    this.exclusive = exclusive;
    this.outputContext = outputContext;

    this.baseLayer = null;

    this._frame = new XRPresentationFrame(this);
    this._frameOfReference = new XRFrameOfReference();
    this._inputSources = [
      new XRInputSource('left', 'hand'),
      new XRInputSource('right', 'hand'),
    ];
    this._lastPresseds = [false, false];
    this._rafs = [];
  }
  get depthNear() {
    return GlobalContext.xrState.depthNear[0];
  }
  set depthNear(depthNear) {}
  get depthFar() {
    return GlobalContext.xrState.depthFar[0];
  }
  set depthFar(depthFar) {}
  get layers() {
    return this.device.layers;
  }
  set layers(layers) {
    this.device.layers = layers;
  }
  requestFrameOfReference(type, options = {}) {
    // const {disableStageEmulation = false, stageEmulationHeight  = 0} = options;
    return Promise.resolve(this._frameOfReference);
  }
  getInputSources() {
    return this._inputSources.filter(inputSource => inputSource.connected);
  }
  requestAnimationFrame(fn) {
    if (this.device.onrequestanimationframe) {
      const animationFrame = this.device.onrequestanimationframe(timestamp => {
        this._rafs.splice(animationFrame, 1);
        fn(timestamp, this._frame);
      });
      this._rafs.push(animationFrame);
      return animationFrame;
    }
  }
  cancelAnimationFrame(animationFrame) {
    if (this.device.oncancelanimationframe) {
      const result = this.device.oncancelanimationframe(animationFrame);
      const index = this._rafs.indexOf(animationFrame);
      if (index !== -1) {
        this._rafs.splice(index, 1);
      }
      return result;
    }
  }
  requestHitTest(origin, direction, coordinateSystem) {
    return new Promise((accept, reject) => {
      if (this.device.onrequesthittest)  {
        this.device.onrequesthittest(origin, direction, result => {
          accept(result);
        });
      } else {
        reject(new Error('api not supported'));
      }
    });
  }
  end() {
    this.emit('end');
    return Promise.resolve();
  }
  update() {
    const gamepads = GlobalContext.getGamepads();
    
    for (let i = 0; i < gamepads.length; i++) {
      const gamepad = gamepads[i];
      const inputSource = this._inputSources[i];
      
      const pressed = gamepad.buttons[1].pressed;
      const lastPressed = this._lastPresseds[i];
      if (pressed && !lastPressed) {
        this.emit('selectstart', new XRInputSourceEvent('selectstart', {
          frame: this._frame,
          inputSource,
        }));
        this.emit('select', new XRInputSourceEvent('select', {
          frame: this._frame,
          inputSource,
        }));
      } else if (lastPressed && !pressed) {
        this.emit('selectend', new XRInputSourceEvent('selectend', {
          frame: this._frame,
          inputSource,
        }));
      }
      this._lastPresseds[i] = pressed;
    }
  }
  /* update(update) {
    const {
      depthNear,
      depthFar,
      renderWidth,
      renderHeight,
      frameData,
      // stageParameters,
      gamepads,
    } = update;

    if (depthNear !== undefined) {
      this.depthNear = depthNear;
    }
    if (depthFar !== undefined) {
      this.depthFar = depthFar;
    }
    if (renderWidth !== undefined && renderHeight !== undefined) {
      if (this.baseLayer) { // XXX potentially handle multiple layers
        const {context} = this.baseLayer;

        if (context.drawingBufferWidth !== renderWidth * 2) {
          context.canvas.width = renderWidth * 2;
        }
        if (context.drawingBufferHeight !== renderHeight) {
          context.canvas.height = renderHeight;
        }
      }

      for (let i = 0; i < this._frame.views.length; i++) {
        this._frame.views[i]._viewport.set(i * renderWidth, 0, renderWidth, renderHeight);
      }
    }
    if (frameData !== undefined) {
      if (this._frame.views[0]) {
        this._frame.views[0].projectionMatrix.set(frameData.leftProjectionMatrix);
        this._frame.views[0]._viewMatrix.set(frameData.leftViewMatrix);
      }
      if (this._frame.views[1]) {
        this._frame.views[1].projectionMatrix.set(frameData.rightProjectionMatrix);
        this._frame.views[1]._viewMatrix.set(frameData.rightViewMatrix);
      }
    }
    // if (stageParameters !== undefined) {
      // this._frameOfReference.emulatedHeight = stageParameters.position.y; // XXX
    // }
    if (gamepads !== undefined) {
      const scale = localVector3.set(1, 1, 1);

      for (let i = 0; i < 2; i++) {
        const gamepad = gamepads[i];
        if (gamepad) {
          const inputSource = this._inputSources[i];
          inputSource._connected = gamepad.connected;
          const position = localVector.fromArray(gamepad.pose.position); // XXX bind position/orientation
          const quaternion = localQuaternion.fromArray(gamepad.pose.orientation);
          const direction = localVector2.set(0, 0, -1).applyQuaternion(quaternion);
          const inputMatrix = localMatrix.compose(position, quaternion, scale);
          inputSource._pose.targetRay.origin.x = position.x;
          inputSource._pose.targetRay.origin.y = position.y;
          inputSource._pose.targetRay.origin.z = position.z;
          inputSource._pose.targetRay.direction.x = direction.x;
          inputSource._pose.targetRay.direction.y = direction.y;
          inputSource._pose.targetRay.direction.z = direction.z;
          inputMatrix.toArray(inputSource._pose.targetRay.transformMatrix);
          inputMatrix.toArray(inputSource._pose._localPointerMatrix);
          inputMatrix.toArray(inputSource._pose.gripMatrix);

          const pressed = gamepad.buttons[1].pressed;
          const lastPressed = this._lastPresseds[i];
          if (pressed && !lastPressed) {
            this.emit('selectstart', new XRInputSourceEvent('selectstart', {
              frame: this._frame,
              inputSource,
            }));
            this.emit('select', new XRInputSourceEvent('select', {
              frame: this._frame,
              inputSource,
            }));
          } else if (lastPressed && !pressed) {
            this.emit('selectend', new XRInputSourceEvent('selectend', {
              frame: this._frame,
              inputSource,
            }));
          }
          this._lastPresseds[i] = pressed;
        }
      }
    }
  } */
  get onblur() {
    return _elementGetter(this, 'blur');
  }
  set onblur(onblur) {
    _elementSetter(this, 'blur', onblur);
  }
  get onfocus() {
    return _elementGetter(this, 'focus');
  }
  set onfocus(onfocus) {
    _elementSetter(this, 'focus', onfocus);
  }
  get onresetpose() {
    return _elementGetter(this, 'resetpose');
  }
  set onresetpose(onresetpose) {
    _elementSetter(this, 'resetpose', onresetpose);
  }
  get onend() {
    return _elementGetter(this, 'end');
  }
  set onend(onend) {
    _elementSetter(this, 'end', onend);
  }
  get onselect() {
    return _elementGetter(this, 'select');
  }
  set onselect(onselect) {
    _elementSetter(this, 'select', onselect);
  }
  get onselectstart() {
    return _elementGetter(this, 'selectstart');
  }
  set onselectstart(onselectstart) {
    _elementSetter(this, 'selectstart', onselectstart);
  }
  get onselectend() {
    return _elementGetter(this, 'selectend');
  }
  set onselectend(onselectend) {
    _elementSetter(this, 'selectend', onselectend);
  }
}
module.exports.XRSession = XRSession;

class XRWebGLLayer {
  constructor(session, context, options = {}) {
    const {
      antialias = true,
      depth = false,
      stencil = false,
      alpha = true,
      multiview = false,
      framebufferScaleFactor = 1,
    } = options;

    this.context = context;

    this.antialias = antialias;
    this.depth = depth;
    this.stencil = stencil;
    this.alpha = alpha;
    this.multiview = multiview;

    const presentSpec = session.device.onrequestpresent ?
      session.device.onrequestpresent([{
        source: context.canvas,
      }])
    :
      {
        width: context.drawingBufferWidth,
        height: context.drawingBufferHeight,
        msFbo: null,
        msTex: 0,
        msDepthTex: 0,
        fbo: null,
        tex: 0,
        depthTex: 0,
      };
    const {width, height, msFbo} = presentSpec;

    this.framebuffer = msFbo !== null ? {
      id: msFbo,
    } : null;
    this.framebufferWidth = width;
    this.framebufferHeight = height;
  }
  getViewport(view) {
    return view._viewport;
  }
  requestViewportScaling(viewportScaleFactor) {
    throw new Error('not implemented'); // XXX
  }
}
module.exports.XRWebGLLayer = XRWebGLLayer;

class XRPresentationFrame {
  constructor(session) {
    this.session = session;
    this.views = [
      new XRView('left'),
      new XRView('right'),
    ];

    this._pose = new XRDevicePose(this);
  }
  getDevicePose(coordinateSystem) {
    return this._pose;
  }
  getInputPose(inputSource, coordinateSystem) {
    localMatrix.fromArray(inputSource._pose._localPointerMatrix);

    if (this.session.baseLayer) {
      const {xrOffset} = this.session.baseLayer.context.canvas.ownerDocument;
      
      if (xrOffset) {
        localMatrix
          .premultiply(
            localMatrix2.fromArray(xrOffset.matrixInverse)
          );
      }
    }

    localMatrix
      .toArray(inputSource._pose.targetRay.transformMatrix)
    localMatrix
      .toArray(inputSource._pose.gripMatrix);
    
    return inputSource._pose;
  }
}
module.exports.XRPresentationFrame = XRPresentationFrame;

class XRView {
  constructor(eye = 'left') {
    this.eye = eye;
    this.projectionMatrix = eye === 'left' ? GlobalContext.xrState.leftProjectionMatrix : GlobalContext.xrState.rightProjectionMatrix;

    this._viewport = new XRViewport(eye);
    this._viewMatrix = eye === 'left' ? GlobalContext.xrState.leftViewMatrix : GlobalContext.xrState.rightViewMatrix;
    this._localViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
}
module.exports.XRView = XRView;

class XRViewport {
  constructor(eye) {
    this.eye = eye;
  }
  get x() {
    return this.eye === 'left' ? 0 : GlobalContext.xrState.renderWidth[0];
  }
  set x(x) {}
  get y() {
    return 0;
  }
  set y(y) {}
  get width() {
    return GlobalContext.xrState.renderWidth[0];
  }
  set width(width) {}
  get height() {
    return GlobalContext.xrState.renderHeight[0];
  }
  set height(height) {}
  /* set(x, y, width, height) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
  } */
}
module.exports.XRViewport = XRViewport;

class XRDevicePose {
  constructor(frame) {
    this.frame = frame; // non-standard
    this.poseModelMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
  getViewMatrix(view) {
    if (this.frame && this.frame.session && this.frame.session.baseLayer && this.frame.session.baseLayer.context.canvas.ownerDocument.xrOffset) {
      const {xrOffset} = this.frame.session.baseLayer.context.canvas.ownerDocument;
      
      localMatrix
        .fromArray(view._viewMatrix)
        .multiply(
          localMatrix2.fromArray(xrOffset.matrix)
        )
        .toArray(view._localViewMatrix);
    } else {
      view._localViewMatrix.set(view._viewMatrix);
    }
    return view._localViewMatrix;
  }
}
module.exports.XRDevicePose = XRDevicePose;

class XRInputSource {
  constructor(handedness = 'left', pointerOrigin = 'hand') {
    this.handedness = handedness;
    this.pointerOrigin = pointerOrigin;

    this._pose = new XRInputPose();
    const gamepad = GlobalContext.xrState.gamepads[handedness === 'left' ? 0 : 1];
    this._pose.targetRay.origin.values = gamepad.position;
    this._pose.targetRay.direction.values = gamepad.direction;
    this._pose._localPointerMatrix = gamepad.transformMatrix;
  }
  get connected() {
    return GlobalContext.xrState.gamepads[this.handedness === 'left' ? 0 : 1].connected[0] !== 0;
  }
  set connected(connected) {}
}
module.exports.XRInputSource = XRInputSource;

class XRRay {
  constructor() {
    this.origin = new GlobalContext.DOMPoint();
    this.direction = new GlobalContext.DOMPoint(0, 0, -1);
    this.transformMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
}
module.exports.XRRay = XRRay;

class XRInputPose {
  constructor() {
    this.emulatedPosition = false;
    this.targetRay = new XRRay();
    this.gripMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
    this._localPointerMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
}
module.exports.XRInputPose = XRInputPose;

class XRInputSourceEvent extends Event {
  constructor(type, init = {}) {
    super(type);

    this.frame = init.frame !== undefined ? init.frame : null;
    this.inputSource = init.inputSource !== undefined ? init.inputSource : null;
  }
}
module.exports.XRInputSourceEvent = XRInputSourceEvent;
GlobalContext.XRInputSourceEvent = XRInputSourceEvent;

class XRRigidTransform {
  constructor(position = {x: 0, y: 0, z: 0}, orientation = {x: 0, y: 0, z: 0, w: 1}, scale = {x: 1, y: 1, z: 1}) {
    this.position = Float32Array.from([position.x, position.y, position.z]);
    this.orientation = Float32Array.from([orientation.x, orientation.y, orientation.z, orientation.w]);
    this.scale = Float32Array.from([scale.x, scale.y, scale.z]); // non-standard
    this.matrix = localMatrix
      .compose(localVector.fromArray(this.position), localQuaternion.fromArray(this.orientation), localVector2.fromArray(this.scale))
      .toArray(new Float32Array(16));
    this.matrixInverse = localMatrix
      .getInverse(localMatrix)
      .toArray(new Float32Array(16));
  }

  updateMatrix() {
    localMatrix
      .compose(
        localVector.fromArray(this.position),
        localQuaternion.fromArray(this.orientation),
        localVector2.fromArray(this.scale)
      )
      .toArray(this.matrix)
    localMatrix
      .getInverse(localMatrix)
      .toArray(this.matrixInverse);
  }
}
module.exports.XRRigidTransform = XRRigidTransform;

class XRCoordinateSystem {
  getTransformTo(other) {
    return Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]); // XXX
  }
}
module.exports.XRCoordinateSystem = XRCoordinateSystem;

class XRFrameOfReference extends XRCoordinateSystem {
  constructor() {
    super();

    this.bounds = new XRStageBounds();
    this.emulatedHeight = 0;
  }
  get onboundschange() {
    return _elementGetter(this, 'boundschange');
  }
  set onboundschange(onboundschange) {
    _elementSetter(this, 'boundschange', onboundschange);
  }
}
module.exports.XRFrameOfReference = XRFrameOfReference;

class XRStageBounds {
  constructor() {
    this.geometry = [
      new XRStageBoundsPoint(-3, -3),
      new XRStageBoundsPoint(3, -3),
      new XRStageBoundsPoint(3, 3),
      new XRStageBoundsPoint(-3, 3),
    ];
  }
}
module.exports.XRStageBounds = XRStageBounds;

class XRStageBoundsPoint {
  constructor(x, z) {
    this.x = x;
    this.z = z;
  }
}
module.exports.XRStageBoundsPoint = XRStageBoundsPoint;
