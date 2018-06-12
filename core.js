const events = require('events');
const {EventEmitter} = events;
const stream = require('stream');
const path = require('path');
const fs = require('fs');
const url = require('url');
const os = require('os');
const util = require('util');
const {URL} = url;
const {performance} = require('perf_hooks');

const parseIntStrict = require('parse-int');
const parse5 = require('parse5');

const fetch = require('window-fetch');
const {Blob} = fetch;

const {LocalStorage} = require('node-localstorage');
const indexedDB = require('fake-indexeddb');
const createMemoryHistory = require('history/createMemoryHistory').default;
const ClassList = require('window-classlist');
const he = require('he');
he.encode.options.useNamedReferences = true;
const selector = require('window-selector');
const css = require('css');
const {TextEncoder, TextDecoder} = require('window-text-encoding');
const parseXml = require('@rgrove/parse-xml');
const THREE = require('./lib/three-min.js');
const {
  MRDisplay,
  VRDisplay,
  FakeVRDisplay,
  VRFrameData,
  VRPose,
  VRStageParameters,
  Gamepad,
  GamepadButton,
  getGamepads,
  getAllGamepads,
} = require('vr-display')(THREE);

const windowSymbol = Symbol();
const htmlTagsSymbol = Symbol();
const optionsSymbol = Symbol();
const elementSymbol = Symbol();
const computedStyleSymbol = Symbol();
const disabledEventsSymbol = Symbol();
const pointerLockElementSymbol = Symbol();
const prioritySymbol = Symbol();
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

class EventTarget extends EventEmitter {
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

  dispatchEvent(event) {
    event.target = this;

    const _emit = (node, event) => {
      event.currentTarget = this;
      node._emit(event.type, event);
      event.currentTarget = null;
    };
    const _recurse = (node, event) => {
      _emit(node, event);

      if (event.bubbles && node instanceof Document) {
        _emit(node.defaultView, event);
      }

      if (event.bubbles && !event.propagationStopped && node.parentNode) {
        _recurse(node.parentNode, event);
      }
    };
    _recurse(this, event);
  }

  _emit() { // need to call this instead of EventEmitter.prototype.emit because some frameworks override HTMLElement.prototype.emit()
    return EventEmitter.prototype.emit.apply(this, arguments);
  }
}

class Resource extends EventEmitter {
  constructor(value = 0.5, total = 1) {
    super();

    this.value = value;
    this.total = total;
  }

  setProgress(value) {
    this.value = value;

    this.emit('update');
  }
}
class Resources extends EventTarget {
  constructor() {
    super();
    this.resources = [];
  }

  getValue() {
    let value = 0;
    for (let i = 0; i < this.resources.length; i++) {
      value += this.resources[i].value;
    }
    return value;
  }
  getTotal() {
    let total = 0;
    for (let i = 0; i < this.resources.length; i++) {
      total += this.resources[i].total;
    }
    return total;
  }
  getProgress() {
    let value = 0;
    let total = 0;
    for (let i = 0; i < this.resources.length; i++) {
      const resource = this.resources[i];
      value += resource.value;
      total += resource.total;
    }
    return total > 0 ? (value / total) : 1;
  }

  addResource() {
    const resource = new Resource();
    resource.on('update', () => {
      if (resource.value >= resource.total) {
        this.resources.splice(this.resources.indexOf(resource), 1);
      }

      const e = new Event('update');
      e.value = this.getValue();
      e.total = this.getTotal();
      e.progress = this.getProgress();
      this.dispatchEvent(e);
    });

    this.resources.push(resource);

    return resource;
  }
}

