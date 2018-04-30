const events = require('events');
const {EventEmitter} = events;
const stream = require('stream');
const path = require('path');
const fs = require('fs');
const url = require('url');
const dgram = require('dgram');
const os = require('os');
const util = require('util');
const {URL} = url;
const {performance} = require('perf_hooks');

const parseIntStrict = require('parse-int');
const parse5 = require('parse5');

const fetch = require('window-fetch');
const {XMLHttpRequest} = require('window-xhr');
const XHRUtils = require('window-xhr/lib/utils');
const {Request, Response, Blob} = fetch;
const WebSocket = require('ws/lib/websocket');
const {LocalStorage} = require('node-localstorage');
const createMemoryHistory = require('history/createMemoryHistory').default;
const ClassList = require('window-classlist');
const he = require('he');
he.encode.options.useNamedReferences = true;
const selector = require('window-selector');
const css = require('css');
const {TextEncoder, TextDecoder} = require('text-encoding');
const parseXml = require('@rgrove/parse-xml');
const THREE = require('./lib/three-min.js');

const windowSymbol = Symbol();
const htmlTagsSymbol = Symbol();
const optionsSymbol = Symbol();
const elementSymbol = Symbol();
const computedStyleSymbol = Symbol();
const disabledEventsSymbol = Symbol();
const pointerLockElementSymbol = Symbol();
const mrDisplaysSymbol = Symbol();
let nativeBindings = false;
let args = {};
let version = '';

const btoa = s => new Buffer(s, 'binary').toString('base64');
const atob = s => new Buffer(s, 'base64').toString('binary');

let styleEpoch = 0;

let id = 0;
const urls = new Map();
URL.createObjectURL = blob => {
  const url = 'blob:' + id++;
  urls.set(url, blob);
  return url;
};
URL.revokeObjectURL = blob => {
  urls.delete(url);
};

const redirectUrls = {};

XHRUtils.createClient = (createClient => function() {
  const properties = arguments[0];
  if (properties._responseFn) {
    const cb = arguments[2];
    properties._responseFn(cb);
    return {
      on() {},
      setHeader() {},
      write() {},
      end() {},
    };
  } else {
    return createClient.apply(this, arguments);
  }
})(XHRUtils.createClient);

class Location extends EventEmitter {
  constructor(u) {
    super();

    this._url = new url.URL(u);
  }
  // triggers navigation
  get href() { return this._url.href || ''; }
  set href(href) { this._url.href = href; this.update(); }
  get protocol() { return this._url.protocol || ''; }
  set protocol(protocol) { this._url.protocol = protocol; this.update(); }
  get host() { return this._url.host || ''; }
  set host(host) { this._url.host = host; this.update(); }
  get hostname() { return this._url.hostname || ''; }
  set hostname(hostname) { this._url.hostname = hostname; this.update(); }
  get port() { return this._url.port || ''; }
  set port(port) { this._url.port = port; this.update(); }
  get pathname() { return this._url.pathname || ''; }
  set pathname(pathname) { this._url.pathname = pathname; this.update(); }
  get search() { return this._url.search || ''; }
  set search(search) { this._url.search = search; this.update(); }
  // does not trigger navigation
  get hash() { return this._url.hash || ''; }
  set hash(hash) { this._url.hash = hash; }
  get username() { return this._url.username || ''; }
  set username(username) { this._url.username = username; }
  get password() { return this._url.password || ''; }
  set password(password) { this._url.password = password; }
  get origin() { return this._url.origin || ''; }
  set origin(origin) {} // read only
  // conversions
  toString() {
    return this.href;
  }
  // helpers
  set(u) {
    this._url.href = u;
  }
  update() {
    this.emit('update', this.href);
  }
}
class History extends EventEmitter {
  constructor(u) {
    super();

    this._history = createMemoryHistory({
      initialEntries: [u],
    });
    this._history.listen((location, action) => {
      if (action === 'POP') {
        const {pathname, search, hash, state} = location;
        this.emit('popstate', url.format({
          pathname,
          search,
          hash,
        }), state);
      }
    });
  }
  back(n) {
    this._history.goBack(n);
  }
  forward(n) {
    this._history.goForward(n);
  }
  go(n) {
    this._history.go(n);
  }
  pushState(state, title, url) {
    this._history.push(url, state);
  }
  replaceState(state, title, url) {
    this._history.replace(url, state);
  }
  get length() {
    return this._history.length;
  }
  set length(length) {}
  get state() {
    return this._history.location.state;
  }
  set state(state) {}
}

class Event {
  constructor(type, init = {}) {
    this.type = type;
    this.target = init.target ? init.target : null;

    this.defaultPrevented = false;
    this.propagationStopped = false;
    this.currentTarget = null;
  }

  preventDefault() {
    this.defaultPrevented = true;
  }

  stopPropagation() {
    this.propagationStopped = true;
  }

  initEvent(type, bubbles, cancelable) {
    this.type = type;
  }
}
class KeyboardEvent extends Event {
  constructor(type, init = {}) {
    super(type, init);

    this.init(init);
  }

  init(init) {
    this.key = init.key !== undefined ? init.key : '';
    this.code = init.code !== undefined ? init.code : '';
    this.location = init.location !== undefined ? init.location : 0;
    this.ctrlKey = init.ctrlKey !== undefined ? init.ctrlKey : false;
    this.shiftKey = init.shiftKey !== undefined ? init.shiftKey : false;
    this.altKey = init.altKey !== undefined ? init.altKey : false;
    this.metaKey = init.metaKey !== undefined ? init.metaKey : false;
    this.repeat = init.repeat !== undefined ? init.repeat : false;
    this.isComposing = init.isComposing !== undefined ? init.isComposing : false;
    this.charCode = init.charCode !== undefined ? init.charCode : 0;
    this.keyCode = init.keyCode !== undefined ? init.keyCode : 0;
    this.which = init.which !== undefined ? init.which : 0;
  }

  initKeyboardEvent(type, canBubble, cancelable, view, charCode, keyCode, location, modifiersList, repeat) {
    this.type = type;

    const modifiers = modifiers.split(/\s/);
    const ctrlKey = modifiers.includes('Control') || modifiers.includes('AltGraph');
    const altKey = modifiers.includes('Alt') || modifiers.includes('AltGraph');
    const metaKey = modifiers.includes('Meta');

    this.init({
      charCode,
      keyCode,
      ctrlKey,
      altKey,
      metaKey,
      repeat,
    });
  }
}
class MouseEvent extends Event {
  constructor(type, init = {}) {
    super(type);

    this.init(init);
  }

  init(init = {}) {
    this.screenX = init.screenX !== undefined ? init.screenX : 0;
    this.screenY = init.screenY !== undefined ? init.screenY : 0;
    this.clientX = init.clientX !== undefined ? init.clientX : 0;
    this.clientY = init.clientY !== undefined ? init.clientY : 0;
    this.pageX = init.pageX !== undefined ? init.pageX : 0;
    this.pageY = init.pageY !== undefined ? init.pageY : 0;
    this.movementX = init.movementX !== undefined ? init.movementX : 0;
    this.movementY = init.movementY !== undefined ? init.movementY : 0;
    this.ctrlKey = init.ctrlKey !== undefined ? init.ctrlKey : false;
    this.shiftKey = init.shiftKey !== undefined ? init.shiftKey : false;
    this.altKey = init.altKey !== undefined ? init.altKey : false;
    this.metaKey = init.metaKey !== undefined ? init.metaKey : false;
    this.button = init.button !== undefined ? init.button : 0;
    this.relatedTarget = init.relatedTarget !== undefined ? init.relatedTarget : null;
    this.region = init.region !== undefined ? init.region : null;
  }

  initMouseEvent(type, canBubble, cancelable, view, detail, screenX, screenY, clientX, clientY, ctrlKey, altKey, shiftKey, metaKey, button, relatedTarget) {
    this.type = type;

    this.init({
      screenX,
      screenY,
      clientX,
      clientY,
      ctrlKey,
      altKey,
      shiftKey,
      metaKey,
      button,
      relatedTarget,
    });
  }
}
class WheelEvent extends MouseEvent {
  constructor(type, init = {}) {
    super(type, init);

    this.deltaX = init.deltaX !== undefined ? init.deltaX : 0;
    this.deltaY = init.deltaY !== undefined ? init.deltaY : 0;
    this.deltaZ = init.deltaZ !== undefined ? init.deltaZ : 0;
    this.deltaMode = init.deltaMode !== undefined ? init.deltaMode : 0;
  }
}
WheelEvent.DOM_DELTA_PIXEL = 0x00;
WheelEvent.DOM_DELTA_LINE = 0x01;
WheelEvent.DOM_DELTA_PAGE = 0x02;
class MessageEvent extends Event {
  constructor(data) {
    super('message');

    this.data = data;
  }
}
class CustomEvent extends Event {
  constructor(type, init = {}) {
    super(type, init);

    this.init(init);
  }

  init(init = {}) {
    this.detail = init.detail !== undefined ? init.detail : null;
  }
}

class MutationRecord {
  constructor(type, target, addedNodes, removedNodes, previousSibling, nextSibling, attributeName, attributeNamespace, oldValue) {
    this.type = type;
    this.target = target;
    this.addedNodes = addedNodes;
    this.removedNodes = removedNodes;
    this.previousSibling = previousSibling;
    this.nextSibling = nextSibling;
    this.attributeName = attributeName;
    this.attributeNamespace = attributeNamespace;
    this.oldValue = oldValue;
  }
}
class MutationObserver {
  constructor(callback) {
    this.callback = callback;

    this.element = null;
    this.options = null;
    this.queue = [];
    this.bindings = new WeakMap();
  }

  observe(element, options) {
    this.element = element;
    this.options = options;

    this.bind(element);
  }

  disconnect() {
    this.unbind(this.element);

    this.element = null;
    this.options = null;
  }

  takeRecords() {
    const oldQueue = this.queue.slice();
    this.queue.length = 0;
    return oldQueue;
  }

  bind(el) {
    el.traverse(el => {
      const _attribute = (name, value) => this.handleAttribute(el, name, value);
      el.on('attribute', _attribute);
      const _children = (addedNodes, removedNodes, previousSibling, nextSibling) => this.handleChildren(el, addedNodes, removedNodes, previousSibling, nextSibling);
      el.on('children', _children);
      const _value = () => this.handleValue(el);
      el.on('value', _value);

      this.bindings.set(el, [
        _attribute,
        _children,
        _value,
      ]);
    });
  }

  unbind(el) {
    el.traverse(el => {
      const bindings = this.bindings.get(el);
      for (let i = 0; i < bindings.length; i++) {
        el.removeListener(bindings[i]);
      }
      this.bindings.remove(el);
    });
  }

  flush() {
    if (this.queue.length > 0) {
      const oldQueue = this.queue.slice();
      this.queue.length = 0;
      this.callback(oldQueue, this);
    }
  }

  handleAttribute(el, name, value, oldValue) {
    this.queue.push(new MutationRecord('attributes', el, null, null, null, null, name, null, oldValue));

    setImmediate(() => {
      this.flush();
    });
  }

  handleChildren(el, addedNodes, removedNodes, previousSibling, nextSibling) {
    this.queue.push(new MutationRecord('childList', el, addedNodes, removedNodes, previousSibling, nextSibling, null, null, null));

    for (let i = 0; i < addedNodes.length; i++) {
      this.bind(addedNodes[i]);
    }
    for (let i = 0; i < removedNodes.length; i++) {
      this.unbind(removedNodes[i]);
    }

    setImmediate(() => {
      this.flush();
    });
  }

  handleValue(el) {
    this.queue.push(new MutationRecord('characterData', el, [], [], null, null, null, null, null));

    setImmediate(() => {
      this.flush();
    });
  }
}
let Image = null;
class ImageData {
  constructor(width, height) {
    this.width = width;
    this.height = height;
    this.data = new Uint8ClampedArray(0);
  }
}
class ImageBitmap {
  constructor() {
    if (arguments.length === 1) {
      const [image] = arguments;
      this.width = image.width;
      this.height = image.height;
      this.data = image.data;
    } else if (arguments.length === 3) {
      const [width, height, data] = arguments;
      this.width = width;
      this.height = height;
      this.data = data;
    } else {
      throw new Error('invalid arguments');
    }
  }
}
ImageBitmap.createImageBitmap = function() {
  return Reflect.construct(ImageBitmap, arguments);
};
let nativeVm = null;
class nativeWorker {
  terminate() {}
}
class Path2D {
  moveTo() {}
  lineTo() {}
  quadraticCurveTo() {}
}
class CanvasGradient {}
class CanvasRenderingContext2D {
  drawImage() {}
  fillRect() {}
  clearRect() {}
  fillText() {}
  stroke() {}
  scale() {}
  measureText() {
    return {width: 0};
  }
  createImageData(w, h) {
    return new ImageData(w, h);
  }
  getImageData(sx, sy, sw, sh) {
    return new ImageData(sw, sh);
  }
  putImageData() {}
}
const VERSION = Symbol();
class WebGLRenderingContext {
  get VERSION() {
    return VERSION;
  }
  getExtension() {
    return null;
  }
  getParameter(param) {
    if (param === VERSION) {
      return 'WebGL 1';
    } else {
      return null;
    }
  }
  createTexture() {}
  bindTexture() {}
  texParameteri() {}
  texImage2D() {}
  createProgram() {}
  createShader() {}
  shaderSource() {}
  compileShader() {}
  getShaderParameter() {}
  getShaderInfoLog() {
    return '';
  }
  attachShader() {}
  linkProgram() {}
  getProgramInfoLog() {
    return '';
  }
  getProgramParameter() {}
  deleteShader() {}
  clearColor() {}
  clearDepth() {}
  clearStencil() {}
  enable() {}
  disable() {}
  depthFunc() {}
  frontFace() {}
  cullFace() {}
  blendEquationSeparate() {}
  blendFuncSeparate() {}
  viewport() {}
}
class Screen {
  constructor(window) {
    this._window = window;
  }

  get top() {
    return 0;
  }
  set top(top) {}
  get left() {
    return 0;
  }
  set left(left) {}
  get width() {
    return this._window.innerWidth;
  }
  set width(width) {}
  get height() {
    return this._window.innerHeight;
  }
  set height(height) {}
  get colorDepth() {
    return 24;
  }
  set colorDepth(colorDepth) {}
  get orientation() {
    return {
      angle: 0,
      type: 'landscape-primary',
      onchange: null,
    };
  }
  set orientation(orientation) {}

