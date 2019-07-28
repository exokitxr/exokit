const {EventEmitter} = require('events');
const {Event, EventTarget} = require('./Event');
const {getHMDType} = require('./VR');
const GlobalContext = require('./GlobalContext');
const THREE = require('../lib/three-min.js');
const {defaultCanvasSize} = require('./constants');
const symbols = require('./symbols');
const {maxNumTrackers} = require('./constants');
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
  supportsSession(mode) {
    return Promise.resolve(true);
  }
  supportsSessionMode(mode) { // non-standard
    return this.supportsSession(mode);
  }
  async requestSession({exclusive = false, outputContext = null, extensions = {}} = {}) {
    if (!this.session) {
      const hmdType = getHMDType();

      if (hmdType) {
        const session = this._window[symbols.mrDisplaysSymbol].xrSession;
        session.exclusive = exclusive;
        session.outputContext = outputContext;

        await session.onrequestpresent();
        session.isPresenting = true;
        session.once('end', () => {
          session.isPresenting = false;
          this.session = null;
        });
        this.session = session;
      } else {
        return Promise.reject(null);
      }
    }
    if (this.session) {
      if (extensions.meshing) {
        GlobalContext.xrState.meshing[0] = 1;
      }
      if (extensions.planeTracking) {
        GlobalContext.xrState.planeTracking[0] = 1;
      }
      if (extensions.handTracking) {
        GlobalContext.xrState.handTracking[0] = 1;
      }
      if (extensions.eyeTracking) {
        GlobalContext.xrState.eyeTracking[0] = 1;
      }
    }
    return Promise.resolve(this.session);
  }
  /* async requestDevice() {
    const hmdType = getHMDType();

    if (hmdType) {
      return new XRDevice(this);
    } else {
      return Promise.resolve(null);
    }
  } */
  get onvrdevicechange() {
    return _elementGetter(this, 'vrdevicechange');
  }
  set onvrdevicechange(onvrdevicechange) {
    _elementSetter(this, 'vrdevicechange', onvrdevicechange);
  }
};
module.exports.XR = XR;

/* class XRDevice { // non-standard
  constructor(xr) {
    this.xr = xr;
  }

  supportsSession(opts) {
    return this.xr.supportsSession(opts);
  }
  requestSession(opts) {
    return this.xr.requestSession();
  }
}
module.exports.XRDevice = XRDevice; */