class Event {
  constructor(type, init = {}) {
    this.type = type;
    this.target = init.target !== undefined ? init.target : null;
    this.bubbles = init.bubbles !== undefined ? init.bubbles : false;
    this.cancelable = init.cancelable !== undefined ? init.cancelable : false;

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

  initEvent(type = '', bubbles = false, cancelable = false) {
    this.type = type;
    this.bubbles = bubbles;
    this.cancelable = cancelable;
  }
}
class KeyboardEvent extends Event {
  constructor(type, init = {}) {
    init.bubbles = true;
    init.cancelable = true;
    super(type, init);

    KeyboardEvent.prototype.init.call(this, init);
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
    init.bubbles = true;
    init.cancelable = true;
    super(type, init);

    MouseEvent.prototype.init.call(this, init);
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
    init.bubbles = true;
    init.cancelable = true;
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
class DragEvent extends MouseEvent {
  constructor(type, init = {}) {
    super(type, init);

    DragEvent.prototype.init.call(this, init);
  }

  init(init = {}) {
    this.dataTransfer = init.dataTransfer !== undefined ? init.dataTransfer : null;
  }
}
class MessageEvent extends Event {
  constructor(data) {
    super('message');

    this.data = data;
  }
}
class CustomEvent extends Event {
  constructor(type, init = {}) {
    super(type, init);

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
      if (bindings) {
        const [
          _attribute,
          _children,
          _value,
        ] = bindings;
        el.removeListener('attribute', _attribute);
        el.removeListener('children', _children);
        el.removeListener('value', _value);
        this.bindings.delete(el);
      }
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

const handEntrySize = (1 + (5 * 5)) * (3 + 3);
const maxNumPlanes = 32 * 3;
const planeEntrySize = 3 + 4 + 2 + 1;
VRFrameData.nonstandard = {
  init() {
    this.hands = [
      new Float32Array(handEntrySize),
      new Float32Array(handEntrySize),
    ];
    this.planes = new Float32Array(maxNumPlanes * planeEntrySize);
    this.numPlanes = 0;
  },
  copy(frameData) {
    for (let i = 0; i < this.hands.length; i++) {
      this.hands[i].set(frameData.hands[i]);
    }
    this.planes.set(frameData.planes);
    this.numPlanes = frameData.numPlanes;
  },
};
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
Gamepad.nonstandard = {
  init() {
    this.gesture = new GamepadGesture();
  },
  copy(gamepad) {
    this.gesture.copy(gamepad.gesture);
  },
};

class XR extends EventEmitter {
  constructor(window) {
    super();

    this._window = window;
  }
  requestDevice() {
    if (nativeVr.VR_IsHmdPresent()) {
      return Promise.resolve(_getXrDisplay(this._window));
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
class XRDevice {
  constructor() {
    this.session = null; // non-standard
  }
  supportsSession({exclusive = false, outputContext} = {}) {
    return Promise.resolve(null);
  }
  requestSession({exclusive = false, outputContext} = {}) {
    if (outputContext) {
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
    } else {
      return Promise.reject(new Error('outputContext is required'));
    }
  }
  update(update) {
    if (this.session) {
      this.session.update(update);
    }
  }
}
class XRSession extends EventTarget {
  constructor({device = null, exclusive = false, outputContext} = {}) {
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
      const scale = localVector2.set(1, 1, 1);

      for (let i = 0; i < 2; i++) {
        const gamepad = gamepads[i];
        if (gamepad) {
          const inputSource = this._inputSources[i];
          const inputMatrix = localMatrix.compose(
            localVector.fromArray(gamepad.pose.position),
            localQuaternion.fromArray(gamepad.pose.orientation),
            scale
          );
          inputMatrix.toArray(inputSource._pose.pointerMatrix);
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

    this.framebuffer = null;
    this.framebufferWidth = 0;
    this.framebufferHeight = 0;

    const presentSpec = session.device.onrequestpresent ?
      session.device.onrequestpresent([{
        source: context.canvas,
      }])
    :
      {
        width: context.drawingBufferWidth,
        height: context.drawingBufferHeight,
        framebuffer: 0,
      };
    const {width, height, framebuffer} = presentSpec;

    this.framebuffer = {
      id: framebuffer,
    };
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
class XRPresentationFrame {
  constructor(session) {
    this.session = session;
    this.views = [
      new XRView('left'),
      new XRView('right'),
    ];

    this._pose = new XRDevicePose();
  }
  getDevicePose(coordinateSystem) {
    return this._pose;
  }
  getInputPose(inputSource, coordinateSystem) {
    return inputSource._pose;
  }
}
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
  }
}
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
class XRDevicePose {
  constructor() {
    this.poseModelMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
  getViewMatrix(view) {
    return view._viewMatrix;
  }
}
class XRInputSource {
  constructor(handedness = 'left', pointerOrigin = 'hand') {
    this.handedness = handedness;
    this.pointerOrigin = pointerOrigin;

    this._pose = new XRInputPose();
  }
}
class XRInputPose {
  constructor() {
    this.emulatedPosition = false;
    this.pointerMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
    this.gripMatrix = Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
  }
}
class XRInputSourceEvent extends Event {
  constructor(type, init = {}) {
    super(type);

    this.frame = init.frame !== undefined ? init.frame : null;
    this.inputSource = init.inputSource !== undefined ? init.inputSource : null;
  }
}
class XRCoordinateSystem {
  getTransformTo(other) {
    return Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]); // XXX
  }
}
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
class XRStageBoundsPoint {
  constructor(x, z) {
    this.x = x;
    this.z = z;
  }
}

const localVector = new THREE.Vector3();
const localVector2 = new THREE.Vector3();
const localQuaternion = new THREE.Quaternion();
const localMatrix = new THREE.Matrix4();
/* class ARDisplay extends MRDisplay {
  constructor(window) {
    super('AR', window);

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
class MLDisplay extends MRDisplay {
  constructor() {
    super('ML');

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

    this._context = null;
  }

  requestPresent(layers) {
    if (this.onrequestpresent) {
      this.onrequestpresent(layers);
    }

    this.isPresenting = true;

    if (this.onvrdisplaypresentchange) {
      this.onvrdisplaypresentchange();
    }
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

  getGeometry(positions, normals, indices, metrics) {
    if (this._context) {
      return this._context.stageGeometry.getGeometry(positions, normals, indices, metrics);
    } else {
      return 0;
    }
  }

  update(update) {
    this._transformArray.set(update.transformArray);
    this._projectionArray.set(update.projectionArray);
    // this._viewportArray.set(update.viewportArray);
    this._planesArray.set(update.planesArray);
    this._numPlanes = update.numPlanes;

    this._width = update.viewportArray[2] / 2;
    this._height = update.viewportArray[3];

    this._context = update.context;
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
class AudioBufferSourceNode extends AudioNode {}
class OscillatorNode extends AudioNode {}
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

class Node extends EventTarget {
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

  const result = [];
  result.item = i => {
    if (typeof i === 'number') {
      return result[i];
    } else {
      return undefined;
    }
  };
  result.update = () => {
    result.length = 0;

    for (let i = 0; i < el.childNodes.length; i++) {
      const childNode = el.childNodes[i];
      if (childNode instanceof HTMLElement) {
        result.push(childNode);
      }
    }
  };
  result.update();
  return result;
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
  let needsReset = true;
  const _reset = () => {
    needsReset = false;
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
  return new Proxy({}, {
    get(target, key) {
      if (key === 'reset') {
        return _reset;
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
        if (needsReset) {
          _reset();
        }
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

    if (this._children) {
      this._children.update();
    }

    this._emit('children', [childNode], [], this.childNodes[this.childNodes.length - 2] || null, null);
    this.ownerDocument._emit('domchange');

    return childNode;
  }
  removeChild(childNode) {
    const index = this.childNodes.indexOf(childNode);
    if (index !== -1) {
      this.childNodes.splice(index, 1);
      childNode.parentNode = null;

      if (this._children) {
        this._children.update();
      }

      this._emit('children', [], [childNode], this.childNodes[index - 1] || null, this.childNodes[index] || null);
      this.ownerDocument._emit('domchange');

      return childNode;
    } else {
      throw new Error('The node to be removed is not a child of this node.');
    }
  }
  remove() {
    if (this.parentNode !== null) {
      this.parentNode.removeChild(this);
    }
  }
  replaceChild(newChild, oldChild) {
    const index = this.childNodes.indexOf(oldChild);
    if (index !== -1) {
      this.childNodes.splice(index, 1, newChild);
      oldChild.parentNode = null;

      if (this._children) {
        this._children.update();
      }

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

      if (this._children) {
        this._children.update();
      }

      this._emit('children', [childNode], [], this.childNodes[index - 1] || null, this.childNodes[index + 1] || null);
      this.ownerDocument._emit('domchange');
    }
  }
  insertAfter(childNode, nextSibling) {
    const index = this.childNodes.indexOf(nextSibling);
    if (index !== -1) {
      this.childNodes.splice(index + 1, 0, childNode);
      childNode.parentNode = this;

      if (this._children) {
        this._children.update();
      }

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
  closest(s) {
    for (let el = this; el; el = el.parentElement || el.parentNode) {
      if (el.matches(s)) {
        return el;
      }
    }
    return null;
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

    if (this._children) {
      this._children.update();
    }

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

        const resource = this.ownerDocument.resources.addResource();

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
          })
          .finally(() => {
            resource.setProgress(1);
          });
      }
    });
    this.on('innerHTML', innerHTML => {
      if (this.isRunnable()) {
        const window = this.ownerDocument.defaultView;
        _runJavascript(innerHTML, window, window.location.href, this.location.line !== null ? this.location.line - 1 : 0, this.location.col !== null ? this.location.col - 1 : 0);

        this.readyState = 'complete';

        const resource = this.ownerDocument.resources.addResource();

        process.nextTick(() => {
          this.dispatchEvent(new Event('load', {target: this}));

          resource.setProgress(1);
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
class HTMLIFrameElement extends HTMLSrcableElement {
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

        const resource = this.ownerDocument.resources.addResource();

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
              const options = parentWindow[optionsSymbol];

              url = _makeNormalizeUrl(options.baseUrl)(url);
              const contentWindow = _makeWindow({
                url,
                baseUrl: url,
                dataPath: options.dataPath,
              }, parentWindow, parentWindow.top);
              const contentDocument = _parseDocument(htmlString, contentWindow[optionsSymbol], contentWindow);
              contentDocument.hidden = this.hidden;

              contentWindow.document = contentDocument;

              this.contentWindow = contentWindow;
              this.contentDocument = contentDocument;

              contentDocument.on('framebuffer', framebuffer => {
                this._emit('framebuffer', framebuffer);
              });
              const _vrdisplaycheck = e => {
                if (contentDocument.readyState === 'complete') {
                  const newEvent = new Event('vrdisplayactivate');
                  newEvent.display = e.display;
                  contentWindow.dispatchEvent(newEvent);
                }
              };
              parentWindow.top.on('vrdisplaycheck', _vrdisplaycheck);
              contentWindow.on('destroy', e => {
                parentWindow.emit('destroy', e);

                parentWindow.top.removeListener('vrdisplaycheck', _vrdisplaycheck);
              });

              this.dispatchEvent(new Event('load', {target: this}));
            }
          })
          .catch(err => {
            this.dispatchEvent(new Event('load', {target: this}));
          })
          .finally(() => {
            resource.setProgress(1);
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
      if (name === 'width' || name === 'height') {
        if (this._context && this._context.resize) {
          this._context.resize(this.width, this.height);
        }
      }
    });
  }

  get width() {
    return parseInt(this.getAttribute('width') || defaultCanvasSize[0] + '', 10);
  }
  set width(value) {
    if (typeof value === 'number' && isFinite(value)) {
      this.setAttribute('width', value);
    }
  }
  get height() {
    return parseInt(this.getAttribute('height') || defaultCanvasSize[1] + '', 10);
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
      } else if (contextType === 'webgl' || contextType === 'xrpresent') {
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
  constructor({items = [], files = []} = {}) {
    this.items = items;
    this.files = files;
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

  getAsString(callback) {
    const {data} = this;
    setImmediate(() => {
      callback(data);
    });
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
    const {attrs, value, sourceCodeLocation} = node;
    const HTMLElementTemplate = window[htmlTagsSymbol][tagName];
    const location = sourceCodeLocation  ? {
      line: sourceCodeLocation.startLine,
      col: sourceCodeLocation.startCol,
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
function tickAnimationFrame() {
  if (rafCbs.length > 0) {
    // console.log('tick rafs', rafCbs.length)

    tickAnimationFrame.window = this;

    const localRafCbs = rafCbs.slice();

    const _handleRaf = localRafCb => {
      if (rafCbs.includes(localRafCb)) {
        localRafCb(now);

        const index = rafCbs.indexOf(localRafCb);
        if (index !== -1) {
          rafCbs.splice(index, 1);
        }
      }
    };

    const now = performance.now();
    // hidden rafs
    for (let i = 0; i < localRafCbs.length; i++) {
      const localRafCb = localRafCbs[i];
      if (localRafCb[windowSymbol].document.hidden) {
        _handleRaf(localRafCb);
      }
    }
    // visible rafs
    for (let i = 0; i < localRafCbs.length; i++) {
      const localRafCb = localRafCbs[i];
      if (!localRafCb[windowSymbol].document.hidden) {
        _handleRaf(localRafCb);
      }
    }

    tickAnimationFrame.window = null;
  }
}
tickAnimationFrame.window = null;
const _makeRequestAnimationFrame = window => (fn, priority) => {
  if (priority === undefined) {
    priority = 0;
  }
  fn[windowSymbol] = window;
  fn[prioritySymbol] = priority;
  rafCbs.push(fn);
  rafCbs.sort((a, b) => b[prioritySymbol] - a[prioritySymbol]);
  return fn;
};
const _getFakeVrDisplay = window => window[mrDisplaysSymbol].fakeVrDisplay;
const _getVrDisplay = window => window[mrDisplaysSymbol].vrDisplay;
const _getXrDisplay = window => window[mrDisplaysSymbol].xrDisplay;
const _getMlDisplay = window => window[mrDisplaysSymbol].mlDisplay;

function _makeNormalizeUrl(baseUrl) {
  return src => {
    if (!/^[a-z]+:\/\//i.test(src)) {
      src = new URL(src, baseUrl).href
        .replace(/^(file:\/\/)\/([a-z]:.*)$/i, '$1$2');
    }
    return src;
  };
}
const _makeWindow = (options = {}, parent = null, top = null) => {
  const _normalizeUrl = _makeNormalizeUrl(options.baseUrl);

  const HTMLImageElementBound = (Old => class HTMLImageElement extends Old {
    constructor() {
      super(...arguments);

      this.ownerDocument = window.document; // need to set owner document here because HTMLImageElement can be manually constructed via new Image()
    }
  })(HTMLImageElement);
  const HTMLAudioElementBound = (Old => class HTMLAudioElement extends Old {
    constructor(src) {
      if (typeof src === 'string') {
        const audio = new HTMLAudioElementBound();
        audio.setAttribute('src', src);
        return audio;
      } else {
        super(...arguments);

        this.ownerDocument = window.document; // need to set owner document here because HTMLAudioElement can be manually constructed via new Audio()
      }
    }
  })(HTMLAudioElement);
  function createImageBitmap(src, x, y, w, h, options) {
    let image;
    if (src.constructor.name === 'HTMLImageElement') {
      image = src.image;
    } else if (src.constructor.name === 'Blob') {
      image = new Image();
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

  const _makeWindowStartScript = baseUrl => `(() => {
    const fs = require('fs');
    const fetch = require('window-fetch');
    const {Request, Response, Headers, Blob} = fetch;
    const WebSocket = require('ws/lib/websocket');
    const {XMLHttpRequest} = require('window-xhr');
    const XHRUtils = require('window-xhr/lib/utils');
    ${!args.require ? 'global.require = undefined;' : ''}
    process.on('uncaughtException', err => {
      console.warn(err.stack);
    });
    process.on('unhandledRejection', err => {
      console.warn(err.stack);
    });

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

    ${_makeNormalizeUrl.toString()}
    const _normalizeUrl = _makeNormalizeUrl(${JSON.stringify(baseUrl)});

    global.fetch = (url, options) => {
      if (typeof url === 'string') {
        const blob = urls.get(url);
        if (blob) {
          return Promise.resolve(new Response(blob));
        } else {
          const oldUrl = url;
          url = _normalizeUrl(url);
          return fetch(url, options);
        }
      } else {
        return fetch(url, options);
      }
    };
    global.Request = Request;
    global.Response = Response;
    global.Headers = Headers;
    global.Blob = Blob;
    global.WebSocket = WebSocket;
    global.XMLHttpRequest = (Old => class XMLHttpRequest extends Old {
      open(method, url, async, username, password) {
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
          const match = arguments[1].match(/^file:\\/\\/(.*)$/);
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
    })(XMLHttpRequest);
  })();`;

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
        } else if (constraints.video) {
          const dev = new VideoDevice();
          dev.constraints = constraints.video;
          return Promise.resolve(dev);
        } else {
          return Promise.reject(new Error('constraints not met'));
        }
      },
    },
    getVRDisplaysSync() {
      const result = [];
      const fakeVrDisplay = _getFakeVrDisplay(window);
      if (fakeVrDisplay) {
        result.push(fakeVrDisplay);
      }
      if (nativeMl && nativeMl.IsPresent()) {
        result.push(_getMlDisplay(window));
      }
      if (nativeVr.VR_IsHmdPresent()) {
        result.push(_getVrDisplay(window));
      }
      result.sort((a, b) => +b.isPresenting - +a.isPresenting);
      return result;
    },
    getVRDisplays() {
      return Promise.resolve(this.getVRDisplaysSync());
    },
    createVRDisplay() {
      const display = new FakeVRDisplay();
      window[mrDisplaysSymbol].fakeVrDisplay = display;
      return display;
    },
    getGamepads,
    xr: new XR(window),
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
  window.URL = URL;
  window.console = console;
  window.setTimeout = setTimeout;
  window.clearTimeout = clearTimeout;
  window.setInterval = setInterval;
  window.clearInterval = clearInterval;
  window.localStorage = new LocalStorage(path.join(options.dataPath, '.localStorage'));
  window.indexedDB = indexedDB;
  window.performance = performance;
  window.screen = new Screen(window);
  window.urls = urls; // XXX non-standard
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
    IFRAME: HTMLIFrameElement,
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
  window.HTMLIFrameElement = HTMLIFrameElement;
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
  // window.Buffer = Buffer; // XXX non-standard
  window.Event = Event;
  window.KeyboardEvent = KeyboardEvent;
  window.MouseEvent = MouseEvent;
  window.WheelEvent = WheelEvent;
  window.DragEvent = DragEvent;
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
  window.FakeVRDisplay = FakeVRDisplay;
  // window.ARDisplay = ARDisplay;
  window.VRFrameData = VRFrameData;
  window.XR = XR;
  window.XRDevice = XRDevice;
  window.XRSession = XRSession;
  window.XRWebGLLayer = XRWebGLLayer;
  window.XRPresentationFrame = XRPresentationFrame;
  window.XRView = XRView;
  window.XRViewport = XRViewport;
  window.XRDevicePose = XRDevicePose;
  window.XRInputSource = XRInputSource;
  window.XRInputPose = XRInputPose;
  window.XRInputSourceEvent = XRInputSourceEvent;
  window.XRCoordinateSystem = XRCoordinateSystem;
  window.XRFrameOfReference = XRFrameOfReference;
  window.XRStageBounds = XRStageBounds;
  window.XRStageBoundsPoint = XRStageBoundsPoint;
  window.btoa = btoa;
  window.atob = atob;
  window.TextEncoder = TextEncoder;
  window.TextDecoder = TextDecoder;
  window.AudioContext = AudioContext;
  window.AudioNode = AudioNode;
  window.AudioBufferSourceNode = AudioBufferSourceNode;
  window.OscillatorNode = OscillatorNode;
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
          ${_makeWindowStartScript(options.baseUrl)}

          const bindings = requireNative("nativeBindings");
          const smiggles = require("smiggles");

          smiggles.bind({ImageBitmap: bindings.nativeImageBitmap});

          global.Image = bindings.nativeImage;
          global.ImageBitmap = bindings.nativeImageBitmap;
          global.createImageBitmap = ${createImageBitmap.toString()};
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
  window.requestAnimationFrame = _makeRequestAnimationFrame(window);
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
      if (nativeVm.isCompiling()) {
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
      if (nativeVm.isCompiling()) {
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

  vmo.run(_makeWindowStartScript(options.baseUrl), 'window-start-script.js');

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

    const _bindMRDisplay = display => {
      display.onrequestanimationframe = _makeRequestAnimationFrame(window);
      display.oncancelanimationframe = window.cancelAnimationFrame;
      display.onvrdisplaypresentchange = () => {
        process.nextTick(() => {
          const e = new Event('vrdisplaypresentchange');
          e.display = display;
          window.dispatchEvent(e);
        });
      };
    };
    const vrDisplay = new VRDisplay();
    _bindMRDisplay(vrDisplay);
    vrDisplay.onrequestpresent = layers => nativeVr.requestPresent(layers);
    vrDisplay.onexitpresent = () => nativeVr.exitPresent();
    const xrDisplay = new XRDevice();
    xrDisplay.onrequestpresent = layers => nativeVr.requestPresent(layers);
    xrDisplay.onexitpresent = () => nativeVr.exitPresent();
    xrDisplay.onrequestanimationframe = _makeRequestAnimationFrame(window);
    xrDisplay.oncancelanimationframe = window.cancelAnimationFrame;
    xrDisplay.requestSession = (requestSession => function() {
      return requestSession.apply(this, arguments)
        .then(session => {
          vrDisplay.isPresenting = true;
          session.once('end', () => {
            vrDisplay.isPresenting = false;
          });
          return session;
        });
    })(xrDisplay.requestSession);
    const mlDisplay = new MLDisplay();
    _bindMRDisplay(mlDisplay);
    mlDisplay.onrequestpresent = layers => nativeMl.requestPresent(layers);
    mlDisplay.onexitpresent = () => nativeMl.exitPresent();
    window[mrDisplaysSymbol] = {
      fakeVrDisplay: null,
      vrDisplay,
      xrDisplay,
      mlDisplay,
    };

    const _updateGamepads = newGamepads => {
      if (newGamepads !== undefined) {
        const gamepads = getGamepads();
        const allGamepads = getAllGamepads();

        if (newGamepads[0]) {
          gamepads[0] = allGamepads[0];
          gamepads[0].copy(newGamepads[0]);
        } else {
          gamepads[0] = null;
        }
        if (newGamepads[1]) {
          gamepads[1] = allGamepads[1];
          gamepads[1].copy(newGamepads[1]);
        } else {
          gamepads[1] = null;
        }
      }
    };
    window.updateVrFrame = update => {
      let updatedHmd = false;
      if (vrDisplay.isPresenting || update.force) {
        vrDisplay.update(update);
        updatedHmd = true;
      }
      if (xrDisplay.session || update.force) {
        xrDisplay.update(update);
        updatedHmd = true;
      }
      if (updatedHmd) {
        _updateGamepads(update.gamepads);
      }
    };
    /* window.updateArFrame = (viewMatrix, projectionMatrix) => {
      arDisplay.update(viewMatrix, projectionMatrix);
    }; */
    window.updateMlFrame = update => {
      mlDisplay.update(update);
      _updateGamepads(update.gamepads);
    };

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
    window[mrDisplaysSymbol] = top[mrDisplaysSymbol];

    top.on('vrdisplaypresentchange', e => {
      window._emit('vrdisplaypresentchange', e);
    });
  }
  return window;
};
const _parseDocument = (s, options, window) => {
  const ast = parse5.parse(s, {
    sourceCodeLocationInfo: true,
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
  document.readyState = 'loading';
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
  document.resources = new Resources(); // non-standard
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
    const bodyChildNodes = body.childNodes;
    body.childNodes = [];

    try {
      await _runHtml(document.head, window);
    } catch(err) {
      console.warn(err);
    }

    body.childNodes = bodyChildNodes;

    document.dispatchEvent(new Event('DOMContentLoaded', {target: document}));

    try {
      await _runHtml(document.body, window);
    } catch(err) {
      console.warn(err);
    }

    document.readyState = 'interactive';
    document.dispatchEvent(new Event('readystatechange', {target: document}));

    document.readyState = 'complete';
    document.dispatchEvent(new Event('readystatechange', {target: document}));

    document.dispatchEvent(new Event('load', {target: document}));
    window.dispatchEvent(new Event('load', {target: window}));
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
exokit.load = (src, options = {}) => {
  if (src.indexOf('://') === -1) {
    src = 'http://' + src;
  }
  return fetch(src)
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
};
exokit.getAllGamepads = getAllGamepads;
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
    WebGLRenderingContext = (OldWebGLRenderingContext => {
      function WebGLRenderingContext() {
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
      }
      for (const k in OldWebGLRenderingContext) {
        WebGLRenderingContext[k] = OldWebGLRenderingContext[k];
      }
      return WebGLRenderingContext;
    })(WebGLRenderingContext);
  }

  HTMLImageElement = class extends HTMLSrcableElement {
    constructor(attrs = [], value = '') {
      super('IMG', attrs, value);

      this.image = new bindings.nativeImage();

      this.on('attribute', (name, value) => {
        if (name === 'src') {
          const src = value;

          const resource = this.ownerDocument.resources.addResource();

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
              console.warn('failed to load image:', src);
              this.dispatchEvent(new Event('error', {target: this}));
            })
            .finally(() => {
              resource.setProgress(1);
            });
        }
      });
    }

    get src() {
      return this.getAttribute('src');
    }
    set src(src) {
      this.setAttribute('src', src);
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

  const {nativeAudio} = bindings;
  AudioContext = nativeAudio.AudioContext;
  AudioNode = nativeAudio.AudioNode;
  AudioBufferSourceNode = nativeAudio.AudioBufferSourceNode;
  OscillatorNode = nativeAudio.OscillatorNode;
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

          const resource = this.ownerDocument.resources.addResource();

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
              console.warn('failed to load audio:', src);
              this.dispatchEvent(new Event('error', {target: this}));
            })
            .finally(() => {
              resource.setProgress(1);
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
  MicrophoneMediaStream = nativeAudio.MicrophoneMediaStream;

  const {nativeVideo} = bindings;
  Video = nativeVideo.Video;
  VideoDevice = nativeVideo.VideoDevice;
  // Video.getDevices fails after opening a webcam, so in order to
  // open multiple webcams we need to call this once on startup.
  const devices = Video.getDevices();
  HTMLVideoElement = class extends HTMLMediaElement {
    constructor(attrs = [], value = '', location = null) {
      super('VIDEO', attrs, value, location);

      this.readyState = HTMLMediaElement.HAVE_NOTHING;
      this.data = new Uint8Array(0);

      this.on('attribute', (name, value) => {
        if (name === 'src') {
          this.readyState = HTMLMediaElement.HAVE_ENOUGH_DATA;

          if (urls.has(value)) {
            const blob = urls.get(value);
            if (blob instanceof VideoDevice) {
              this.video = blob;
            }
          }

          const resource = this.ownerDocument.resources.addResource();

          process.nextTick(() => {
            this.dispatchEvent(new Event('canplay', {target: this}));
            this.dispatchEvent(new Event('canplaythrough', {target: this}));

            resource.setProgress(1);
          });
        }
      });
    }

    get width() {
      return this.video ? this.video.width : 0;
    }
    set width(width) {}
    get height() {
      return this.video ? this.video.height : 0;
    }
    set height(height) {}

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
      return this.video ? this.video.data : null;
    }
    set data(data) {}

    play() {
      const _getDevice = facingMode => {
        switch (facingMode) {
          case 'user': return devices[0];
          case 'environment': return devices[1];
          case 'left': return devices[2];
          case 'right': return devices[3];
          default: return devices[0];
        }
      }
      const _getName = facingMode => (process.platform === 'darwin' ? '' : 'video=') + _getDevice(facingMode).name;
      const _getOptions = facingMode => {
        if (process.platform === 'darwin') {
          return 'framerate='+_getDevice(facingMode).modes[0].fps;
        } else {
          return null;
        }
      }
      if (this.video) {
        this.video.close();
        this.video.open(
          _getName(this.video.constraints.facingMode),
          _getOptions(this.video.constraints.facingMode)
        );
      }
    }
    pause() {
      if (this.video) {
        this.video.close();
      }
    }
    update() {
      if (this.video) {
        this.video.update();
      }
    }
  }

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