  get pixelDepth() {
    return this.colorDepth;
  }
  set pixelDepth(pixelDepth) {}
  get availTop() {
    return this.top;
  }
  set availTop(availTop) {}
  get availLeft() {
    return this.left;
  }
  set availLeft(availLeft) {}
  get availWidth() {
    return this.width;
  }
  set availWidth(availWidth) {}
  get availHeight() {
    return this.height;
  }
  set availHeight(availHeight) {}
}
let nativeVr = null;
let nativeMl = null;
const maxNumPlanes = 32 * 3;
const planeEntrySize = 3 + 4 + 2 + 1;
class VRFrameData {
  constructor() {
    // new THREE.PerspectiveCamera().projectionMatrix.toArray()
    this.leftProjectionMatrix = Float32Array.from([2.1445069205095586, 0, 0, 0, 0, 2.1445069205095586, 0, 0, 0, 0, -1.00010000500025, -1, 0, 0, -0.200010000500025, 0]);
    // new THREE.Matrix4().toArray()
    this.leftViewMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
    this.rightProjectionMatrix = this.leftProjectionMatrix.slice();
    this.rightViewMatrix = this.leftViewMatrix.slice();
    this.pose = new VRPose();

    // non-standard
    this.planes = new Float32Array(maxNumPlanes * planeEntrySize);
    this.numPlanes = 0;
  }

  copy(frameData) {
    this.leftProjectionMatrix.set(frameData.leftProjectionMatrix);
    this.leftViewMatrix.set(frameData.leftViewMatrix);
    this.rightProjectionMatrix.set(frameData.rightProjectionMatrix);
    this.rightViewMatrix.set(frameData.rightViewMatrix);
    this.pose.copy(frameData.pose);

    // non-standard
    this.planes.set(frameData.planes);
    this.numPlanes = frameData.numPlanes;
  }
}
class VRPose {
  constructor(position = new Float32Array(3), orientation = Float32Array.from([0, 0, 0, 1])) {
    this.position = position;
    this.orientation = orientation;
  }

  set(position, orientation) {
    position.toArray(this.position);
    orientation.toArray(this.orientation);
  }

  copy(vrPose) {
    this.position.set(vrPose.position);
    this.orientation.set(vrPose.orientation);
  }
}
class GamepadButton {
  constructor() {
     this.value = 0;
     this.pressed = false;
     this.touched = false;
  }

  copy(button) {
    this.value = button.value;
    this.pressed = button.pressed;
    this.touched = button.touched;
  }
}
class GamepadPose {
  constructor() {
    this.hasPosition = true;
    this.hasOrientation = true;
    this.position = new Float32Array(3);
    this.linearVelocity = new Float32Array(3);
    this.linearAcceleration = new Float32Array(3);
    this.orientation = Float32Array.from([0, 0, 0, 1]);
    this.angularVelocity = new Float32Array(3);
    this.angularAcceleration = new Float32Array(3);
  }

  copy(pose) {
    this.hasPosition = pose.hasPosition;
    this.hasOrientation = pose.hasOrientation;
    this.position.set(pose.position);
    this.linearVelocity.set(pose.linearVelocity);
    this.linearAcceleration.set(pose.linearAcceleration);
    this.orientation.set(pose.orientation);
    this.angularVelocity.set(pose.angularVelocity);
    this.angularAcceleration.set(pose.angularAcceleration);
  }
}
class GamepadGesture {
  constructor() {
    this.position = new Float32Array(3);
    this.gesture = null;
  }

  copy(gesture) {
    this.position.set(gesture.position);
    this.gesture = gesture.gesture;
  }
}
class Gamepad {
  constructor(hand, index) {
    this.hand = hand;
    this.index = index;

    this.connected = true;
    this.buttons = [
      new GamepadButton(),
      new GamepadButton(),
      new GamepadButton(),
      new GamepadButton(),
    ];
    this.pose = new GamepadPose();
    this.axes = new Float32Array(2);

    // non-standard
    this.gesture = new GamepadGesture();
  }

  copy(gamepad) {
    this.connected = gamepad.connected;
    for (let i = 0; i < this.buttons.length; i++) {
      this.buttons[i].copy(gamepad.buttons[i]);
    }
    this.pose.copy(gamepad.pose);
    this.axes.set(gamepad.axes);

    // non-standard
    this.gesture.copy(gamepad.gesture);
  }
}
class VRStageParameters {
  constructor() {
    // new THREE.Matrix4().compose(new THREE.Vector3(0, 0, 0), new THREE.Quaternion(), new THREE.Vector3(1, 1, 1)).toArray(new Float32Array(16))
    this.sittingToStandingTransform = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }

  copy(vrStageParameters) {
    this.sittingToStandingTransform.set(vrStageParameters.sittingToStandingTransform);
  }
}
const localVector = new THREE.Vector3();
const localVector2 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
const localMatrix2 = new THREE.Matrix4();
class MRDisplay {
  constructor(name, window, displayId) {
    this.name = name;
    this[windowSymbol] = window;
    this.displayId = displayId;

    this.isPresenting = false;
    this.capabilities = {
      canPresent: true,
      hasExternalDisplay: true,
      hasPosition: true,
      maxLayers: 1,
    };
    this.depthNear = 0.1;
    this.depthFar = 10000.0;
    this.stageParameters = new VRStageParameters();

    this._width = window.innerWidth / 2;
    this._height = window.innerHeight;
    this._leftOffset = 0;
    this._leftFov = Float32Array.from([45, 45, 45, 45]);
    this._rightOffset = 0;
    this._rightFov = Float32Array.from([45, 45, 45, 45]);
    this._cleanups = [];
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
      renderWidth: this._width,
      renderHeight: this._height,
      offset: leftEye ? this._leftOffset : this._rightOffset,
      fieldOfView: _fovArrayToVRFieldOfView(leftEye ? this._leftFov : this._rightFov),
    };
  }

  requestPresent(layers) {
    return (nativeVr !== null ? nativeVr.requestPresent(layers) : Promise.resolve())
      .then(() => {
        this.isPresenting = true;

        process.nextTick(() => {
          const e = new Event('vrdisplaypresentchange');
          e.display = this;
          this[windowSymbol].dispatchEvent(e);
        });
      });
  }

  exitPresent() {
    return (nativeVr !== null ? nativeVr.exitPresent() : Promise.resolve())
      .then(() => {
        this.isPresenting = false;

        for (let i = 0; i < this._rafs.length; i++) {
          this.cancelAnimationFrame(this._rafs[i]);
        }
        this._rafs.length = 0;

        process.nextTick(() => {
          const e = new Event('vrdisplaypresentchange');
          e.display = this;
          this[windowSymbol].dispatchEvent(e);
        });
      });
  }

  requestAnimationFrame(fn) {
    const animationFrame = this[windowSymbol].requestAnimationFrame(timestamp => {
      this._rafs.splice(animationFrame, 1);
      fn(timestamp);
    });
    this._rafs.push(animationFrame);
    return animationFrame;
  }

  cancelAnimationFrame(animationFrame) {
    const result = this[windowSymbol].cancelAnimationFrame(animationFrame);
    const index = this._rafs.indexOf(animationFrame);
    if (index !== -1) {
      this._rafs.splice(index, 1);
    }
    return result;
  }

  submitFrame() {}

  destroy() {
    for (let i = 0; i < this._rafs.length; i++) {
      this.cancelAnimationFrame(this._rafs[i]);
    }
    for (let i = 0; i < this._cleanups.length; i++) {
      this._cleanups[i]();
    }
  }
}
class VRDisplay extends MRDisplay {
  constructor(window, displayId) {
    super('VR', window, displayId);

    this._frameData = new VRFrameData();

    const _updatevrframe = update => {
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
        this._leftOffset = leftOffset;
      }
      if (leftFov !== undefined) {
        this._leftFov = leftOffset;
      }
      if (rightOffset !== undefined) {
        this._rightOffset = rightOffset;
      }
      if (rightFov !== undefined) {
        this._rightFov = rightFov;
      }
      if (frameData !== undefined) {
        this._frameData.copy(frameData);
      }
      if (stageParameters !== undefined) {
        this.stageParameters.copy(stageParameters);
      }
    };
    window.top.on('updatevrframe', _updatevrframe);

    this._cleanups.push(() => {
      window.top.removeListener('updatevrframe', _updatevrframe);
    });
  }

  getFrameData(frameData) {
    frameData.copy(this._frameData);
  }
}
class FakeDisplay extends MRDisplay {
  constructor(window, displayId) {
    super('FAKE', window, displayId);

    this.position = new THREE.Vector3();
    this.quaternion = new THREE.Quaternion();
    this.gamepads = [leftGamepad, rightGamepad];

    this.isPresenting = false;
    this.depthNear = 0.1;
    this.depthFar = 10 * 1024;
    this._width = defaultCanvasSize[0];
    this._height = defaultCanvasSize[1];
    this._leftOffset = 0;
    this._leftFov = 90;
    this._rightOffset = 0;
    this._rightFov = 90;
    this.stageParameters = new VRStageParameters();

    this._frameData = new VRFrameData();
  }

  update() {
    localMatrix.compose(
      this.position,
      this.quaternion,
      localVector.set(1, 1, 1)
    )
     .getInverse(localMatrix)
     .toArray(this._frameData.leftViewMatrix);
    this._frameData.rightViewMatrix.set(this._frameData.leftViewMatrix);
    this._frameData.pose.set(this.position, this.quaternion);

    localGamepads[0] = leftGamepad;
    localGamepads[1] = rightGamepad;
  }

  requestPresent(layers) {
    return Promise.resolve()
      .then(() => {
        this.isPresenting = true;
      });
  }
  
  exitPresent() {
    return Promise.resolve()
      .then(() => {
        this.isPresenting = false;
      });
  }

  getFrameData(frameData) {
    frameData.copy(this._frameData);
  }
}
/* class ARDisplay extends MRDisplay {
  constructor(window, displayId) {
    super('AR', window, displayId);

    this._viewMatrix = new Float32Array(16);
    this._projectionMatrix = new Float32Array(16);

    const _resize = () => {
      this._width = window.innerWidth / 2;
      this._height = window.innerHeight;
    };
    window.top.on('resize', _resize);
    const _updatearframe = (viewMatrix, projectionMatrix) => {
      this._viewMatrix.set(viewMatrix);
      this._projectionMatrix.set(projectionMatrix);
    };
    window.top.on('updatearframe', _updatearframe);

    this._cleanups.push(() => {
      window.top.removeListener('resize', _resize);
      window.top.removeListener('updatearframe', _updatearframe);
    });
  }

  getFrameData(frameData) {
    const hmdMatrix = localMatrix.fromArray(this._viewMatrix);
    hmdMatrix.decompose(localVector, localQuaternion, localVector2);
    frameData.pose.set(localVector, localQuaternion);

    frameData.leftViewMatrix.set(this._viewMatrix);
    frameData.rightViewMatrix.set(this._viewMatrix);

    frameData.leftProjectionMatrix.set(this._projectionMatrix);
    frameData.rightProjectionMatrix.set(this._projectionMatrix);
  }
} */
class MLMesh {
  constructor() {
    this.positions = new Float32Array(9);
    this.normals = Float32Array.from([
      0, 1, 0,
      0, 1, 0,
      0, 1, 0,
    ]);
    this.indices = Uint32Array.from([0, 1, 2]);
  }
}
class MLDisplay extends MRDisplay {
  constructor(window, displayId) {
    super('ML', window, displayId);

    new THREE.Matrix4().compose(
      new THREE.Vector3(0, 0, 0),
      new THREE.Quaternion(),
      new THREE.Vector3(1, 1, 1)
    ).toArray(this.stageParameters.sittingToStandingTransform);

    this._transformArray = Float32Array.from([
      0, 0, 0,
      0, 0, 0, 1,
      0, 0, 0,
      0, 0, 0, 1,
    ]);
    this._projectionArray = Float32Array.from([
      2.1445069205095586, 0, 0, 0,
      0, 2.1445069205095586, 0, 0,
      0, 0, -1.00010000500025, -1,
      0, 0, -0.200010000500025, 0,
      2.1445069205095586, 0, 0, 0,
      0, 2.1445069205095586, 0, 0,
      0, 0, -1.00010000500025, -1,
      0, 0, -0.200010000500025, 0,
    ]);
    // this._viewportArray = new Float32Array(4);
    this._planesArray = new Float32Array(maxNumPlanes * planeEntrySize);
    this._numPlanes = 0;
    this.mesh = [null, null, null];

    /* const _resize = () => {
      this._width = window.innerWidth / 2;
      this._height = window.innerHeight;
    };
    window.on('resize', _resize); */
    const _updatemlframe = update => {
      this._transformArray.set(update.transformArray);
      this._projectionArray.set(update.projectionArray);
      // this._viewportArray.set(update.viewportArray);
      this._planesArray.set(update.planesArray);
      this._numPlanes = update.numPlanes;
      for (let i = 0; i < 3; i++) {
        this.mesh[i] = update.meshArray[i];
      }

      this._width = update.viewportArray[2] / 2;
      this._height = update.viewportArray[3];
    };
    window.top.on('updatemlframe', _updatemlframe);

    this._cleanups.push(() => {
      // window.removeListener('resize', _resize);
      window.top.removeListener('updatemlframe', _updatemlframe);
    });
  }

  requestPresent(layers) {
    return (nativeMl ? nativeMl.requestPresent(layers) : Promise.resolve())
      .then(() => {
        this.isPresenting = true;

        process.nextTick(() => {
          const e = new Event('vrdisplaypresentchange');
          e.display = this;
          this[windowSymbol].dispatchEvent(e);
        });
      });
  }

  exitPresent() {
    return (nativeMl ? nativeMl.exitPresent() : Promise.resolve())
      .then(() => {
        this.isPresenting = false;

        for (let i = 0; i < this._rafs.length; i++) {
          this.cancelAnimationFrame(this._rafs[i]);
        }
        this._rafs.length = 0;

        process.nextTick(() => {
          const e = new Event('vrdisplaypresentchange');
          e.display = this;
          this[windowSymbol].dispatchEvent(e);
        });
      });
  }

  getFrameData(frameData) {
    localVector.set(this._transformArray[0], this._transformArray[1], this._transformArray[2]);
    localQuaternion.set(this._transformArray[3], this._transformArray[4], this._transformArray[5], this._transformArray[6]);
    localVector2.set(1, 1, 1);
    localMatrix.getInverse(localMatrix.compose(localVector, localQuaternion, localVector2));

    frameData.pose.set(localVector, localQuaternion);
    localMatrix.toArray(frameData.leftViewMatrix);

    localVector.set(this._transformArray[7], this._transformArray[8], this._transformArray[9]);
    localQuaternion.set(this._transformArray[10], this._transformArray[11], this._transformArray[12], this._transformArray[13]);
    localVector2.set(1, 1, 1);
    localMatrix.getInverse(localMatrix.compose(localVector, localQuaternion, localVector2));
    localMatrix.toArray(frameData.rightViewMatrix);

    frameData.leftProjectionMatrix.set(this._projectionArray.slice(0, 16));
    frameData.rightProjectionMatrix.set(this._projectionArray.slice(16, 32));

    if (frameData.planes) {
      frameData.planes.set(this._planesArray);
      frameData.numPlanes = this._numPlanes;
    }
  }
}
class AudioNode {
  connect() {}
}
class AudioDestinationNode extends AudioNode {}
class AudioListener extends AudioNode {
  constructor() {
    super();

    this.positionX = new AudioParam();
    this.positionY = new AudioParam();
    this.positionZ = new AudioParam();
    this.forwardX = new AudioParam();
    this.forwardY = new AudioParam();
    this.forwardZ = new AudioParam();
    this.upX = new AudioParam();
    this.upY = new AudioParam();
    this.upZ = new AudioParam();
  }