class XRSession extends EventTarget {
  constructor({exclusive = false, outputContext = null} = {}, window) {
    super();

    this.exclusive = exclusive;
    this.outputContext = outputContext;
    this.window = window;

    this.environmentBlendMode = 'opaque';
    this.renderState = new XRRenderState();
    this.viewerSpace = new XRSpace();
    this.isPresenting = false; // non-standard

    this._frame = new XRFrame(this);
    this._referenceSpace = new XRReferenceSpace();
    this._gamepadInputSources = (() => {
      const result = Array(2 + maxNumTrackers);
      for (let i = 0; i < maxNumTrackers; i++) {
        let hand, targetRayMode;
        if (i === 0) {
          hand = 'left';
          targetRayMode = 'tracked-pointer';
        } else if (i === 1) {
          hand = 'right';
          targetRayMode = 'tracked-pointer';
        } else {
          hand = null;
          targetRayMode = 'tracker';
        }
        result[i] = new XRInputSource(hand, targetRayMode, GlobalContext.xrState.gamepads[i]);
      }
      return result;
    })();
    this._handInputSources = (() => {
      const result = [
        new XRInputSource('left', 'hand', GlobalContext.xrState.hands[0]),
        new XRInputSource('right', 'hand', GlobalContext.xrState.hands[1]),
      ];
      for (let i = 0; i < result.length; i++) {
        const inputSource = result[i];
        inputSource.wrist = inputSource._xrStateGamepad.wrist;
        inputSource.fingers = inputSource._xrStateGamepad.fingers;
      }
      return result;
    })();
    this._eyeInputSource = new XRInputSource('', 'gaze', GlobalContext.xrState.eye);
    this._inputSources = this._gamepadInputSources
      .concat(this._handInputSources)
      .concat(this._eyeInputSource);
    this._lastPresseds = [false, false];
    this._rafs = [];
    this._layers = [];

    this.onrequestpresent = null;
    this.onmakeswapchain = null;
    this.onexitpresent = null;
    this.onrequestanimationframe = null;
    this.oncancelanimationframe = null;
    this.onlayers = null;
  }
  requestReferenceSpace(type, options = {}) {
    // const {disableStageEmulation = false, stageEmulationHeight  = 0} = options;
    return Promise.resolve(this._referenceSpace);
  }
  /* requestFrameOfReference() { // non-standard
    return this.requestReferenceSpace.apply(this, arguments);
  } */
  getInputSources() {
    return this._inputSources.filter(inputSource => inputSource.connected);
  }
  requestAnimationFrame(fn) {
    if (this.onrequestanimationframe) {
      const animationFrame = this.onrequestanimationframe(timestamp => {
        this._rafs.splice(animationFrame, 1);
        fn(timestamp, this._frame);
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
  requestHitTest(origin, direction, coordinateSystem) {
    return new Promise((accept, reject) => {
      if (this.onrequesthittest)  {
        this.onrequesthittest(origin, direction, coordinateSystem)
          .then(accept)
          .catch(reject);
      } else {
        reject(new Error('api not supported'));
      }
    });
  }
  updateRenderState(newState) {
    this.renderState.update(newState);
  }
  get baseLayer() { // non-standard
    return this.renderState.baseLayer;
  }
  set baseLayer(baseLayer) {
    this.renderState.update({baseLayer});
  }
  async end() {
    await this.onexitpresent();
    this.emit('end');
  }
  update() {
    const inputSources = this.getInputSources();
    const gamepads = GlobalContext.getGamepads();

    for (let i = 0; i < inputSources.length; i++) {
      const inputSource = inputSources[i];
      const gamepad = gamepads[i];

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

class XRRenderState {
  constructor() {
    this._inlineVerticalFieldOfView = 90;
    this._baseLayer = null;
    this._outputContext = null;
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
  get inlineVerticalFieldOfView() {
    return this._inlineVerticalFieldOfView;
  }
  set inlineVerticalFieldOfView(inlineVerticalFieldOfView) {
    this._inlineVerticalFieldOfView = inlineVerticalFieldOfView;
  }
  get baseLayer() {
    return this._baseLayer;
  }
  set baseLayer(baseLayer) {
    this._baseLayer = baseLayer;
  }
  get outputContext() {
    return this._outputContext;
  }
  set outputContext(outputContext) {
    this._outputContext = outputContext;
  }
  
  update(newState) {
    for (const k in newState) {
      this[k] = newState[k];
    }
  }
};
module.exports.XRRenderState = XRRenderState;

class XRWebGLLayer {
  constructor(session, context, options = {}) {
    this.session = session;
    this.context = context;
    
    const {
      antialias = true,
      depth = false,
      stencil = false,
      alpha = true,
      framebufferScaleFactor = 1,
    } = options;
    this.antialias = antialias;
    this.depth = depth;
    this.stencil = stencil;
    this.alpha = alpha;

    const {fbo} = this.session.onmakeswapchain(context);
    
    this.framebuffer = {
      id: fbo,
    };
  }
  getViewport(view) {
    return view._viewport;
  }
  requestViewportScaling(viewportScaleFactor) {
    throw new Error('not implemented'); // XXX
  }
  
  get framebuffer() {
    return this.session._framebuffer;
  }
  set framebuffer(framebuffer) {}
  
  get framebufferWidth() {
    return xrState.renderWidth[0]*2;
  }
  set framebufferWidth(framebufferWidth) {}
  
  get framebufferHeight() {
    return xrState.renderHeight[0];
  }
  set framebufferHeight(framebufferHeight) {}
}
module.exports.XRWebGLLayer = XRWebGLLayer;

const _applyXrOffsetToPose = (pose, xrOffset, inverse, premultiply) => {
  if (xrOffset) {
    localMatrix
      .fromArray(pose._realViewMatrix);
    localMatrix2.fromArray(inverse ? xrOffset.matrixInverse : xrOffset.matrix);
    if (premultiply) {
      localMatrix.premultiply(localMatrix2);
    } else {
      localMatrix.multiply(localMatrix2);
    }
    localMatrix
      .toArray(pose._localViewMatrix);
  } else {
    pose._localViewMatrix.set(pose._realViewMatrix);
  }
};

class XRFrame {
  constructor(session) {
    this.session = session;

    this._viewerPose = new XRViewerPose(this);
  }
  getViewerPose(coordinateSystem) {
    const xrOffset = this.session.renderState.baseLayer && this.session.renderState.baseLayer.context.canvas.ownerDocument.xrOffset;
    for (let i = 0; i < this._viewerPose.views.length; i++) {
      _applyXrOffsetToPose(this._viewerPose.views[i], xrOffset, false, false);
    }

    return this._viewerPose;
  }
  /* getDevicePose() { // non-standard
    return this.getViewerPose.apply(this, arguments);
  } */
  getPose(sourceSpace, destinationSpace) {
    _applyXrOffsetToPose(sourceSpace._pose, this.session.renderState.baseLayer && this.session.renderState.baseLayer.context.canvas.ownerDocument.xrOffset, true, true);

    return sourceSpace._pose;
  }
  getInputPose(inputSource, coordinateSystem) { // non-standard
    _applyXrOffsetToPose(inputSource._inputPose, this.session.renderState.baseLayer && this.session.renderState.baseLayer.context.canvas.ownerDocument.xrOffset, true, true);
    inputSource._inputPose.targetRay.transformMatrix.set(inputSource._inputPose._localViewMatrix);
    inputSource._inputPose.gripTransform.matrix.set(inputSource._inputPose._localViewMatrix);

    return inputSource._inputPose;
  }
}
module.exports.XRFrame = XRFrame;

class XRView {
  constructor(eye = 'left') {
    this.eye = eye;
    this.transform = new XRRigidTransform(eye);
    this.projectionMatrix = eye === 'left' ? GlobalContext.xrState.leftProjectionMatrix : GlobalContext.xrState.rightProjectionMatrix;

    this._viewport = new XRViewport(eye);
    this._realViewMatrix = this.transform.inverse.matrix;
    this._localViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
    this.transform.inverse.matrix = this._localViewMatrix;

    this.viewMatrix = this.transform.inverse.matrix; // non-standard
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
}
module.exports.XRViewport = XRViewport;

class XRPose {
  constructor() {
    this.transform = new XRRigidTransform();
    this.emulatedPosition = false;

    this._realViewMatrix = this.transform.inverse.matrix;
    this._localViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
    this.transform.inverse.matrix = this._localViewMatrix;
  }
}
module.exports.XRPose = XRPose;

class XRViewerPose extends XRPose {
  constructor(frame) {
    super();

    this.frame = frame; // non-standard

    this._views = [
      new XRView('left'),
      new XRView('right'),
    ];

    this.poseModelMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]); // non-standard
  }
  get views() {
    return this._views;
  }
  set views(views) {}
  getViewMatrix(view) { // non-standard
    if (this.frame.session.renderState.baseLayer && this.frame.session.renderState.baseLayer.context.canvas.ownerDocument.xrOffset) {
      const {xrOffset} = this.frame.session.renderState.baseLayer.context.canvas.ownerDocument;

      localMatrix
        .fromArray(view._realViewMatrix)
        .multiply(
          localMatrix2.fromArray(xrOffset.matrix)
        )
        .toArray(view._localViewMatrix);
    } else {
      view._localViewMatrix.set(view._realViewMatrix);
    }
    return view._localViewMatrix;
  }
}
module.exports.XRViewerPose = XRViewerPose;

class XRInputSource {
  constructor(handedness, targetRayMode, xrStateGamepad) {
    this.handedness = handedness;
    this.targetRayMode = targetRayMode;
    this._xrStateGamepad = xrStateGamepad;

    this.targetRaySpace = new XRSpace();
    this.targetRaySpace._pose._realViewMatrix = xrStateGamepad.transformMatrix;
    this.targetRaySpace._pose._localViewMatrix = this.targetRaySpace._pose.transform.inverse.matrix;
    this.gripSpace = new XRSpace();
    this.gripSpace._pose._realViewMatrix = xrStateGamepad.transformMatrix;
    this.gripSpace._pose._localViewMatrix = this.targetRaySpace._pose.transform.inverse.matrix;

    this._inputPose = new XRInputPose();
    this._inputPose.targetRay.origin.values = xrStateGamepad.position;
    this._inputPose.targetRay.direction.values = xrStateGamepad.direction;
    this._inputPose._realViewMatrix = xrStateGamepad.transformMatrix;
    this._inputPose._localViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
  get connected() {
    return this._xrStateGamepad.connected[0] !== 0;
  }
  set connected(connected) {
    this._xrStateGamepad.connected[0] = connected ? 1 : 0;
  }
}
module.exports.XRInputSource = XRInputSource;

class XRRay { // non-standard
  constructor() {
    this.origin = new GlobalContext.DOMPoint();
    this.direction = new GlobalContext.DOMPoint(0, 0, -1);
    this.transformMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
}
module.exports.XRRay = XRRay;

class XRInputPose { // non-standard
  constructor() {
    this.targetRay = new XRRay();
    this.gripTransform = new XRRigidTransform();
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
  constructor(position, orientation, scale) {
    if (position instanceof SharedArrayBuffer) {
      const inverse = orientation instanceof XRRigidTransform ? orientation : null;

      this.initialize(position, inverse);
    } else if (typeof position === 'string') {
      const eye = position;

      const result = new XRRigidTransform();
      result.inverse.matrix = eye === 'left' ? GlobalContext.xrState.leftViewMatrix : GlobalContext.xrState.rightViewMatrix; // XXX share all other XRRigidTransform properties
      return result;
    } else {
      this.initialize();

      if (!position) {
        position = {x: 0, y: 0, z: 0};
      }
      if (!orientation) {
        orientation = {x: 0, y: 0, z: 0, w: 1};
      }
      if (!scale) {
        scale = {x: 1, y: 1, z: 1};
      }

      this.position[0] = position.x;
      this.position[1] = position.y;
      this.position[2] = position.z;

      this.orientation[0] = orientation.x;
      this.orientation[1] = orientation.y;
      this.orientation[2] = orientation.z;
      this.orientation[3] = orientation.w;

      this.scale[0] = scale.x;
      this.scale[1] = scale.y;
      this.scale[2] = scale.z;

      localMatrix
        .compose(localVector.fromArray(this.position), localQuaternion.fromArray(this.orientation), localVector2.fromArray(this.scale))
        .toArray(this.matrix);
      localMatrix
        .getInverse(localMatrix)
        .toArray(this.matrixInverse);
      localMatrix
        .decompose(localVector, localQuaternion, localVector2);
      localVector.toArray(this.positionInverse);
      localQuaternion.toArray(this.orientationInverse);
      localVector2.toArray(this.scaleInverse);
    }

    if (!this._inverse) {
      this._inverse = new XRRigidTransform(this._buffer, this);
    }
  }
  
  initialize(_buffer = new SharedArrayBuffer((3 + 4 + 3 + 16) * 2 * Float32Array.BYTES_PER_ELEMENT), inverse = null) {
    this._buffer = _buffer;
    this._inverse = inverse;

    {
      let index = this._inverse ? ((3 + 4 + 3 + 16) * Float32Array.BYTES_PER_ELEMENT) : 0;

      this.position = new Float32Array(this._buffer, index, 3);
      index += 3 * Float32Array.BYTES_PER_ELEMENT;

      this.orientation = new Float32Array(this._buffer, index, 4);
      index += 4 * Float32Array.BYTES_PER_ELEMENT;

      this.scale = new Float32Array(this._buffer, index, 3);
      index += 3 * Float32Array.BYTES_PER_ELEMENT;

      this.matrix = new Float32Array(this._buffer, index, 16);
      index += 16 * Float32Array.BYTES_PER_ELEMENT;
    }
    {
      let index = this._inverse ? 0 : ((3 + 4 + 3 + 16) * Float32Array.BYTES_PER_ELEMENT);

      this.positionInverse = new Float32Array(this._buffer, index, 3);
      index += 3 * Float32Array.BYTES_PER_ELEMENT;

      this.orientationInverse = new Float32Array(this._buffer, index, 4);
      index += 4 * Float32Array.BYTES_PER_ELEMENT;

      this.scaleInverse = new Float32Array(this._buffer, index, 3);
      index += 3 * Float32Array.BYTES_PER_ELEMENT;

      this.matrixInverse = new Float32Array(this._buffer, index, 16);
      index += 16 * Float32Array.BYTES_PER_ELEMENT;
    }
  }
  
  get inverse() {
    return this._inverse;
  }
  set inverse(inverse) {}

  pushUpdate() {
    localMatrix
      .compose(
        localVector.fromArray(this.position),
        localQuaternion.fromArray(this.orientation),
        localVector2.fromArray(this.scale)
      )
      .toArray(this.matrix);
    localMatrix
      .getInverse(localMatrix)
      .toArray(this.matrixInverse);
    localMatrix
      .decompose(localVector, localQuaternion, localVector2);
    localVector.toArray(this.positionInverse);
    localQuaternion.toArray(this.orientationInverse);
    localVector2.toArray(this.scaleInverse);
  }
}
module.exports.XRRigidTransform = XRRigidTransform;

class XRSpace extends EventTarget {
  constructor() {
    super();
    
    this._pose = new XRPose();
  }
}
module.exports.XRSpace = XRSpace;

class XRReferenceSpace extends XRSpace {
  getOffsetReferenceSpace(originOffset) {
    return this; // XXX do the offsetting
  }
  get onreset() {
    return _elementGetter(this, 'reset');
  }
  set onreset(onreset) {
    _elementSetter(this, 'reset', onreset);
  }
}
module.exports.XRReferenceSpace = XRReferenceSpace;

class XRBoundedReferenceSpace extends XRReferenceSpace {
  constructor() {
    super();

    this.boundsGeometry = [
      new GlobalContext.DOMPoint(-3, -3),
      new GlobalContext.DOMPoint(3, -3),
      new GlobalContext.DOMPoint(3, 3),
      new GlobalContext.DOMPoint(-3, 3),
    ];
    this.emulatedHeight = 0;
  }
}
module.exports.XRBoundedReferenceSpace = XRBoundedReferenceSpace;
