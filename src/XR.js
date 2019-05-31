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
  async supportsSession(mode) {
    return true;
  }
  supportsSessionMode(mode) { // XXX non-standard
    return this.supportsSession(mode);
  }
  requestSession({exclusive = false, outputContext = null} = {}) {
    if (!this.session) {
      const hmdType = getHMDType();

      if (hmdType) {
        const device = this._window[symbols.mrDisplaysSymbol].vrDevice;

        const session = new XRSession({
          device,
          exclusive,
          outputContext,
        });
        await device.onrequestpresent();
        session.once('end', () => {
          this.session = null;
        });
        this.session = session;
      } else {
        return Promise.reject(null);
      }
    }
    return Promise.resolve(this.session);
  }
  /* async requestDevice() {
    const hmdType = getHMDType();

    if (hmdType) {
      if (hmdType === 'fake') {
        return this._window[symbols.mrDisplaysSymbol].fakeVrDisplay;
      } else {
        return this._window[symbols.mrDisplaysSymbol].vrDevice;
      }
    } else {
      return null;
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

class XRDevice {
  constructor(name, window) {
    this.name = name; // non-standard
    this.window = window; // non-standard
    this.session = null; // non-standard

    this.onrequestpresent = null;
    this.onmakeswapchain = null;
    this.onexitpresent = null;
    this.onrequestanimationframe = null;
    
    this._layers = [];
  }
  /* supportsSession() {
    return Promise.resolve(null);
  }
  async requestSession({exclusive = false, outputContext = null} = {}) {
    if (!this.session) {
      const session = new XRSession({
        device: this,
        exclusive,
        outputContext,
      });
      await this.onrequestpresent();
      session.once('end', () => {
        this.session = null;
      });
      this.session = session;
    }
    return this.session;
  } */
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

    this.renderState = new XRRenderState();

    this._frame = new XRFrame(this);
    this._frameOfReference = new XRFrameOfReference();
    this._inputSources = (() => {
      const result = Array(2 + maxNumTrackers);
      for (let i = 0; i < maxNumTrackers; i++) {
        let hand, pointerOrigin;
        if (i === 0) {
          hand = 'left';
          pointerOrigin = 'hand';
        } else if (i === 1) {
          hand = 'right';
          pointerOrigin = 'hand';
        } else {
          hand = null;
          pointerOrigin = 'tracker';
        }
        result[i] = new XRInputSource(hand, pointerOrigin, i);
      }
      return result;
    })();
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
  requestReferenceSpace(type, options = {}) {
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
  updateRenderState(newState) {
    this.renderState.set(newState);
  }
  end() {
    this.emit('end');
    return Promise.resolve();
  }
  update() {
    const gamepads = GlobalContext.getGamepads(this.device.window);
    
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
  
  set(newState) {
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
      multiview = false,
      framebufferScaleFactor = 1,
    } = options;
    this.antialias = antialias;
    this.depth = depth;
    this.stencil = stencil;
    this.alpha = alpha;
    this.multiview = multiview;

    const {fbo} = this.session.device.onmakeswapchain(context);
    
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

class XRFrame {
  constructor(session) {
    this.session = session;
    this.views = [
      new XRView('left'),
      new XRView('right'),
    ];

    this._pose = new XRViewerPose(this);
  }
  getViewerPose(coordinateSystem) {
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
module.exports.XRFrame = XRFrame;

class XRView {
  constructor(eye = 'left') {
    this.eye = eye;
    this.projectionMatrix = eye === 'left' ? GlobalContext.xrState.leftProjectionMatrix : GlobalContext.xrState.rightProjectionMatrix;
    this.viewMatrix = eye === 'left' ? GlobalContext.xrState.leftViewMatrix : GlobalContext.xrState.rightViewMatrix; // XXX non-standard
    this.transform = new XRRigidTransform();

    this._viewport = new XRViewport(eye);
    /* this._viewMatrix = eye === 'left' ? GlobalContext.xrState.leftViewMatrix : GlobalContext.xrState.rightViewMatrix;
    this._localViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]); */
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

class XRPose {
  constructor() {
    this.transform = new XRRigidTransform();
    this.emulatedPosition = false;
  }
}
module.exports.XRPose = XRPose;

class XRViewerPose extends XRPose {
  constructor() {
    super();

    this._views = [
      new XRView('left'),
      new XRView('right'),
    ];
    /* this.frame = frame; // non-standard
    this.poseModelMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]); */
  }
  /* getViewMatrix(view) {
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
  } */
  get views() {
    return this._views;
  }
  set views(views) {}
}
module.exports.XRViewerPose = XRViewerPose;

class XRInputSource {
  constructor(handedness = 'left', pointerOrigin = 'hand', index = 0) {
    this.handedness = handedness;
    this.pointerOrigin = pointerOrigin;
    this._index = index;

    this._pose = new XRInputPose();
    const gamepad = GlobalContext.xrState.gamepads[index];
    this._pose.targetRay.origin.values = gamepad.position;
    this._pose.targetRay.direction.values = gamepad.direction;
    this._pose._localPointerMatrix = gamepad.transformMatrix;
  }
  get connected() {
    return GlobalContext.xrState.gamepads[this._index].connected[0] !== 0;
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
    this._environmentBlendMode = 'opaque';
    this._targetRay = new XRRay();
    this._targetRayMode = 'hand';
    this.gripMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
    // this._localPointerMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
  get environmentBlendMode() {
    return this._environmentBlendMode;
  }
  set environmentBlendMode(environmentBlendMode) {}
  get targetRayMode() {
    return this._targetRay;
  }
  set targetRayMode(targetRayMode) {}
  get targetRay() {
    return this._targetRay;
  }
  set targetRay(targetRay) {}
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
    if (position instanceof SharedArrayBuffer) {
      this.initialize(position);
    } else {
      this.initialize();

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
    }
  }
  
  initialize(_buffer = new SharedArrayBuffer((3 + 4 + 3 + 16*2) * Float32Array.BYTES_PER_ELEMENT)) {
    this._buffer = _buffer;
    let index = 0;

    this.position = new Float32Array(this._buffer, index, 3);
    index += 3 * Float32Array.BYTES_PER_ELEMENT;

    this.orientation = new Float32Array(this._buffer, index, 4);
    index += 4 * Float32Array.BYTES_PER_ELEMENT;

    this.scale = new Float32Array(this._buffer, index, 3);
    index += 3 * Float32Array.BYTES_PER_ELEMENT;

    this.matrix = new Float32Array(this._buffer, index, 16);
    index += 16 * Float32Array.BYTES_PER_ELEMENT;

    this.matrixInverse = new Float32Array(this._buffer, index, 16);
    index += 16 * Float32Array.BYTES_PER_ELEMENT;
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