  setPosition(x, y, z) {
    this.positionX.value = x;
    this.positionY.value = y;
    this.positionZ.value = z;
  }

  setOrientation(fx, fy, fz, ux, uy, uz) {
    this.forwardX.value = fx;
    this.forwardY.value = fy;
    this.forwardZ.value = fz;
    this.upX.value = ux;
    this.upY.value = uy;
    this.upZ.value = uz;
  }
}
class AudioParam {
  constructor() {
    this.value = 0;
    this.minValue = 0;
    this.maxValue = 0;
    this.defaultValue = 0;
  }

  setValueAtTime() {}
  exponentialRampToValueAtTime() {}
  cancelScheduledValues() {}
}
class GainNode extends AudioNode {
  constructor() {
    super();

    this.gain = new AudioParam();
  }
}
class AnalyserNode extends AudioNode {}
class PannerNode extends AudioNode {
  setPosition() {}
}
class BiquadFilterNode extends AudioNode {
  constructor() {
    super();

    this.frequency = new AudioParam();
    this.detune = new AudioParam();
    this.Q = new AudioParam();
    this.gain = new AudioParam();
    this.type = '';
  }
}
class AudioBuffer {}
class StereoPannerNode extends AudioNode {}
class AudioContext {
  constructor() {
    this.listener = new AudioListener();

    this._startTime = 0;
    this._startTimestamp = Date.now();
  }

  get currentTime() {
    return this._startTime + (this._startTimestamp !== null ? (Date.now() - this._startTimestamp) : 0);
  }
  set currentTime(currentTime) {}

  suspend() {
    this._startTime = this.currentTime;
    this._startTimestamp = null;
    return Promise.resolve();
  }
  resume() {
    this._startTimestamp = Date.now();
    return Promise.resolve();
  }
  close() {
    this._startTimestamp = null;
    return Promise.resolve();
  }

  createMediaElementSource() {
    return new AudioNode();
  }
  createMediaStreamSource() {
    return new AudioNode();
  }
  createBufferSource() {
    return new AudioNode();
  }
  createGain() {
    return new GainNode();
  }
  createAnalyser() {
    return new AnalyserNode();
  }
  createPanner() {
    return new PannerNode();
  }
  createBiquadFilter() {
    return new BiquadFilterNode();
  }
  createBuffer() {
    return new AudioBuffer();
  }
}

class DOMRect {
  constructor(x = 0, y = 0, w = 0, h = 0) {
    this.x = x;
    this.y = y;
    this.width = w;
    this.height = h;
    this.left = w >= 0 ? x : x + w;
    this.top = h >= 0 ? y : y + h;
    this.right = w >= 0 ? x + w : x;
    this.bottom = h >= 0 ? y + h : y;
  }
}

class Node extends EventEmitter {
  constructor() {
    super();

    this.parentNode = null;
    this.childNodes = [];
    this.ownerDocument = null;
  }

  get parentElement() {
    if (this.parentNode && this.parentNode.nodeType === Node.ELEMENT_NODE) {
      return this.parentNode;
    } else {
      return null;
    }
  }
  set parentElement(parentElement) {}

  get nextSibling() {
    if (this.parentNode) {
      const selfIndex = this.parentNode.childNodes.indexOf(this);
      const nextIndex = selfIndex + 1;
      if (nextIndex < this.parentNode.childNodes.length) {
        return this.parentNode.childNodes[nextIndex];
      } else {
        return null;
      }
    } else {
      return null;
    }
  }
  set nextSibling(nextSibling) {}
  get previousSibling() {
    if (this.parentNode) {
      const selfIndex = this.parentNode.childNodes.indexOf(this);
      const prevIndex = selfIndex - 1;
      if (prevIndex >= 0) {
        return this.parentNode.childNodes[prevIndex];
      } else {
        return null;
      }
    } else {
      return null;
    }
  }
  set previousSibling(previousSibling) {}

  get nextElementSibling() {
    if (this.parentNode) {
      const selfIndex = this.parentNode.childNodes.indexOf(this);
      for (let i = selfIndex + 1; i < this.parentNode.childNodes.length; i++) {
        const childNode = this.parentNode.childNodes[i];
        if (childNode.nodeType === Node.ELEMENT_NODE) {
          return childNode;
        }
      }
      return null;
    } else {
      return null;
    }
  }
  set nextElementSibling(nextElementSibling) {}
  get previousElementSibling() {
    if (this.parentNode) {
      const selfIndex = this.parentNode.childNodes.indexOf(this);
      for (let i = selfIndex - 1; i >= 0; i--) {
        const childNode = this.parentNode.childNodes[i];
        if (childNode.nodeType === Node.ELEMENT_NODE) {
          return childNode;
        }
      }
      return null;
    } else {
      return null;
    }
  }
  set previousElementSibling(previousElementSibling) {}

  contains(el) {
    for (;;) {
      if (el === this) {
        return true;
      } else if (el.parentNode) {
        el = el.parentNode;
      } else {
        return false;
      }
    }
  }

  cloneNode(deep = false) {
    const el = new this.constructor();
    el.attrs = this.attrs;
    el.value = this.value;
    if (deep) {
      el.childNodes = this.childNodes.map(childNode => childNode.cloneNode(true));
    }
    return el;
  }

  _emit() { // need to call this instead of EventEmitter.prototype.emit because some frameworks override HTMLElement.prototype.emit()
    return EventEmitter.prototype.emit.apply(this, arguments);
  }
}
Node.ELEMENT_NODE = 1;
Node.TEXT_NODE = 3;
Node.PROCESSING_INSTRUCTION_NODE = 7;
Node.COMMENT_NODE = 8;
Node.DOCUMENT_NODE = 9;
Node.DOCUMENT_TYPE_NODE = 10;
Node.DOCUMENT_FRAGMENT_NODE = 11;
const _setAttributeRaw = (el, prop, value) => {
  const propN = parseIntStrict(prop);
  if (propN !== undefined) { // XXX handle attribute emits for indexed attribute sets
    el.attrs[propN] = value;
  } else if (prop === 'length') {
    el.attrs.length = value;
  } else {
    const attr = el.attrs.find(attr => attr.name === prop);
    if (!attr) {
      const attr = {
        name: prop,
        value,
      };
      el.attrs.push(attr);
      el._emit('attribute', prop, value, null);
    } else {
      const oldValue = attr.value;
      attr.value = value;
      el._emit('attribute', prop, value, oldValue);
    }
  }
};
const _makeAttributesProxy = el => new Proxy(el.attrs, {
  get(target, prop) {
    const propN = parseIntStrict(prop);
    if (propN !== undefined) {
      return target[propN];
    } else if (prop === 'length') {
      return target.length;
    } else {
      return target.find(attr => attr.name === prop);
    }
  },
  set(target, prop, value) {
    _setAttributeRaw(el, prop, value);
    return true;
  },
  deleteProperty(target, prop) {
    const index = target.findIndex(attr => attr.name === prop);
    if (index !== -1) {
      const oldValue = target[index].value;
      target.splice(index, 1);
      el._emit('attribute', prop, null, oldValue);
    }
    return true;
  },
  has(target, prop) {
    if (typeof prop === 'number') {
      return target[prop] !== undefined;
    } else if (prop === 'length') {
      return true;
    } else {
      return target.findIndex(attr => attr.name === prop) !== -1;
    }
  },
});
const _makeChildrenProxy = el => {
  const {HTMLElement} = el.ownerDocument.defaultView;
  const _getChildren = childNodes => childNodes.filter(childNode => childNode instanceof HTMLElement);

  return new Proxy(el.childNodes, {
    get(target, prop) {
      const propN = parseIntStrict(prop);
      if (propN !== undefined) {
        return _getChildren(target)[propN];
      } else if (prop === 'length') {
        return _getChildren(target).length;
      } else if (prop === 'item') {
        return i => {
          if (typeof i === 'number') {
            return _getChildren(target)[i];
          } else {
            return undefined;
          }
        };
      } else {
        return undefined;
      }
    },
    set(target, prop, value) {
      return true;
    },
    deleteProperty(target, prop) {
      return true;
    },
    has(target, prop) {
      if (typeof prop === 'number') {
        return _getChildren(target)[prop] !== undefined;
      } else if (prop === 'length' || prop === 'item') {
        return true;
      } else {
        return false;
      }
    },
  });
};
const _makeHtmlCollectionProxy = (el, query) => new Proxy(el, {
  get(target, prop) {
    const propN = parseIntStrict(prop);
    if (propN !== undefined) {
      return el.querySelectorAll(query)[propN];
    } else if (prop === 'length') {
      return el.querySelectorAll(query).length;
    } else {
      return undefined;
    }
  },
  set(target, prop, value) {
    return true;
  },
  deleteProperty(target, prop) {
    return true;
  },
  has(target, prop) {
    if (typeof prop === 'number') {
      return el.querySelectorAll(query)[prop] !== undefined;
    } else if (prop === 'length') {
      return true;
    } else {
      return false;
    }
  },
});
const _cssText = style => {
  let styleString = '';
  for (const k in style) {
    const v = style[k];
    if (v !== undefined) {
      styleString += (styleString.length > 0 ? ' ' : '') + k + ': ' + v + ';';
    }
  }
  return styleString;
};
const _makeStyleProxy = el => {
  let style = {};
  return new Proxy({}, {
    get(target, key) {
      if (key === 'reset') {
        return () => {
          let stylesheet, err;
          try {
            stylesheet = css.parse(`x{${el.getAttribute('style')}}`).stylesheet;
          } catch(e) {
            err = e;
          }
          if (!err) {
            style = {};
            const {rules} = stylesheet;
            for (let j = 0; j < rules.length; j++) {
              const rule = rules[j];
              const {declarations} = rule;
              for (let k = 0; k < declarations.length; k++) {
                const {property, value} = declarations[k];
                style[property] = value;
              }
            }
          }
        };
      } else if (key === 'clone') {
        return () => {
          const result = {};
          for (const k in style) {
            const v = style[k];
            if (v !== undefined) {
              result[k] = v;
            }
          }
          return result;
        };
      } else if (key === 'cssText') {
        return el.getAttribute('style') || '';
      } else {
        return style[key];
      }
    },
    set(target, key, value) {
      if (key === 'cssText') {
        el.setAttribute('style', value);
      } else {
        style[key] = value;
        el.setAttribute('style', _cssText(style));
      }
      return true;
    },
  });
};
const _dashToCamelCase = s => {
  let match = s.match(/^data-(.+)$/);
  if (match) {
    s = match[1];
    s = s.replace(/-([a-z])/g, (all, letter) => letter.toUpperCase());
    return s;
  } else {
    return null;
  }
};
const _camelCaseToDash = s => {
  if (!/-[a-z]/.test(s)) {
    s = 'data-' + s;
    s = s.replace(/([A-Z])/g, (all, letter) => '-' + letter.toLowerCase());
    return s;
  } else {
    return null;
  }
};
const _makeDataset = el => new Proxy(el.attrs, {
  get(target, key) {
    for (let i = 0; i < target.length; i++) {
      const attr = target[i];
      if (_dashToCamelCase(attr.name) === key) {
        return attr.value;
      }
    }
  },
  set(target, key, value) {
    const dashName = _camelCaseToDash(key);
    if (dashName) {
      _setAttributeRaw(el, dashName, value);
    }
    return true;
  },
});
const autoClosingTags = {
  area: true,
  base: true,
  br: true,
  embed: true,
  hr: true,
  iframe: true,
  img: true,
  input: true,
  link: true,
  meta: true,
  param: true,
  source: true,
  track: true,
  window: true,
};
const _defineId = (window, id, el) => {
  let value;
  Object.defineProperty(window, id, {
    get() {
      if (value !== undefined) {
        return value;
      } else if (el.ownerDocument.documentElement.contains(el) && el.getAttribute('id') === id) {
        return el;
      }
    },
    set(newValue) {
      value = newValue;
    },
    configurable: true,
  });
};
class Element extends Node {
  constructor(tagName = 'DIV', attrs = [], value = '', location = null) {
    super();

    this.tagName = tagName;
    this.attrs = attrs;
    this.value = value;
    this.location = location;

    this._attributes = null;
    this._children = null;
    this._innerHTML = '';
    this._classList = null;
    this._dataset = null;

    this.on('attribute', (name, value) => {
      if (name === 'id') {
        if (this.ownerDocument.defaultView[value] === undefined) {
          _defineId(this.ownerDocument.defaultView, value, this);
        }
      } else if (name === 'class' && this._classList) {
        this._classList.reset(value);
      }
    });
  }

  get nodeType() {
    return Node.ELEMENT_NODE;
  }
  set nodeType(nodeType) {}

  get nodeName() {
    return this.tagName;
  }
  set nodeName(nodeName) {}

  get attributes() {
    if (!this._attributes) {
      this._attributes = _makeAttributesProxy(this);
    }
    return this._attributes;
  }
  set attributes(attributes) {}

  get children() {
    if (!this._children) {
      this._children = _makeChildrenProxy(this);
    }
    return this._children;
  }
  set children(children) {}

  getAttribute(name) {
    const attr = this.attributes[name];
    return attr && attr.value;
  }
  setAttribute(name, value) {
    this.attributes[name] = value;
  }
  setAttributeNS(namespace, name, value) {
    this.setAttribute(name, value);
  }
  hasAttribute(name) {
    return name in this.attributes;
  }
  removeAttribute(name) {
    const oldValue = this.attributes[name];
    delete this.attributes[name];
  }

  appendChild(childNode) {
    if (childNode.parentNode) {
      childNode.parentNode.removeChild(childNode);
    }

    this.childNodes.push(childNode);
    childNode.parentNode = this;

    this._emit('children', [childNode], [], this.childNodes[this.childNodes.length - 2] || null, null);
    this.ownerDocument._emit('domchange');

    return childNode;
  }
  removeChild(childNode) {
    const index = this.childNodes.indexOf(childNode);
    if (index !== -1) {
      this.childNodes.splice(index, 1);
      childNode.parentNode = null;

      this._emit('children', [], [childNode], this.childNodes[index - 1] || null, this.childNodes[index] || null);
      this.ownerDocument._emit('domchange');

      return childNode;
    } else {
      throw new Error('The node to be removed is not a child of this node.');
    }
  }
  replaceChild(newChild, oldChild) {
    const index = this.childNodes.indexOf(oldChild);
    if (index !== -1) {
      this.childNodes.splice(index, 1, newChild);
      oldChild.parentNode = null;

      this._emit('children', [], [oldChild], this.childNodes[index - 1] || null, this.childNodes[index] || null);
      this.ownerDocument._emit('domchange');

      return oldChild;
    } else {
      throw new Error('The node to be replaced is not a child of this node.');
    }
  }
  insertBefore(childNode, nextSibling) {
    const index = this.childNodes.indexOf(nextSibling);
    if (index !== -1) {
      this.childNodes.splice(index, 0, childNode);
      childNode.parentNode = this;

      this._emit('children', [childNode], [], this.childNodes[index - 1] || null, this.childNodes[index + 1] || null);
      this.ownerDocument._emit('domchange');
    }
  }
  insertAfter(childNode, nextSibling) {
    const index = this.childNodes.indexOf(nextSibling);
    if (index !== -1) {
      this.childNodes.splice(index + 1, 0, childNode);
      childNode.parentNode = this;

      this._emit('children', [childNode], [], this.childNodes[index] || null, this.childNodes[index + 2] || null);
      this.ownerDocument._emit('domchange');
    }
  }

