const {EventEmitter} = require('events');
const {Event, EventTarget} = require('./Event');
const {defaultCanvasSize} = require('./constants');
const GlobalContext = require('./GlobalContext');
const THREE = require('../lib/three-min.js');
const symbols = require('./symbols');
const {_elementGetter, _elementSetter} = require('./utils');

const _getXrDisplay = window => window[symbols.mrDisplaysSymbol].xrDisplay;
const _getXmDisplay = window => window[symbols.mrDisplaysSymbol].xmDisplay;
const _getFakeVrDisplay = window => {
  const {fakeVrDisplay} = window[symbols.mrDisplaysSymbol];
  return fakeVrDisplay.isActive ? fakeVrDisplay : null;
};

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
    const fakeVrDisplay = _getFakeVrDisplay(this._window);
    if (fakeVrDisplay) {
      return Promise.resolve(fakeVrDisplay);
    } else if ((name === 'VR' || name === null) && GlobalContext.nativeVr && GlobalContext.nativeVr.VR_IsHmdPresent()) {
      return Promise.resolve(_getXrDisplay(this._window));
    } else if ((name === 'AR' || name === null) && GlobalContext.nativeMl && GlobalContext.nativeMl.IsPresent()) {
      return Promise.resolve(_getXmDisplay(this._window));
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
    this.ownerDocument = null; // non-standard
    
    this._layers = [];
  }
  supportsSession({exclusive = false, outputContext = null} = {}) {
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
  update(update) {
    if (this.session) {
      this.session.update(update);
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
  clone() {
    const o = new this.constructor();
    for (const k in this) {
      o[k] = this[k];
    }
    if (o.session) {
      o.session = o.session.clone();
      o.session.device = o;
    }
    return o;
  }
}
module.exports.XRDevice = XRDevice;

class XRSession extends EventTarget {
  constructor({device = null, exclusive = false, outputContext = null} = {}) {
    super();

    this.device = device;
    this.exclusive = exclusive;
    this.outputContext = outputContext;

    this.depthNear = 0.1;
    this.depthFar = 10000.0;
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
    return this._inputSources;
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
  update(update) {
    const {
      depthNear,
      depthFar,
      renderWidth,
      renderHeight,
      frameData,
      stageParameters,
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
    /* if (stageParameters !== undefined) {
      this._frameOfReference.emulatedHeight = stageParameters.position.y; // XXX
    } */
    if (gamepads !== undefined) {
      const scale = localVector3.set(1, 1, 1);

      for (let i = 0; i < 2; i++) {
        const gamepad = gamepads[i];
        if (gamepad) {
          const inputSource = this._inputSources[i];
          const position = localVector.fromArray(gamepad.pose.position);
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
  }
  clone() {
    const o = new this.constructor();
    for (const k in this) {
      o[k] = this[k];
    }
    o.baseLayer = null;
    o._frame = o._frame.clone();
    o._frame.session = o;
    return o;
  }
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

    localMatrix.toArray(inputSource._pose.targetRay.transformMatrix);
    
    return inputSource._pose;
  }
  clone() {
    const o = new this.constructor();
    for (const k in this) {
      o[k] = this[k];
    }
    o._pose = o._pose.clone();
    o._pose.frame = o;
    return o;
  }
}
module.exports.XRPresentationFrame = XRPresentationFrame;

class XRView {
  constructor(
    eye = 'left',
    projectionMatrix = Float32Array.from([
      2.1445069205095586, 0, 0, 0,
      0, 2.1445069205095586, 0, 0,
      0, 0, -1.00010000500025, -1,
      0, 0, -0.200010000500025, 0,
    ]),
  ) {
    this.eye = eye;
    this.projectionMatrix = projectionMatrix;

    this._viewport = new XRViewport();
    this._viewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
    this._localViewMatrix = this._viewMatrix.slice();
  }
}
module.exports.XRView = XRView;

class XRViewport {
  constructor(x = 0, y = 0, width = defaultCanvasSize[0], height = defaultCanvasSize[1]) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
  }
  set(x, y, width, height) {
    this.x = x;
    this.y = y;
    this.width = width;
    this.height = height;
  }
}
module.exports.XRViewport = XRViewport;

class XRDevicePose {
  constructor(frame) {
    this.frame = frame; // non-standard
    this.poseModelMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
  getViewMatrix(view) {
    if (this.frame && this.frame.session && this.frame.session.device && this.frame.session.device.window) {
      const {xrOffset} = this.frame.session.device.window.document;
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
  clone() {
    const o = new this.constructor();
    for (const k in this) {
      o[k] = this[k];
    }
    return o;
  }
}
module.exports.XRDevicePose = XRDevicePose;

class XRInputSource {
  constructor(handedness = 'left', pointerOrigin = 'hand') {
    this.handedness = handedness;
    this.pointerOrigin = pointerOrigin;

    this._pose = new XRInputPose();
  }
}
module.exports.XRInputSource = XRInputSource;

class XRRay {
  constructor() {
    this.origin = new GlobalContext.DOMPoint(0, 0, 0, 1);
    this.direction = new GlobalContext.DOMPoint(0, 0, 1, 0);
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
      .compose(localVector.fromArray(this.position), localQuaternion.fromArray(this.orientation), localVector2.fromArray(this.scale))
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