  get firstChild() {
    return this.childNodes.length > 0 ? this.childNodes[0] : null;
  }
  set firstChild(firstChild) {}
  get lastChild() {
    return this.childNodes.length > 0 ? this.childNodes[this.childNodes.length - 1] : null;
  }
  set lastChild(lastChild) {}

  get firstElementChild() {
    for (let i = 0; i < this.childNodes.length; i++) {
      const childNode = this.childNodes[i];
      if (childNode.nodeType === Node.ELEMENT_NODE) {
        return childNode;
      }
    }
    return null;
  }
  set firstElementChild(firstElementChild) {}
  get lastElementChild() {
    for (let i = this.childNodes.length - 1; i >= 0; i--) {
      const childNode = this.childNodes[i];
      if (childNode.nodeType === Node.ELEMENT_NODE) {
        return childNode;
      }
    }
    return null;
  }
  set lastElementChild(lastElementChild) {}

  get childElementCount() {
    let result = 0;
    for (let i = 0; i < this.childNodes.length; i++) {
      if (this.childNodes[i].nodeType === Node.ELEMENT_NODE) {
        result++;
      }
    }
    return result;
  }
  set childElementCount(childElementCount) {}

  get id() {
    return this.getAttribute('id') || '';
  }
  set id(id) {
    id = id + '';
    this.setAttribute('id', id);
  }

  get className() {
    return this.getAttribute('class') || '';
  }
  set className(className) {
    className = className + '';
    this.setAttribute('class', className);
  }

  get classList() {
    if (!this._classList) {
      this._classList = new ClassList(this.className, className => {
        _setAttributeRaw(this, 'className', className);
      });
    }
    return this._classList;
  }
  set classList(classList) {}

  getElementById(id) {
    id = id + '';
    return selector.find(this, '#' + id, true);
  }
  getElementByClassName(className) {
    className = className + '';
    return selector.find(this, '.' + className, true);
  }
  getElementByTagName(tagName) {
    tagName = tagName + '';
    return selector.find(this, tagName, true);
  }
  querySelector(s) {
    s = s + '';
    return selector.find(this, s, true);
  }
  getElementsById(id) {
    id = id + '';
    return selector.find(this, '#' + id);
  }
  getElementsByClassName(className) {
    className = className + '';
    return selector.find(this, '.' + className);
  }
  getElementsByTagName(tagName) {
    tagName = tagName + '';
    return selector.find(this, tagName);
  }
  querySelectorAll(s) {
    s = s + '';
    return selector.find(this, s);
  }
  matches(s) {
    s = s + '';
    return selector.matches(this, s);
  }

  dispatchEvent(event) {
    event.target = this;

    const _emit = (node, event) => {
      event.currentTarget = this;
      node._emit(event.type, event);
      event.currentTarget = null;
    };
    const _recurse = (node, event) => {
      _emit(node, event);

      if (!event.propagationStopped && node.parentNode) {
        _recurse(node.parentNode, event);
      }
    };
    _recurse(this, event);

    if (this.ownerDocument) {
      _emit(this.ownerDocument.defaultView, event);
    }
  }

  getBoundingClientRect() {
    return new DOMRect();
  }

  focus() {
    const document = this.ownerDocument;
    document.activeElement.dispatchEvent(new Event('blur', {
      target: document.activeElement,
    }));

    document.activeElement = this;
    this.dispatchEvent(new Event('focus', {
      target: this,
    }));
  }

  blur() {
    const document = this.ownerDocument;
    if (document.activeElement !== document.body) {
      document.body.focus();
    }
  }

  click() {
    this.dispatchEvent(new MouseEvent('click'));
  }

  addEventListener(event, listener) {
    if (typeof listener === 'function') {
      this.on(event, listener);
    }
  }
  removeEventListener(event, listener) {
    if (typeof listener === 'function') {
      this.removeListener(event, listener);
    }
  }

  get clientWidth() {
    const style = this.ownerDocument.defaultView.getComputedStyle(this);
    const fontFamily = style['font-family'];
    if (fontFamily) {
      return _hash(fontFamily) * _hash(this.innerHTML);
    } else {
      let result = 0;
      const _recurse = el => {
        if (el.nodeType === Node.ELEMENT_NODE) {
          if (el.tagName === 'CANVAS' || el.tagName === 'IMAGE' || el.tagName === 'VIDEO') {
            result = Math.max(el.width, result);
          }
          for (let i = 0; i < el.childNodes.length; i++) {
            _recurse(el.childNodes[i]);
          }
        }
      };
      _recurse(this);
      return result;
    }
  }
  set clientWidth(clientWidth) {}
  get clientHeight() {
    let result = 0;
    const _recurse = el => {
      if (el.nodeType === Node.ELEMENT_NODE) {
        if (el.tagName === 'CANVAS' || el.tagName === 'IMAGE' || el.tagName === 'VIDEO') {
          result = Math.max(el.height, result);
        }
        for (let i = 0; i < el.childNodes.length; i++) {
          _recurse(el.childNodes[i]);
        }
      }
    };
    _recurse(this);
    return result;
  }
  set clientHeight(clientHeight) {}

  get dataset() {
    if (!this._dataset) {
      this._dataset = _makeDataset(this);
    }
    return this._dataset;
  }
  set dataset(dataset) {}

  get innerHTML() {
    return parse5.serialize(this);
  }
  set innerHTML(innerHTML) {
    innerHTML = innerHTML + '';
    const oldChildNodes = this.childNodes;
    const newChildNodes = parse5.parseFragment(innerHTML, {
      locationInfo: true,
    }).childNodes.map(childNode => _fromAST(childNode, this.ownerDocument.defaultView, this, this.ownerDocument, true));
    this.childNodes = newChildNodes;

    if (oldChildNodes.length > 0) {
      this._emit('children', [], oldChildNodes, null, null);
    }
    if (newChildNodes.length > 0) {
      this._emit('children', newChildNodes, [], null, null);
    }
    this.ownerDocument._emit('domchange');

    _promiseSerial(newChildNodes.map(childNode => () => _runHtml(childNode, this.ownerDocument.defaultView)))
      .catch(err => {
        console.warn(err);
      });

    this._emit('innerHTML', innerHTML);
  }

  get innerText() {
    return he.encode(this.innerHTML);
  }
  set innerText(innerText) {
    innerText = innerText + '';
    this.innerHTML = he.decode(innerText);
  }

  get textContent() {
    let result = '';
    const _recurse = el => {
      if (el.nodeType === Node.TEXT_NODE) {
        result += el.value;
      } else if (el.childNodes) {
        for (let i = 0; i < el.childNodes.length; i++) {
          _recurse(el.childNodes[i]);
        }
      }
    };
    _recurse(this);
    return result;
  }
  set textContent(textContent) {
    textContent = textContent + '';

    while (this.childNodes.length > 0) {
      this.removeChild(this.childNodes[this.childNodes.length - 1]);
    }
    this.appendChild(new Text(textContent));
  }

  get onclick() {
    return _elementGetter(this, 'click');
  }
  set onclick(onclick) {
    _elementSetter(this, 'click', onclick);
  }
  get onmousedown() {
    return _elementGetter(this, 'mousedown');
  }
  set onmousedown(onmousedown) {
    _elementSetter(this, 'mousedown', onmousedown);
  }
  get onmouseup() {
    return _elementGetter(this, 'mouseup');
  }
  set onmouseup(onmouseup) {
    _elementSetter(this, 'mouseup', onmouseup);
  }

  requestPointerLock() {
    const topDocument = this.ownerDocument.defaultView.top.document;

    if (topDocument[pointerLockElementSymbol] === null) {
      topDocument[pointerLockElementSymbol] = this;

      process.nextTick(() => {
        topDocument._emit('pointerlockchange');
      });
    }
  }

  [util.inspect.custom]() {
    const _getIndent = depth => Array(depth*2 + 1).join(' ');
    const _recurse = (el, depth = 0) => {
      let result = '';
      if (el.tagName) {
        const tagName = el.tagName.toLowerCase();
        const indent = _getIndent(depth);
        const isAutoClosingTag = autoClosingTags[tagName];

        result += indent;
        result += '<' + tagName;
        for (let i = 0; i < el.attrs.length; i++) {
          const attr = el.attrs[i];
          result += ' ' + attr.name + '=' + JSON.stringify(attr.value);
        }
        if (isAutoClosingTag) {
          result += '/';
        }
        result += '>';

        if (!isAutoClosingTag) {
          let childrenResult = '';
          const childNodes = el.childNodes.concat(el.contentDocument ? [el.contentDocument] : []);
          for (let i = 0; i < childNodes.length; i++) {
            const childResult = _recurse(childNodes[i], depth + 1);
            if (childResult && !childrenResult) {
              childrenResult += '\n';
            }
            childrenResult += childResult;
          }
          if (childrenResult) {
            result += childrenResult;
            result += indent;
          }
          result += '</' + tagName + '>';
        }
        if (depth !== 0) {
          result += '\n';
        }
      } else if (el.constructor.name === 'Text' && /\S/.test(el.value)) {
        result += _getIndent(depth);
        result += el.value;
        if (depth !== 0) {
          result += '\n';
        }
      } else if (el.constructor.name === 'Comment') {
        result += _getIndent(depth);
        result += '<!--' + el.value + '-->';
        if (depth !== 0) {
          result += '\n';
        }
      }
      return result;
    };
    return _recurse(this);
  }

  traverse(fn) {
    const _recurse = node => {
      const result = fn(node);
      if (result !== undefined) {
        return result;
      } else {
        if (node.childNodes) {
          for (let i = 0; i < node.childNodes.length; i++) {
            const result = _recurse(node.childNodes[i]);
            if (result !== undefined) {
              return result;
            }
          }
        }
        if (node.contentDocument) {
          return _recurse(node.contentDocument);
        }
      }
    };
    return _recurse(this);
  }
  async traverseAsync(fn) {
    const nodes = [];
    (function _recurse(node) {
      nodes.push(node);
      if (node.childNodes) {
        for (let i = 0; i < node.childNodes.length; i++) {
          _recurse(node.childNodes[i]);
        }
      }
      if (node.contentDocument) {
        _recurse(node.contentDocument);
      }
    })(this);

    for (let i = 0; i < nodes.length; i++) {
      const result = await fn(nodes[i]);
      if (result !== undefined) {
        return result;
      }
    }
  }
}
class HTMLElement extends Element {
  constructor(tagName = 'DIV', attrs = [], value = '', location = null) {
    super(tagName, attrs, value, location);

    this._style = null;
    this[computedStyleSymbol] = null;

    this.on('attribute', (name, value) => {
      if (name === 'class' && this._classList) {
        this._classList.reset(value);
      } else if (name === 'style') {
        if (this._style) {
          this._style.reset();
        }
        if (this[computedStyleSymbol]) {
          this[computedStyleSymbol] = null;
        }
      }
    });
  }

  get offsetWidth() {
    return this.clientWidth;
  }
  set offsetWidth(offsetWidth) {}
  get offsetHeight() {
    return this.clientHeight;
  }
  set offsetHeight(offsetHeight) {}
  get offsetTop() {
    return 0;
  }
  set offsetTop(offsetTop) {}
  get offsetLeft() {
    return 0;
  }
  set offsetLeft(offsetLeft) {}

  get offsetParent() {
    const body = this.ownerDocument.body;
    for (let el = this; el; el = el.parentNode) {
      if (el.parentNode === body) {
        return body;
      }
    }
    return null;
  }
  set offsetParent(offsetParent) {}

  get style() {
    if (!this._style) {
      this._style = _makeStyleProxy(this);
    }
    return this._style;
  }
  set style(style) {}
}
class HTMLAnchorElement extends HTMLElement {
  constructor(attrs = [], value = '', location = null) {
    super('A', attrs, value, location);
  }

  get href() {
    return this.getAttribute('href') || '';
  }
  set href(href) {
    href = href + '';
    this.setAttribute('href', href);
  }
  get hash() {
    return new url.URL(this.href).hash || '';
  }
  set hash(hash) {
    hash = hash + '';
    const u = new url.URL(this.href);
    u.hash = hash;
    this.href = u.href;
  }
  get host() {
    return new url.URL(this.href).host || '';
  }
  set host(host) {
    host = host + '';
    const u = new url.URL(this.href);
    u.host = host;
    this.href = u.href;
  }
  get hostname() {
    return new url.URL(this.href).hostname || '';
  }
  set hostname(hostname) {
    hostname = hostname + '';
    const u = new url.URL(this.href);
    u.hostname = hostname;
    this.href = u.href;
  }
  get password() {
    return new url.URL(this.href).password || '';
  }
  set password(password) {
    password = password + '';
    const u = new url.URL(this.href);
    u.password = password;
    this.href = u.href;
  }
  get origin() {
    return new url.URL(this.href).origin || '';
  }
  set origin(origin) {
    origin = origin + '';
    const u = new url.URL(this.href);
    u.origin = origin;
    this.href = u.href;
  }
  get pathname() {
    return new url.URL(this.href).pathname || '';
  }
  set pathname(pathname) {
    pathname = pathname + '';
    const u = new url.URL(this.href);
    u.pathname = pathname;
    this.href = u.href;
  }
  get port() {
    return new url.URL(this.href).port || '';
  }
  set port(port) {
    port = port + '';
    const u = new url.URL(this.href);
    u.port = port;
    this.href = u.href;
  }
  get protocol() {
    return new url.URL(this.href).protocol || '';
  }
  set protocol(protocol) {
    protocol = protocol + '';
    const u = new url.URL(this.href);
    u.protocol = protocol;
    this.href = u.href;
  }
  get search() {
    return new url.URL(this.href).search || '';
  }
  set search(search) {
    search = search + '';
    const u = new url.URL(this.href);
    u.search = search;
    this.href = u.href;
  }
  get username() {
    return new url.URL(this.href).username || '';
  }
  set username(username) {
    username = username + '';
    const u = new url.URL(this.href);
    u.username = username;
    this.href = u.href;
  }
}
class HTMLLoadableElement extends HTMLElement {
  constructor(tagName, attrs = [], value = '', location = null) {
    super(tagName, attrs, value, location);
  }

  get onload() {
    return _elementGetter(this, 'load');
  }
  set onload(onload) {
    _elementSetter(this, 'load', onload);
  }

  get onerror() {
    return _elementGetter(this, 'error');
  }
  set onerror(onerror) {
    _elementSetter(this, 'error', onerror);
  }
}
class Document extends HTMLLoadableElement {
  constructor() {
    super('DOCUMENT');

    this.hidden = false;
  }

  get nodeType() {
    return Node.DOCUMENT_NODE;
  }

  get pointerLockElement() {
    if (this.defaultView.top === this.defaultView) {
      return this[pointerLockElementSymbol];
    } else {
      return this.defaultView.top.document.pointerLockElement;
    }
  }
  set pointerLockElement(pointerLockElement) {}

  exitPointerLock() {
    const topDocument = this.defaultView.top.document;

    if (topDocument[pointerLockElementSymbol] !== null) {
      topDocument[pointerLockElementSymbol] = null;

      process.nextTick(() => {
        topDocument._emit('pointerlockchange');
      });
    }
  }
}
class DocumentFragment extends HTMLElement {
  constructor() {
    super('DOCUMENTFRAGMENT');
  }

  get nodeType() {
    return Node.DOCUMENT_FRAGMENT_NODE;
  }
}
class HTMLBodyElement extends HTMLElement {
  constructor() {
    super('BODY');
  }

  get clientWidth() {
    return this.ownerDocument.defaultView.innerWidth;
  }
  set clientWidth(clientWidth) {}
  get clientHeight() {
    return this.ownerDocument.defaultView.innerHeight;
  }
  set clientHeight(clientHeight) {}
}
class HTMLStyleElement extends HTMLLoadableElement {
  constructor(attrs = [], value = '', location = null) {
    super('STYLE', attrs, value, location);

    this.stylesheet = null;

    this.on('attribute', (name, value) => {
      if (name === 'src' && this.isRunnable()) {
        const url = value;
        this.ownerDocument.defaultView.fetch(url)
          .then(res => {
            if (res.status >= 200 && res.status < 300) {
              return res.text();
            } else {
              return Promise.reject(new Error('style src got invalid status code: ' + res.status + ' : ' + url));
            }
          })
          .then(s => css.parse(s).stylesheet)
          .then(stylesheet => {
            this.stylesheet = stylesheet;
            styleEpoch++;
            this.dispatchEvent(new Event('load', {target: this}));
          })
          .catch(err => {
            this.dispatchEvent(new Event('error', {target: this}));
          });
      }
    });
    this.on('innerHTML', innerHTML => {
      Promise.resolve()
        .then(() => css.parse(innerHTML).stylesheet)
        .then(stylesheet => {
          this.stylesheet = stylesheet;
          styleEpoch++;
          this.dispatchEvent(new Event('load', {target: this}));
        })
        .catch(err => {
          this.dispatchEvent(new Event('error', {target: this}));
        });
    });
  }

  get src() {
    return this.getAttribute('src') || '';
  }
  set src(src) {
    src = src + '';
    this.setAttribute('src', src);
  }

  get type() {
    type = type + '';
    return this.getAttribute('type') || '';
  }
  set type(type) {
    this.setAttribute('type', type);
  }

  set innerHTML(innerHTML) {
    innerHTML = innerHTML + '';
    this._emit('innerHTML', innerHTML);
  }

  run() {
    let running = false;
    const srcAttr = this.attributes.src;
    if (srcAttr) {
      this._emit('attribute', 'src', srcAttr.value);
      running = true;
    }
    if (this.childNodes.length > 0) {
      this.innerHTML = this.childNodes[0].value;
      running = true;
    }
    return running;
  }
}
class HTMLScriptElement extends HTMLLoadableElement {
  constructor(attrs = [], value = '', location = null) {
    super('SCRIPT', attrs, value, location);

    this.readyState = null;

    this.on('attribute', (name, value) => {
      if (name === 'src' && this.isRunnable()) {
        this.readyState = null;

        const url = value;
        this.ownerDocument.defaultView.fetch(url)
          .then(res => {
            if (res.status >= 200 && res.status < 300) {
              return res.text();
            } else {
              return Promise.reject(new Error('script src got invalid status code: ' + res.status + ' : ' + url));
            }
          })
          .then(s => {
            _runJavascript(s, this.ownerDocument.defaultView, url);

            this.readyState = 'complete';

            this.dispatchEvent(new Event('load', {target: this}));
          })
          .catch(err => {
            this.readyState = 'complete';

            this.dispatchEvent(new Event('error', {target: this}));
          });
      }
    });
    this.on('innerHTML', innerHTML => {
      if (this.isRunnable()) {
        const window = this.ownerDocument.defaultView;
        _runJavascript(innerHTML, window, window.location.href, this.location.line !== null ? this.location.line - 1 : 0, this.location.col !== null ? this.location.col - 1 : 0);

        this.readyState = 'complete';

        process.nextTick(() => {
          this.dispatchEvent(new Event('load', {target: this}));
        });
      }
    });
  }

  get src() {
    return this.getAttribute('src') || '';
  }
  set src(src) {
    src = src + '';
    this.setAttribute('src', src);
  }

  get type() {
    return this.getAttribute('type') || '';
  }
  set type(type) {
    type = type + '';
    this.setAttribute('type', type);
  }

  get text() {
    let result = '';
    this.traverse(el => {
      if (el.nodeType === Node.TEXT_NODE) {
        result += el.value;
      }
    });
    return result;
  }
  set text(text) {
    this.textContent = text;
  }

  set innerHTML(innerHTML) {
    innerHTML = innerHTML + '';
    this._emit('innerHTML', innerHTML);
  }

  set textContent(textContent) {
    this.innerHTML = textContent;
  }

  isRunnable() {
    const {type} = this;
    return !type || /^(?:(?:text|application)\/javascript|application\/ecmascript)$/.test(type);
  }

  run() {
    if (this.isRunnable()) {
      let running = false;
      const srcAttr = this.attributes.src;
      if (srcAttr) {
        this._emit('attribute', 'src', srcAttr.value);
        running = true;
      }
      if (this.childNodes.length > 0) {
        this.innerHTML = this.childNodes[0].value;
        running = true;
      }
      return running;
    } else {
      return false;
    }
  }
}
class HTMLSrcableElement extends HTMLLoadableElement {
  constructor(tagName = null, attrs = [], value = '', location = null) {
    super(tagName, attrs, value, location);
  }

  get src() {
    return this.getAttribute('src');
  }
  set src(value) {
    this.setAttribute('src', value);
  }

  run() {
    const srcAttr = this.attributes.src;
    if (srcAttr) {
      this._emit('attribute', 'src', srcAttr.value);
      return true;
    } else {
      return false;
    }
  }
}
class HTMLMediaElement extends HTMLSrcableElement {
  constructor(tagName = null, attrs = [], value = '', location = null) {
    super(tagName, attrs, value, location);

    this._startTime = 0;
    this._startTimestamp = null;
  }

  get currentTime() {
    return this._startTime + (this._startTimestamp !== null ? (Date.now() - this._startTimestamp) : 0);
  }
  set currentTime(currentTime) {}

  play() {
    this._startTimestamp = Date.now();
  }
  pause() {}

  get paused() {
    return true;
  }
  set paused(paused) {
    this._startTime = this.currentTime;
    this._startTimestamp = null;
  }
  get duration() {
    return 1;
  }
  set duration(duration) {}
  get loop() {
    return false;
  }
  set loop(loop) {}
  get autoplay() {
    return false;
  }
  set autoplay(autoplay) {}

  canPlayType(type) {
    return ''; // XXX
  }

  get HAVE_NOTHING() {
    return HTMLMediaElement.HAVE_NOTHING;
  }
  set HAVE_NOTHING(v) {}
  get HAVE_METADATA() {
    return HTMLMediaElement.HAVE_METADATA;
  }
  set HAVE_METADATA(v) {}
  get HAVE_CURRENT_DATA() {
    return HTMLMediaElement.HAVE_CURRENT_DATA;
  }
  set HAVE_CURRENT_DATA(v) {}
  get HAVE_FUTURE_DATA() {
    return HTMLMediaElement.HAVE_FUTURE_DATA;
  }
  set HAVE_FUTURE_DATA(v) {}
  get HAVE_ENOUGH_DATA() {
    return HTMLMediaElement.HAVE_ENOUGH_DATA;
  }
  set HAVE_ENOUGH_DATA(v) {}
}
HTMLMediaElement.HAVE_NOTHING = 0;
HTMLMediaElement.HAVE_METADATA = 1;
HTMLMediaElement.HAVE_CURRENT_DATA = 2;
HTMLMediaElement.HAVE_FUTURE_DATA = 3;
HTMLMediaElement.HAVE_ENOUGH_DATA = 4;
class HTMLSourceElement extends HTMLSrcableElement {
  constructor(attrs = [], value = '', location = null) {
    super('SOURCE', attrs, value, location);
  }
}
class HTMLImageElement extends HTMLSrcableElement {
  constructor(attrs = [], value = '', location = null) {
    super('IMG', attrs, value, location);

    this.data = new Uint8Array(0);
    // this.stack = new Error().stack;

    this.on('attribute', (name, value) => {
      if (name === 'src') {
        process.nextTick(() => { // XXX
          this.dispatchEvent(new Event('load', {target: this}));
        });
      }
    });
  }

  get width() {
    return 0;
  }
  set width(width) {}

  get height() {
    return 0;
  }
  set height(height) {}

  get naturalWidth() {
    return this.width;
  }
  set naturalWidth(naturalWidth) {}

  get naturalHeight() {
    return this.height;
  }
  set naturalHeight(naturalHeight) {}
};
class HTMLAudioElement extends HTMLMediaElement {
  constructor(attrs = [], value = '', location = null) {
    super('AUDIO', attrs, value, location);

    this.readyState = HTMLMediaElement.HAVE_NOTHING;

    this.on('attribute', (name, value) => {
      if (name === 'src') {
        this.readyState = HTMLMediaElement.HAVE_ENOUGH_DATA;

        process.nextTick(() => { // XXX
          this.dispatchEvent(new Event('canplay', {target: this}));
          this.dispatchEvent(new Event('canplaythrough', {target: this}));
        });
      }
    });
  }

  get oncanplay() {
    return _elementGetter(this, 'canplay');
  }
  set oncanplay(oncanplay) {
    _elementSetter(this, 'canplay', oncanplay);
  }

  get oncanplaythrough() {
    return _elementGetter(this, 'canplaythrough');
  }
  set oncanplaythrough(oncanplaythrough) {
    _elementSetter(this, 'canplaythrough', oncanplaythrough);
  }
}
HTMLAudioElement.HAVE_NOTHING = HTMLMediaElement.HAVE_NOTHING;
HTMLAudioElement.HAVE_METADATA = HTMLMediaElement.HAVE_METADATA;
HTMLAudioElement.HAVE_CURRENT_DATA = HTMLMediaElement.HAVE_CURRENT_DATA;
HTMLAudioElement.HAVE_FUTURE_DATA = HTMLMediaElement.HAVE_FUTURE_DATA;
HTMLAudioElement.HAVE_ENOUGH_DATA = HTMLMediaElement.HAVE_ENOUGH_DATA;
class MicrophoneMediaStream {}
class HTMLVideoElement extends HTMLMediaElement {
  constructor(attrs = [], value = '', location = null) {
    super('VIDEO', attrs, value, location);

    this.readyState = HTMLMediaElement.HAVE_NOTHING;
    this.data = new Uint8Array(0);

    this.on('attribute', (name, value) => {
      if (name === 'src') {
        this.readyState = HTMLMediaElement.HAVE_ENOUGH_DATA;

        process.nextTick(() => { // XXX
          this.dispatchEvent(new Event('canplay', {target: this}));
          this.dispatchEvent(new Event('canplaythrough', {target: this}));
        });
      }
    });
  }

  get width() {
    return 0;
  }
  set width(width) {}
  get height() {
    return 0;
  }
  set height(height) {}
}
HTMLVideoElement.HAVE_NOTHING = HTMLMediaElement.HAVE_NOTHING;
HTMLVideoElement.HAVE_METADATA = HTMLMediaElement.HAVE_METADATA;
HTMLVideoElement.HAVE_CURRENT_DATA = HTMLMediaElement.HAVE_CURRENT_DATA;
HTMLVideoElement.HAVE_FUTURE_DATA = HTMLMediaElement.HAVE_FUTURE_DATA;
HTMLVideoElement.HAVE_ENOUGH_DATA = HTMLMediaElement.HAVE_ENOUGH_DATA;
class HTMLIframeElement extends HTMLSrcableElement {
  constructor(attrs = [], value = '', location = null) {
    super('IFRAME', attrs, value, location);

    this.contentWindow = null;
    this.contentDocument = null;
    this.live = true;

    this.on('attribute', (name, value) => {
      if (name === 'src') {
        let url = value;
        const match = url.match(/^javascript:(.+)$/); // XXX should support this for regular fetches too
        if (match) {
          url = 'data:text/html,' + encodeURIComponent(`<!doctype html><html><head><script>${match[1]}</script></head></html>`);
        }
        this.ownerDocument.defaultView.fetch(url)
          .then(res => {
            if (res.status >= 200 && res.status < 300) {
              return res.text();
            } else {
              return Promise.reject(new Error('iframe src got invalid status code: ' + res.status + ' : ' + url));
            }
          })
          .then(htmlString => {
            if (this.live) {
              const parentWindow = this.ownerDocument.defaultView;

              const contentWindow = _makeWindow({
                url,
                baseUrl: url,
                dataPath: parentWindow[optionsSymbol].dataPath,
              }, parentWindow, parentWindow.top);
              const contentDocument = _parseDocument(htmlString, contentWindow[optionsSymbol], contentWindow);
              contentDocument.hidden = this.hidden;

              contentWindow.document = contentDocument;

              this.contentWindow = contentWindow;
              this.contentDocument = contentDocument;

              contentWindow.on('destroy', e => {
                parentWindow.emit('destroy', e);
              });

              contentDocument.once('readystatechange', () => {
                this.dispatchEvent(new Event('load', {target: this}));
              });
              contentDocument.on('framebuffer', framebuffer => {
                this._emit('framebuffer', framebuffer);
              });
            }
          })
          .catch(err => {
            this._emit('error', err);
          });
      } else if (name === 'hidden') {
        if (this.contentDocument) {
          this.contentDocument.hidden = value;
        }
      }
    });
    this.on('destroy', () => {
      if (this.contentWindow) {
        this.contentWindow.destroy();
        this.contentWindow = null;
      }
      this.contentDocument = null;
    });
  }

  get hidden() {
    return this.getAttribute('hidden');
  }
  set hidden(hidden) {
    this.setAttribute('hidden', hidden);
  }

  destroy() {
    if (this.live) {
      this._emit('destroy');
      this.live = false;
    }
  }
}
const defaultCanvasSize = [1280, 1024];
const defaultEyeSeparation = 0.625;
class HTMLCanvasElement extends HTMLElement {
  constructor(attrs = [], value = '', location = null) {
    super('CANVAS', attrs, value, location);

    this._context = null;

    this.on('attribute', (name, value) => {
      if (name === 'width') {
        // console.log('gl canvas set width', this.width, this.height, this._context && this._context.resize, new Error().stack);
        this._context && this._context.resize && this._context.resize(this.width, this.height);
      } else if (name === 'height') {
        // console.log('gl canvas set height', this.width, this.height, this._context && this._context.resize, new Error().stack);
        this._context && this._context.resize && this._context.resize(this.width, this.height);
      }
    });
  }

  get width() {
     return this.getAttribute('width') || defaultCanvasSize[0];
  }
  set width(value) {
    if (typeof value === 'number' && isFinite(value)) {
      this.setAttribute('width', value);
    }
  }
  get height() {
    return this.getAttribute('height') || defaultCanvasSize[1];
  }
  set height(value) {
    if (typeof value === 'number' && isFinite(value)) {
      this.setAttribute('height', value);
    }
  }

  get clientWidth() {
    return this.width;
  }
  set clientWidth(clientWidth) {}
  get clientHeight() {
    return this.height;
  }
  set clientHeight(clientHeight) {}

  getBoundingClientRect() {
    return new DOMRect(0, 0, this.width, this.height);
  }

  get data() {
    return (this._context && this._context.data) || null;
  }
  set data(data) {}

  getContext(contextType) {
    if (this._context === null) {
      if (contextType === '2d') {
        this._context = new CanvasRenderingContext2D(this.width, this.height);
      } else if (contextType === 'webgl') {
        this._context = new WebGLRenderingContext(this);
      }
    }
    return this._context;
  }

  captureStream(frameRate) {
    return {}; // XXX
  }
}
class MediaRecorder extends EventEmitter {
  constructor() {
    super();
  }

  start() {}

  stop() {}

  requestData() {}
}
class HTMLTemplateElement extends HTMLElement {
  constructor(attrs = [], value = '', location = null) {
    super('TEMPLATE', attrs, value, location);
  }

  get content() {
    const wrapperEl = this.ownerDocument.createElement('div');
    wrapperEl.childNodes = this.childNodes;
    return wrapperEl;
  }
  set content(content) {}
}
class CharacterNode extends Node {
  constructor(value) {
    super();

    this.value = value;
  }

  get textContent() {
    return this.value;
  }
  set textContent(textContent) {
    this.value = textContent;

    this._emit('value');
  }

  get data() {
    return this.value;
  }
  set data(data) {
    this.value = data;

    this._emit('value');
  }
  get length() {
    return this.value.length;
  }
  set length(length) {}

  get firstChild() {
    return null;
  }
  set firstChild(firstChild) {}
  get lastChild() {
    return null;
  }
  set lastChild(lastChild) {}

  traverse(fn) {
    fn(this);
  }
}
class Text extends CharacterNode {
  constructor(value) {
    super(value);
  }

  get nodeType() {
    return Node.TEXT_NODE;
  }
  set nodeType(nodeType) {}

  get nodeName() {
    return '#text';
  }
  set nodeName(nodeName) {}

  get firstChild() {
    return null;
  }
  set firstChild(firstChild) {}
  get lastChild() {
    return null;
  }
  set lastChild(lastChild) {}

  [util.inspect.custom]() {
    return JSON.stringify(this.value);
  }
}
class Comment extends CharacterNode {
  constructor(value) {
    super(value);
  }

  get nodeType() {
    return Node.COMMENT_NODE;
  }
  set nodeType(nodeType) {}

  get nodeName() {
    return '#comment';
  }
  set nodeName(nodeName) {}

  [util.inspect.custom]() {
    return `<!--${this.value}-->`;
  }
}

class DocumentType {}
class DOMImplementation {
  constructor(window) {
    this._window = window;
  }

  createDocument() {
    throw new Error('not implemented');
  }

  createDocumentType() {
    return new DocumentType();
  }

  createHTMLDocument() {
    return _parseDocument('', this._window[optionsSymbol], this._window);
  }

  hasFeature() {
    return false;
  }
}

class DataTransfer {
  constructor() {
    this.items = [];
    this.files = [];
  }
}
class DataTransferItem {
  constructor(kind = 'string', type = 'text/plain', data = null) {
    this.kind = kind;
    this.type = type;
    this.data = data;
  }

  getAsFile() {
    return new Blob([this.data], {
      type: this.type,
    });
  }

  getAsString() {
    return this.data;
  }
}

const _fromAST = (node, window, parentNode, ownerDocument, uppercase) => {
  if (node.nodeName === '#text') {
    const text = new Text(node.value);
    text.parentNode = parentNode;
    text.ownerDocument = ownerDocument;
    return text;
  } else if (node.nodeName === '#comment') {
    const comment = new Comment(node.data);
    comment.parentNode = parentNode;
    comment.ownerDocument = ownerDocument;
    return comment;
  } else {
    let {tagName} = node;
    if (tagName && uppercase) {
      tagName = tagName.toUpperCase();
    }
    const {attrs, value, __location} = node;
    const HTMLElementTemplate = window[htmlTagsSymbol][tagName];
    const location = __location ? {
      line: __location.line,
      col: __location.col,
    } : null;
    const element = HTMLElementTemplate ?
      new HTMLElementTemplate(
        attrs,
        value,
        location,
      )
    :
      new HTMLElement(
        tagName,
        attrs,
        value,
        location,
      );
    element.parentNode = parentNode;
    if (!ownerDocument) { // if there is no owner document, it's us
      ownerDocument = element;
      ownerDocument.defaultView = window;
    }
    element.ownerDocument = ownerDocument;
    if (node.childNodes) {
      element.childNodes = node.childNodes.map(childNode => _fromAST(childNode, window, element, ownerDocument, uppercase));
    }
    return element;
  }
};
const _hash = s => {
  let result = 0;
  for (let i = 0; i < s.length; i++) {
    result += s.codePointAt(i);
  }
  return result;
};
const _elementGetter = (self, attribute) => self.listeners(attribute)[0];
const _elementSetter = (self, attribute, cb) => {
  if (typeof cb === 'function') {
    self.addEventListener(attribute, cb);
  } else {
    const listeners = self.listeners(attribute);
    for (let i = 0; i < listeners.length; i++) {
      self.removeEventListener(attribute, listeners[i]);
    }
  }
};
const _promiseSerial = async promiseFns => {
  for (let i = 0; i < promiseFns.length; i++) {
    await promiseFns[i]();
  }
};
const _loadPromise = el => new Promise((accept, reject) => {
  const load = () => {
    _cleanup();
    accept();
  };
  const error = err => {
    _cleanup();
    reject(err);
  };
  const _cleanup = () => {
    el.removeListener('load', load);
    el.removeListener('error', error);
  };
  el.on('load', load);
  el.on('error', error);
});
const _runHtml = (element, window) => {
  if (element instanceof HTMLElement) {
    return element.traverseAsync(async el => {
      const {id} = el;
      if (id) {
        el._emit('attribute', 'id', id);
      }

      if (el instanceof window.HTMLStyleElement) {
        if (el.run()) {
          if (el.childNodes.length > 0) {
            try {
              await _loadPromise(el)
                .catch(err => {
                  console.warn(err);
                });
            } catch(err) {
              console.warn(err);
            }
          } else {
            _loadPromise(el)
              .catch(err => {
                console.warn(err);
              });
          }
        }
      } else if (el instanceof window.HTMLScriptElement) {
        if (el.run()) {
          const asyncAttr = el.attributes.async;
          if (!(asyncAttr && asyncAttr.value)) {
            try {
              await _loadPromise(el);
            } catch(err) {
              console.warn(err);
            }
          } else {
            _loadPromise(el)
              .catch(err => {
                console.warn(err);
              });
          }
        }
      } else if (el instanceof window.HTMLImageElement) {
        if (el.run()) {
          await _loadPromise(el);
        }
      } else if (el instanceof window.HTMLAudioElement || el instanceof window.HTMLVideoElement) {
        el.run();
      }
    });
  } else {
    return Promise.resolve();
  }
};
const _runJavascript = (jsString, window, filename = 'script', lineOffset = 0, colOffset = 0) => {
  try {
    window.vm.run(jsString, filename, lineOffset, colOffset);
  } catch (err) {
    console.warn(err.stack);
  }
};

let rafCbs = [];
const tickAnimationFrame = () => {
  if (rafCbs.length > 0) {
    const localRafCbs = rafCbs.slice();

    const now = performance.now();
    for (let i = 0; i < localRafCbs.length; i++) {
      const localRafCb = localRafCbs[i];
      if (rafCbs.includes(localRafCb)) {
        localRafCb(now);

        const index = rafCbs.indexOf(localRafCb);
        if (index !== -1) {
          rafCbs.splice(index, 1);
        }
      }
    }
  }
};

const fakeVrDisplays = [];
const localGamepads = [null, null];
const leftGamepad = new Gamepad('left', 0);
const rightGamepad = new Gamepad('right', 1);
/* let vrMode = null;
let vrTexture = null;
let vrTextures = []; */
const _getVrDisplay = window => window[mrDisplaysSymbol] ? window[mrDisplaysSymbol].vrDisplay : window.top[mrDisplaysSymbol].vrDisplay;
const _getMlDisplay = window => window[mrDisplaysSymbol] ? window[mrDisplaysSymbol].mlDisplay : window.top[mrDisplaysSymbol].mlDisplay;

const _makeWindow = (options = {}, parent = null, top = null) => {
  const _normalizeUrl = src => {
    if (!/^[a-z]+:\/\//i.test(src)) {
      src = new URL(src, options.baseUrl).href
        .replace(/^(file:\/\/)\/([a-z]:.*)$/i, '$1$2');
    }
    return src;
  };

  const HTMLImageElementBound = (Old => class HTMLImageElement extends Old {
    constructor() {
      super(...arguments);

      this.ownerDocument = window.document; // need to set owner document here because HTMLImageElement can be manually constructed via new Image()
    }
  })(HTMLImageElement);
  const HTMLAudioElementBound = (Old => class HTMLAudioElement extends Old {
    constructor() {
      super(...arguments);

      this.ownerDocument = window.document; // need to set owner document here because HTMLAudioElement can be manually constructed via new Audio()
    }
  })(HTMLAudioElement);
  function createImageBitmap(src, x, y, w, h, options) {
    let image;
    if (src.constructor.name === 'HTMLImageElement') {
      image = src.image;
    } else if (src.constructor.name === 'Blob') {
      image = new Image();
      // console.log('load blob', src.buffer.byteLength);
      try {
        image.load(src.buffer);
      } catch (err) {
        return Promise.reject(new Error('failed to load image'));
      }
    } else {
      return Promise.reject(new Error('invalid arguments. Unknown constructor type: ' + src.constructor.name));
    }

    x = x || 0;
    y = y || 0;
    w = w || image.width;
    h = h || image.height;
    const flipY = !!options && options.imageOrientation === 'flipY';
    const imageBitmap = new ImageBitmap(
      image,
      x,
      y,
      w,
      h,
      flipY,
    );
    return Promise.resolve(imageBitmap);
  }

  const vmo = nativeVm.make();
  const window = vmo.getGlobal();
  window.vm = vmo;

  for (const k in EventEmitter.prototype) {
    window[k] = EventEmitter.prototype[k];
  }
  EventEmitter.call(window);

  window.window = window;
  window.self = window;
  window.parent = parent || window;
  window.top = top || window;

  window.innerWidth = defaultCanvasSize[0];
  window.innerHeight = defaultCanvasSize[1];
  window.devicePixelRatio = 1;
  window.document = null;
  const location = new Location(options.url);
  Object.defineProperty(window, 'location', {
    get() {
      return location;
    },
    set(href) {
      href = href + '';
      location.href = href;
    },
  });
  window.history = new History(location.href);
  window.navigator = {
    userAgent: `MixedReality (Exokit ${version})`,
    platform: os.platform(),
    appCodeName: 'Mozilla',
    appName: 'Netscape',
    appVersion: '5.0',
    mediaDevices: {
      getUserMedia(constraints) {
        if (constraints.audio) {
          return Promise.resolve(new MicrophoneMediaStream());
        } else {
          return Promise.reject(new Error('constraints not met'));
        }
      },
    },
    getVRDisplaysSync() {
      const result = [];
      if (nativeMl && nativeMl.IsPresent()) {
        result.push(_getMlDisplay(window));
      }
      if (nativeVr.VR_IsHmdPresent()) {
        result.push(_getVrDisplay(window));
      }
      return result;
    },
    getVRDisplays() {
      return Promise.resolve(this.getVRDisplaysSync());
    },
    createVRDisplay() {
      const display = new FakeDisplay(window, 2);
      fakeVrDisplays.push(display);
      return display;
    },
    getGamepads: () => localGamepads,
    /* getVRMode: () => vrMode,
    setVRMode: newVrMode => {
      for (let i = 0; i < vrDisplays.length; i++) {
        vrDisplays[i].destroy();
      }

      if (newVrMode === 'vr') {
        vrDisplays = [new VRDisplay(window, 0)];
      } else if (newVrMode === 'ar') {
        display = new ARDisplay(window, 1);
      } else if (newVrMode === 'ml') {
        vrDisplays = [new MLDisplay(window, 2)];
      }
      vrMode = newVrMode;
    },
    getVRTexture: () => vrTexture,
    setVRTexture: newVrTexture => {
      vrTexture = newVrTexture;
    },
    getVRTextures: () => vrTextures,
    setVRTextures: newVrTextures => {
      vrTextures = newVrTextures;
    }, */
  };
  window.destroy = function() {
    this._emit('destroy', {window: this});
  };
  window.localStorage = new LocalStorage(path.join(options.dataPath, '.localStorage'));
  window.URL = URL;
  window.console = console;
  window.setTimeout = setTimeout;
  window.clearTimeout = clearTimeout;
  window.setInterval = setInterval;
  window.clearInterval = clearInterval;
  window.performance = performance;
  window.screen = new Screen(window);
  window.dgram = dgram; // XXX non-standard
  window.scrollTo = function(x = 0, y = 0) {
    this.scrollX = x;
    this.scrollY = y;
  };
  window.scrollX = 0;
  window.scrollY = 0;
  window[htmlTagsSymbol] = {
    DOCUMENT: Document,
    BODY: HTMLBodyElement,
    A: HTMLAnchorElement,
    STYLE: HTMLStyleElement,
    SCRIPT: HTMLScriptElement,
    IMG: HTMLImageElementBound,
    AUDIO: HTMLAudioElementBound,
    VIDEO: HTMLVideoElement,
    SOURCE: HTMLSourceElement,
    IFRAME: HTMLIframeElement,
    CANVAS: HTMLCanvasElement,
    TEMPLATE: HTMLTemplateElement,
  };
  window[optionsSymbol] = options;
  window.DocumentFragment = DocumentFragment;
  window.Element = Element;
  window.HTMLElement = HTMLElement;
  window.HTMLAnchorElement = HTMLAnchorElement;
  window.HTMLStyleElement = HTMLStyleElement;
  window.HTMLScriptElement = HTMLScriptElement;
  window.HTMLImageElement = HTMLImageElementBound,
  window.HTMLAudioElement = HTMLAudioElementBound;
  window.HTMLVideoElement = HTMLVideoElement;
  window.HTMLIframeElement = HTMLIframeElement;
  window.HTMLCanvasElement = HTMLCanvasElement;
  window.HTMLTemplateElement = HTMLTemplateElement;
  window.Node = Node;
  window.Text = Text;
  window.Comment = Comment;
  window.MutationObserver = MutationObserver;
  window.getComputedStyle = el => {
    let styleSpec = el[computedStyleSymbol];
    if (!styleSpec || styleSpec.epoch !== styleEpoch) {
      const style = el.style.clone();
      const styleEls = el.ownerDocument.documentElement.getElementsByTagName('style');
      for (let i = 0; i < styleEls.length; i++) {
        const {stylesheet} = styleEls[i];
        if (stylesheet) {
          const {rules} = stylesheet;
          for (let j = 0; j < rules.length; j++) {
            const rule = rules[j];
            const {selectors} = rule;
            if (selectors && selectors.some(selector => el.matches(selector))) {
              const {declarations} = rule;
              for (let k = 0; k < declarations.length; k++) {
                const {property, value} = declarations[k];
                style[property] = value;
              }
            }
          }
        }
      }
      styleSpec = {
        style,
        styleEpoch,
      };
      el[computedStyleSymbol] = styleSpec;
    }
    return styleSpec.style;
  };
  window.DOMParser = class DOMParser {
    parseFromString(htmlString, type) {
      const _recurse = node => {
        let nodeName = null;
        let value = null;
        if (node.type === 'text') {
          nodeName = '#text';
          value = node.text;
        } else if (node.type === 'comment') {
          nodeName = '#comment';
          value = node.content;
        }

        const tagName = node.name || null;

        const attrs = [];
        if (node.attributes) {
          for (const name in node.attributes) {
            attrs.push({
              name,
              value: node.attributes[name],
            });
          }
        }

        const childNodes = node.children ? node.children.map(childNode => _recurse(childNode)) : [];

        return {
          nodeName,
          tagName,
          attrs,
          value,
          childNodes,
        };
      };
      const xmlAst = parseXml(htmlString, {
        // preserveComments: true,
      });
      const htmlAst = _recurse(xmlAst);
      return _parseDocumentAst(htmlAst, window[optionsSymbol], window, false);
    }
  };
  window.Boolean = Boolean;
  window.Number = Number;
  window.String = String;
  window.Object = Object;
  window.Array = Array;
  window.Symbol = Symbol;
  window.Buffer = Buffer; // XXX non-standard
  window.ArrayBuffer = ArrayBuffer;
  window.Int8Array = Int8Array;
  window.Uint8Array = Uint8Array;
  window.Uint8ClampedArray = Uint8ClampedArray;
  window.Int16Array = Int16Array;
  window.Uint16Array = Uint16Array;
  window.Int32Array = Int32Array;
  window.Uint32Array = Uint32Array;
  window.Float32Array = Float32Array;
  window.Float64Array = Float64Array;
  window.Event = Event;
  window.KeyboardEvent = KeyboardEvent;
  window.MouseEvent = MouseEvent;
  window.WheelEvent = WheelEvent;
  window.MessageEvent = MessageEvent;
  window.CustomEvent = CustomEvent;
  window.addEventListener = Element.prototype.addEventListener.bind(window);
  window.removeEventListener = Element.prototype.removeEventListener.bind(window);
  window.dispatchEvent = Element.prototype.dispatchEvent.bind(window);
  window.Image = HTMLImageElementBound;
  window.ImageData = ImageData;
  window.ImageBitmap = ImageBitmap;
  window.Path2D = Path2D;
  window.CanvasGradient = CanvasGradient;
  window.CanvasRenderingContext2D = CanvasRenderingContext2D;
  window.WebGLRenderingContext = WebGLRenderingContext;
  window.Audio = HTMLAudioElementBound;
  window.MediaRecorder = MediaRecorder;
  window.Document = Document;
  window.DocumentType = DocumentType;
  window.DOMImplementation = DOMImplementation;
  window.DataTransfer = DataTransfer;
  window.DataTransferItem = DataTransferItem;
  window.Screen = Screen;
  window.Gamepad = Gamepad;
  window.VRStageParameters = VRStageParameters;
  window.VRDisplay = VRDisplay;
  window.MLDisplay = MLDisplay;
  window.FakeDisplay = FakeDisplay;
  // window.ARDisplay = ARDisplay;
  window.VRFrameData = VRFrameData;
  window.btoa = btoa;
  window.atob = atob;
  window.TextEncoder = TextEncoder;
  window.TextDecoder = TextDecoder;
  window.fetch = (url, options) => {
    if (typeof url === 'string') {
      const blob = urls.get(url);
      if (blob) {
        return Promise.resolve(new Response(blob));
      } else {
        url = _normalizeUrl(url);
        if (redirectUrls[url]) {
          url = redirectUrls[url];
        }
        return fetch(url, options);
      }
    } else {
      return fetch(url, options);
    }
  };
  window.redirect = (url1, url2) => { // XXX non-standard
    redirectUrls[url1] = url2;
  };
  window.XMLHttpRequest = (Old => class XMLHttpRequest extends Old {
    open() {
      const blob = urls.get(url);
      if (blob) {
        this._properties._responseFn = cb => {
          process.nextTick(() => {
            const {buffer} = blob;
            const response = new stream.PassThrough();
            response.statusCode = 200;
            response.headers = {
              'content-length': buffer.length + '',
            };
            cb(response);
          });
        };
      } else {
        arguments[1] = _normalizeUrl(arguments[1]);
        if (redirectUrls[arguments[1]]) {
          arguments[1] = redirectUrls[arguments[1]];
        }
        const match = arguments[1].match(/^file:\/\/(.*)$/);
        if (match) {
          const p = match[1];
          this._properties._responseFn = cb => {
            fs.lstat(p, (err, stats) => {
              if (!err) {
                const response = fs.createReadStream(p);
                response.statusCode = 200;
                response.headers = {
                  'content-length': stats.size + '',
                };
                cb(response);
              } else if (err.code === 'ENOENT') {
                const response = new stream.PassThrough();
                response.statusCode = 404;
                response.headers = {};
                response.end('file not found: ' + p);
                cb(response);
              } else {
                const response = new stream.PassThrough();
                response.statusCode = 500;
                response.headers = {};
                response.end(err.stack);
                cb(response);
              }
            });
          };
          arguments[1] = 'http://127.0.0.1/'; // needed to pass protocol check, will not be fetched
        }
      }

      return Old.prototype.open.apply(this, arguments);
    }
  })(XMLHttpRequest),
  window.Promise = Promise;
  window.WebSocket = WebSocket;
  window.Request = Request;
  window.Response = Response;
  window.Blob = Blob;
  window.AudioContext = AudioContext;
  window.AudioNode = AudioNode;
  window.AudioDestinationNode = AudioDestinationNode;
  window.AudioParam = AudioParam;
  window.AudioListener = AudioListener;
  window.GainNode = GainNode;
  window.AnalyserNode = AnalyserNode;
  window.PannerNode = PannerNode;
  window.StereoPannerNode = StereoPannerNode;
  window.createImageBitmap = createImageBitmap;
  window.Worker =  class Worker extends nativeWorker {
    constructor(src, workerOptions = {}) {
      workerOptions.baseUrl = options.baseUrl;
      if (nativeBindings) {
        workerOptions.startScript = `
          const bindings = requireNative("nativeBindings");
          global.Image = bindings.nativeImage;
          global.ImageBitmap = bindings.nativeImageBitmap;
          global.createImageBitmap = ${createImageBitmap.toString()};
          const smiggles = require("smiggles");
          smiggles.bind({ImageBitmap: bindings.nativeImageBitmap});
        `;
      }

      if (src instanceof Blob) {
        super('data:application/javascript,' + src.buffer.toString('utf8'), workerOptions);
      } else {
        const blob = urls.get(src);
        const normalizedSrc = blob ?
          'data:application/octet-stream;base64,' + blob.buffer.toString('base64')
        :
          _normalizeUrl(src);
        super(normalizedSrc, workerOptions);
      }
    }
  };
  window.requestAnimationFrame = fn => {
    fn[windowSymbol] = window;
    rafCbs.push(fn);
    return fn;
  };
  window.cancelAnimationFrame = fn => {
    const index = rafCbs.indexOf(fn);
    if (index !== -1) {
      rafCbs.splice(index, 1);
    }
  };
  window.postMessage = function(data) {
    this._emit('message', new MessageEvent(data));
  };
  /*
    Treat function onload() as a special case that disables automatic event attach for onload, because this is how browsers work. E.g.
      <!doctype html><html><head><script>
        function onload() {
          console.log ('onload'); // NOT called; presence of top-level function onload() makes all the difference
        }
        window.onload = onload;
      </script></head></html>
  */
  window[disabledEventsSymbol] = {
    load: undefined,
    error: undefined,
  };
  window._emit = function(type) {
    if (!this[disabledEventsSymbol][type]) {
      Node.prototype._emit.apply(this, arguments);
    }
  };
  Object.defineProperty(window, 'onload', {
    get() {
      return window[disabledEventsSymbol]['load'] !== undefined ? window[disabledEventsSymbol]['load'] : _elementGetter(window, 'load');
    },
    set(onload) {
      if (vmOne.isCompiling()) {
        this[disabledEventsSymbol]['load'] = onload;
      } else {
        if (window[disabledEventsSymbol]['load'] !== undefined) {
          this[disabledEventsSymbol]['load'] = onload;
        } else {
          _elementSetter(window, 'load', onload);
        }
      }
    },
  });
  Object.defineProperty(window, 'onerror', {
    get() {
      return window[disabledEventsSymbol]['error'] !== undefined ? window[disabledEventsSymbol]['error'] : _elementGetter(window, 'error');
    },
    set(onerror) {
      if (vmOne.isCompiling()) {
        window[disabledEventsSymbol]['error'] = onerror;
      } else {
        if (window[disabledEventsSymbol]['error'] !== undefined) {
          window[disabledEventsSymbol]['error'] = onerror;
        } else {
          _elementSetter(window, 'error', onerror);
        }
      }
    },
  });
  Object.defineProperty(window, 'onmessage', {
    get() {
      return _elementGetter(window, 'message');
    },
    set(onmessage) {
      _elementSetter(window, 'message', onmessage);
    },
  });
  Object.defineProperty(window, 'onpopstate', {
    get() {
      return _elementGetter(window, 'popstate');
    },
    set(onpopstate) {
      _elementSetter(window, 'popstate', onpopstate);
    },
  });

  window.on('destroy', e => {
    const {window: destroyedWindow} = e;
    rafCbs = rafCbs.filter(fn => fn[windowSymbol] !== destroyedWindow);
  });
  window.history.on('popstate', (u, state) => {
    window.location.set(u);

    const event = new Event('popstate');
    event.state = state;
    window.dispatchEvent(event);
  });
  let loading = false;
  window.location.on('update', href => {
    if (!loading) {
      exokit.load(href)
        .then(newWindow => {
          window._emit('beforeunload');
          window._emit('unload');
          window._emit('navigate', newWindow);

          rafCbs = rafCbs.filter(fn => fn[windowSymbol] !== window);
        })
        .catch(err => {
          loading = false;
          window._emit('error', {
            error: err,
          });
        });
      loading = true;
    }
  });

  if (!parent) {
    window.tickAnimationFrame = tickAnimationFrame;

    window[mrDisplaysSymbol] = {
      vrDisplay: new VRDisplay(window, 0),
      mlDisplay: new MLDisplay(window, 1),
    };

    window.updateVrFrame = update => {
      window._emit('updatevrframe', update);
    };
    /* window.updateArFrame = (viewMatrix, projectionMatrix) => {
      window._emit('updatearframe', viewMatrix, projectionMatrix);
    }; */
    window.updateMlFrame = update => {
      window._emit('updatemlframe', update);
    };

    window.on('updatevrframe', update => {
      const {gamepads} = update;

      if (gamepads !== undefined) {
        if (gamepads[0]) {
          localGamepads[0] = leftGamepad;
          localGamepads[0].copy(gamepads[0]);
        } else {
          localGamepads[0] = null;
        }
        if (gamepads[1]) {
          localGamepads[1] = rightGamepad;
          localGamepads[1].copy(gamepads[1]);
        } else {
          localGamepads[1] = null;
        }
      }
    });

    window.on('updatemlframe', update => {
      const {gamepads} = update;

      if (gamepads !== undefined) {
        if (gamepads[0]) {
          localGamepads[0] = leftGamepad;
          localGamepads[0].copy(gamepads[0]);
        } else {
          localGamepads[0] = null;
        }
        if (gamepads[1]) {
          localGamepads[1] = rightGamepad;
          localGamepads[1].copy(gamepads[1]);
        } else {
          localGamepads[1] = null;
        }
      }
    });

    if (nativeMl) {
      let lastPresent = nativeMl.IsPresent();

      nativeMl.OnPresentChange(isPresent => {
        if (isPresent && !lastPresent) {
          const e = new Event('vrdisplayconnect');
          e.display = _getMlDisplay(window);
          window.dispatchEvent(e);
        } else if (!isPresent && lastPresent) {
          const e = new Event('vrdisplaydisconnect');
          e.display = _getMlDisplay(window);
          window.dispatchEvent(e);
        }
        lastPresent = isPresent;
      });
    }
  } else {
    top.on('vrdisplaypresentchange', e => {
      window._emit('vrdisplaypresentchange', e);
    });
    /* top.on('updatevrframe', update => { // XXX clean up listeners on window destroy
      window._emit('updatevrframe', update);
    });
    top.on('updatearframe', update => {
      window._emit('updatearframe', update);
    });
    top.on('updatemlframe', update => {
      window._emit('updatemlframe', update);
    }); */
  }
  return window;
};
const _parseDocument = (s, options, window) => {
  const ast = parse5.parse(s, {
    locationInfo: true,
  });
  ast.tagName = 'document';
  return _parseDocumentAst(ast, options, window, true);
};
const _parseDocumentAst = (ast, options, window, uppercase) => {
  const document = _fromAST(ast, window, null, null, uppercase);
  const html = document.childNodes.find(el => el.tagName === 'HTML');
const documentElement = html || (document.childNodes.length > 0 ? document.childNodes[0] : null);
  const head = html ? html.childNodes.find(el => el.tagName === 'HEAD') : null;
  const body = html ? html.childNodes.find(el => el.tagName === 'BODY') : null;

  document.documentElement = documentElement;
  document.readyState = null;
  document.head = head;
  document.body = body;
  document.location = window.location;
  document.createElement = tagName => {
    tagName = tagName.toUpperCase();
    const HTMLElementTemplate = window[htmlTagsSymbol][tagName];
    const element = HTMLElementTemplate ? new HTMLElementTemplate() : new HTMLElement(tagName);
    element.ownerDocument = document;
    return element;
  };
  document.createElementNS = (namespace, tagName) => document.createElement(tagName);
  document.createDocumentFragment = () => {
    const documentFragment = new DocumentFragment();
    documentFragment.ownerDocument = document;
    return documentFragment;
  };
  document.createTextNode = text => new Text(text);
  document.createComment = comment => new Comment(comment);
  document.createEvent = type => {
    switch (type) {
      case 'KeyboardEvent':
      case 'KeyboardEvents':
        return new KeyboardEvent();
      case 'MouseEvent':
      case 'MouseEvents':
        return new MouseEvent();
      case 'Event':
      case 'Events':
      case 'HTMLEvents':
        return new Event();
      default:
        throw new Error('invalid createEvent type: ' + type);
    }
  };
  document.importNode = (el, deep) => el.cloneNode(deep);
  document.scripts = _makeHtmlCollectionProxy(document.documentElement, 'script');
  document.styleSheets = [];
  document.implementation = new DOMImplementation(window);
  document.activeElement = body;
  document.open = () => {
    document.innerHTML = '';
  };
  document.close = () => {};
  document.write = htmlString => {
    const childNodes = parse5.parseFragment(htmlString, {
      locationInfo: true,
    }).childNodes.map(childNode => _fromAST(childNode, window, document.body, document, true));
    for (let i = 0; i < childNodes.length; i++) {
      document.body.appendChild(childNodes[i]);
    }
  };
  document.execCommand = command => {
    if (command === 'copy') {
      // nothing
    } else if (command === 'paste') {
      document.dispatchEvent(new Event('paste'));
    }
  };
  document[pointerLockElementSymbol] = null;

  if (window.top === window) {
    document.addEventListener('pointerlockchange', () => {
      const iframes = document.getElementsByTagName('iframe');
      for (let i = 0; i < iframes.length; i++) {
        const iframe = iframes[i];
        if (iframe.contentDocument) {
          iframe.contentDocument._emit('pointerlockchange');
        }
      }
    });
  }

  process.nextTick(async () => {
    document.readyState = 'complete';

    try {
      await _runHtml(document, window);
    } catch(err) {
      console.warn(err);
    }

    document.dispatchEvent(new Event('readystatechange', {target: document}));
    document.dispatchEvent(new Event('load', {target: document}));
    window.dispatchEvent(new Event('load', {target: document}));
  });

  return document;
};
const _makeWindowWithDocument = (s, options, parent, top) => {
  const window = _makeWindow(options, parent, top);
  window.document = _parseDocument(s, options, window);
  return window;
};

const exokit = (s = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1/';
  options.baseUrl = options.baseUrl || options.url;
  options.dataPath = options.dataPath || __dirname;
  return _makeWindowWithDocument(s, options);
};
exokit.load = (src, options = {}) => fetch(src)
  .then(res => {
    if (res.status >= 200 && res.status < 300) {
      return res.text();
    } else {
      return Promise.reject(new Error('fetch got invalid status code: ' + res.status + ' : ' + src));
    }
  })
  .then(htmlString => {
    const baseUrl = (() => {
      if (options.baseUrl) {
        return options.baseUrl;
      } else {
        if (/^file:\/\/(.*)$/.test(src)) {
          return src;
        } else {
          const parsedUrl = url.parse(src, {
            locationInfo: true,
          });
          return url.format({
            protocol: parsedUrl.protocol || 'http:',
            host: parsedUrl.host || '127.0.0.1',
            pathname: parsedUrl.pathname,
            search: parsedUrl.search,
          });
        }
      }
    })();
    return exokit(htmlString, {
      url: options.url || src,
      baseUrl,
      dataPath: options.dataPath,
    });
  });
exokit.THREE = THREE;
exokit.setArgs = newArgs => {
  args = newArgs;
};
exokit.setVersion = newVersion => {
  version = newVersion;
};
exokit.setNativeBindingsModule = nativeBindingsModule => {
  nativeBindings = true;

  const bindings = require(nativeBindingsModule);

  nativeVm = bindings.nativeVm;
  nativeWorker = bindings.nativeWorker;
  nativeWorker.setNativeRequire('nativeBindings', bindings.initFunctionAddress);
  nativeWorker.bind({
    ImageBitmap: bindings.nativeImageBitmap,
  });

  Image = bindings.nativeImage;
  ImageData = bindings.nativeImageData;
  ImageBitmap = bindings.nativeImageBitmap;
  Path2D = bindings.nativePath2D;
  CanvasGradient = bindings.nativeCanvasGradient;
  CanvasRenderingContext2D = bindings.nativeCanvasRenderingContext2D;
  WebGLRenderingContext = bindings.nativeGl;
  if (args.frame || args.minimalFrame) {
    WebGLRenderingContext = function WebGLRenderingContext() {
      const result = Reflect.construct(bindings.nativeGl, arguments);
      for (const k in result) {
        if (typeof result[k] === 'function') {
          result[k] = (old => function() {
            if (args.frame) {
              console.log(k, arguments);
            } else if (args.minimalFrame) {
              console.log(k);
            }
            return old.apply(this, arguments);
          })(result[k]);
        }
      }
      return result;
    };
  }

  HTMLImageElement = class extends HTMLSrcableElement {
    constructor(attrs = [], value = '') {
      super('IMG', attrs, value);

      this.image = new bindings.nativeImage();
    }

    get src() {
      return this.getAttribute('src');
    }
    set src(src) {
      this.setAttribute('src', src);

      // const srcError = new Error();

      this.ownerDocument.defaultView.fetch(src)
        .then(res => {
          if (res.status >= 200 && res.status < 300) {
            return res.arrayBuffer();
          } else {
            return Promise.reject(new Error(`img src got invalid status code (url: ${JSON.stringify(src)}, code: ${res.status})`));
          }
        })
        .then(arrayBuffer => {
          try {
            this.image.load(arrayBuffer);
          } catch(err) {
            throw new Error(`failed to decode image: ${err.message} (url: ${JSON.stringify(src)}, size: ${arrayBuffer.byteLength})`);
          }
        })
        .then(() => {
          this.dispatchEvent(new Event('load', {target: this}));
        })
        .catch(err => {
          this.dispatchEvent(new Event('error', {target: this}));
        });
    }

    get onload() {
      return _elementGetter(this, 'load');
    }
    set onload(onload) {
      _elementSetter(this, 'load', onload);
    }

    get onerror() {
      return _elementGetter(this, 'error');
    }
    set onerror(onerror) {
      _elementSetter(this, 'error', onerror);
    }

    get width() {
      return this.image.width;
    }
    set width(width) {}
    get height() {
      return this.image.height;
    }
    set height(height) {}

    get naturalWidth() {
      return this.width;
    }
    set naturalWidth(naturalWidth) {}
    get naturalHeight() {
      return this.height;
    }
    set naturalHeight(naturalHeight) {}

    getBoundingClientRect() {
      return new DOMRect(0, 0, this.width, this.height);
    }

    get data() {
      return this.image.data;
    }
    set data(data) {}
  };

  /* const {nativeAudio} = bindings;
  AudioContext = nativeAudio.AudioContext;
  AudioNode = nativeAudio.AudioNode;
  AudioDestinationNode = nativeAudio.AudioDestinationNode;
  AudioParam = nativeAudio.AudioParam;
  AudioListener = nativeAudio.AudioListener;
  GainNode = nativeAudio.GainNode;
  AnalyserNode = nativeAudio.AnalyserNode;
  PannerNode = nativeAudio.PannerNode;
  StereoPannerNode = nativeAudio.StereoPannerNode;
  HTMLAudioElement = class extends HTMLMediaElement {
    constructor(attrs = [], value = '') {
      super('AUDIO', attrs, value);

      this.readyState = HTMLMediaElement.HAVE_NOTHING;
      this.audio = new nativeAudio.Audio();

      this.on('attribute', (name, value) => {
        if (name === 'src') {
          const src = value;
          this.ownerDocument.defaultView.fetch(src)
            .then(res => {
              if (res.status >= 200 && res.status < 300) {
                return res.arrayBuffer();
              } else {
                return Promise.reject(new Error(`audio src got invalid status code (url: ${JSON.stringify(src)}, code: ${res.status})`));
              }
            })
            .then(arrayBuffer => {
              try {
                this.audio.load(arrayBuffer);
              } catch(err) {
                throw new Error(`failed to decode audio: ${err.message} (url: ${JSON.stringify(src)}, size: ${arrayBuffer.byteLength})`);
              }
            })
            .then(() => {
              this.readyState = HTMLMediaElement.HAVE_ENOUGH_DATA;
              this._emit('canplay');
              this._emit('canplaythrough');
            })
            .catch(err => {
              this._emit('error', err);
            });
        }
      });
    }

    play() {
      this.audio.play();
    }
    pause() {
      this.audio.pause();
    }

    get currentTime() {
      return this.audio && this.audio.currentTime;
    }
    set currentTime(currentTime) {
      if (this.audio) {
        this.audio.currentTime = currentTime;
      }
    }

    get duration() {
      return this.audio && this.audio.duration;
    }
    set duration(duration) {
      if (this.audio) {
        this.audio.duration = duration;
      }
    }

    get oncanplay() {
      return _elementGetter(this, 'canplay');
    }
    set oncanplay(oncanplay) {
      _elementSetter(this, 'canplay', oncanplay);
    }

    get oncanplaythrough() {
      return _elementGetter(this, 'canplaythrough');
    }
    set oncanplaythrough(oncanplaythrough) {
      _elementSetter(this, 'canplaythrough', oncanplaythrough);
    }

    get onerror() {
      return _elementGetter(this, 'error');
    }
    set onerror(onerror) {
      _elementSetter(this, 'error', onerror);
    }
  };
  MicrophoneMediaStream = nativeAudio.MicrophoneMediaStream; */

  /* const {nativeVideo} = bindings;
  HTMLVideoElement = class extends HTMLMediaElement {
    constructor(attrs = [], value = '') {
      super('VIDEO', attrs, value);

      this.readyState = HTMLMediaElement.HAVE_NOTHING;
      this.video = new nativeVideo.Video();

      this.on('attribute', (name, value) => {
        if (name === 'src') {
          console.log('video downloading...');
          const src = value;
          this.ownerDocument.defaultView.fetch(src)
            .then(res => {
              console.log('video download res');
              if (res.status >= 200 && res.status < 300) {
                return res.arrayBuffer();
              } else {
                return Promise.reject(new Error(`video src got invalid status code (url: ${JSON.stringify(src)}, code: ${res.status})`));
              }
            })
            .then(arrayBuffer => {
              console.log('video download arraybuffer');
              try {
                this.video.load(arrayBuffer);
              } catch(err) {
                throw new Error(`failed to decode video: ${err.message} (url: ${JSON.stringify(src)}, size: ${arrayBuffer.byteLength})`);
              }
            })
            .then(() => {
              console.log('video download done');
              this.readyState = HTMLMediaElement.HAVE_ENOUGH_DATA;
              this._emit('canplay');
              this._emit('canplaythrough');
            })
            .catch(err => {
              this._emit('error', err);
            });
        } else if (name === 'loop') {
          this.video.loop = !!value || value === '';
        } else if (name === 'autoplay') {
          const autoplay = !!value || value === '';
          if (autoplay) {
            console.log('video set autoplay');
            const canplay = () => {
              console.log('video autoplay play');
              this.video.play();
              _cleanup();
            };
            const error = () => {
              _cleanup();
            };
            const _cleanup = () => {
              this.removeListener('canplay', canplay);
              this.removeListener('error', error);
            };
            this.on('canplay', canplay);
            this.on('error', error);
          }
        }
      });
    }

    get width() {
      return this.video.width;
    }
    set width(width) {}
    get height() {
      return this.video.height;
    }
    set height(height) {}

    get loop() {
      return this.getAttribute('loop');
    }
    set loop(loop) {
      this.setAttribute('loop', loop);
    }

    get autoplay() {
      return this.getAttribute('autoplay');
    }
    set autoplay(autoplay) {
      this.setAttribute('autoplay', autoplay);
    }

    getBoundingClientRect() {
      return new DOMRect(0, 0, this.width, this.height);
    }

    get data() {
      return this.video.data;
    }
    set data(data) {}

    play() {
      this.video.play();
    }
    pause() {
      this.video.pause();
    }

    get currentTime() {
      return this.video && this.video.currentTime;
    }
    set currentTime(currentTime) {
      if (this.video) {
        this.video.currentTime = currentTime;
      }
    }

    get duration() {
      return this.video && this.video.duration;
    }
    set duration(duration) {
      if (this.video) {
        this.video.duration = duration;
      }
    }

    get oncanplay() {
      return _elementGetter(this, 'canplay');
    }
    set oncanplay(oncanplay) {
      _elementSetter(this, 'canplay', oncanplay);
    }

    get oncanplaythrough() {
      return _elementGetter(this, 'canplaythrough');
    }
    set oncanplaythrough(oncanplaythrough) {
      _elementSetter(this, 'canplaythrough', oncanplaythrough);
    }

    get onerror() {
      return _elementGetter(this, 'error');
    }
    set onerror(onerror) {
      _elementSetter(this, 'error', onerror);
    }

    run() {
      let running = false;

      let sources;
      const srcAttr = this.attributes.src;
      if (srcAttr) {
        this._emit('attribute', 'src', srcAttr.value);
        running = true;
      } else if (sources = this.childNodes.filter(childNode => childNode.nodeType === Node.ELEMENT_NODE && childNode.matches('source'))) {
        for (let i = 0; i < sources.length; i++) {
          const source = sources[i];
          const {src} = source;
          if (src) {
            this.src = src;
            running = true;
            break;
          }
        }
      }
      const loopAttr = this.attributes.loop;
      const loopAttrValue = loopAttr && loopAttr.value;
      if (loopAttrValue || loopAttrValue === '') {
        this.loop = loopAttrValue;
      }
      const autoplayAttr = this.attributes.loop;
      const autoplayAttrValue = autoplayAttr && autoplayAttr.value;
      if (autoplayAttrValue || autoplayAttrValue === '') {
        this.autoplay = autoplayAttrValue;
      }

      return running;
    }
  }; */

  nativeVr = bindings.nativeVr;
  nativeMl = bindings.nativeMl;
};
module.exports = exokit;

if (require.main === module) {
  if (process.argv.length === 3) {
    const baseUrl = 'file://' + __dirname + '/';
    const u = new URL(process.argv[2], baseUrl).href;
    exokit.load(u);
  }
}
