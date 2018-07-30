const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const fs = require('fs');
const url = require('url');
const child_process = require('child_process');
const os = require('os');
const util = require('util');
const {URL} = url;
const {performance} = require('perf_hooks');
const {XMLHttpRequest: XMLHttpRequestBase, FormData} = require('window-xhr');

const fetch = require('window-fetch');
const {Request, Response, Headers, Blob} = fetch;

const WebSocket = require('ws/lib/websocket');

const {LocalStorage} = require('node-localstorage');
const indexedDB = require('fake-indexeddb');
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

const puppeteer = require('puppeteer');

const BindingsModule = require('./src/bindings');
const {defaultCanvasSize} = require('./src/constants');
const GlobalContext = require('./src/GlobalContext');
const symbols = require('./src/symbols');
const {urls} = require('./src/urls');

GlobalContext.args = {};
GlobalContext.version = '';

// Class imports.
const {_parseDocument, _parseDocumentAst, Document, DocumentFragment, DocumentType,
       DOMImplementation, initDocument} = require('./src/Document');
const DOM = require('./src/DOM');
const {DOMRect, Node, NodeList} = require('./src/DOM');
const {CustomEvent, DragEvent, Event, EventTarget, KeyboardEvent, MessageEvent, MouseEvent,
       WheelEvent} = require('./src/Event');
const {History} = require('./src/History');
const {Location} = require('./src/Location');
const {XMLHttpRequest} = require('./src/Network');
const XR = require('./src/XR');
const utils = require('./src/utils');
const {_elementGetter, _elementSetter} = require('./src/utils');

let browser = null;
const _requestBrowser = async () => {
  if (browser === null) {
    browser = await puppeteer.launch()
  }
  return browser;
};

let nativeBindings = false;

const btoa = s => Buffer.from(s, 'binary').toString('base64');
const atob = s => Buffer.from(s, 'base64').toString('binary');
const parseJson = s => {
  try {
    return JSON.parse(s);
  } catch (err) {
    return null;
  }
};

GlobalContext.styleEpoch = 0;

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
GlobalContext.Resources = Resources;

class CustomElementRegistry {
  constructor(window) {
    this._window = window;

    this.elements = {};
    this.elementPromises = {};
  }

  define(name, constructor, options) {
    name = name.toUpperCase();

    this.elements[name] = constructor;

    this._window.document.traverse(el => {
      if (el.tagName === name) {
        this.upgrade(el, constructor);
      }
    });

    const promises = this.elementPromises[name];
    if (promises) {
      for (let i = 0; i < promises.length; i++) {
        promises[i].accept();
      }
      this.elementPromises[name] = null;
    }
  }
  get(name) {
    name = name.toUpperCase();

    return this.elements[name];
  }
  whenDefined(name) {
    name = name.toUpperCase();

    if (this.elements[name]) {
      return Promise.resolve();
    } else {
      let promises = this.elementPromises[name];
      if (!promises) {
        promises = [];
        this.elementPromises[name] = promises;
      }
      const promise = new Promise((accept, reject) => {
        promise.accept = accept;
        promise.reject = reject;
      });
      promises.push(promise);
      return promise;
    }
  }

  upgrade(el, constructor) {
    el.setProtototypeOf(el, constructor.prototype);
    constructor.call(el);
  }
}

let nativeVm = GlobalContext.nativeVm = null;
let nativeWorker = null;

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
let nativeVr = GlobalContext.nativeVr = null;
let nativeMl = GlobalContext.nativeMl = null;

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

class MediaRecorder extends EventEmitter {
  constructor() {
    super();
  }

  start() {}

  stop() {}

  requestData() {}
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

class FileReader extends EventTarget {
  constructor() {
    super();

    this.result = null;
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

  readAsArrayBuffer(file) {
    this.result = file.buffer.buffer.slice(file.buffer.byteOffset, file.buffer.byteOffset + file.buffer.byteLength);

    process.nextTick(() => {
      this.dispatchEvent(new Event('load', {
        target: this,
      }));
    });
  }

  readAsDataURL(file) {
    this.result = 'data:' + file.type + ';base64,' + file.buffer.toString('base64');

    process.nextTick(() => {
      this.dispatchEvent(new Event('load', {
        target: this,
      }));
    });
  }
}

const _fromAST = (node, window, parentNode, ownerDocument, uppercase) => {
  if (node.nodeName === '#text') {
    const text = new DOM.Text(node.value);
    text.parentNode = parentNode;
    text.ownerDocument = ownerDocument;
    return text;
  } else if (node.nodeName === '#comment') {
    const comment = new DOM.Comment(node.data);
    comment.parentNode = parentNode;
    comment.ownerDocument = ownerDocument;
    return comment;
  } else {
    let {tagName} = node;
    if (tagName && uppercase) {
      tagName = tagName.toUpperCase();
    }
    let {attrs, value, content, childNodes, sourceCodeLocation} = node;
    const HTMLElementTemplate = window[symbols.htmlTagsSymbol][tagName];
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
      new DOM.HTMLElement(
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
    if (content) {
      element.childNodes = new NodeList(
        content.childNodes.map(childNode =>
          _fromAST(childNode, window, element, ownerDocument, uppercase)
        )
      );
    } else if (childNodes) {
      element.childNodes = new NodeList(
        childNodes.map(childNode =>
          _fromAST(childNode, window, element, ownerDocument, uppercase)
        )
      );
    }
    return element;
  }
};
GlobalContext._fromAST = _fromAST;
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

// To "run" the HTML means to walk it and execute behavior on the elements such as <script src="...">.
// Each candidate element exposes a method on runSymbol which returns whether to await the element load or not.
const _runHtml = (element, window) => {
  if (element instanceof DOM.HTMLElement) {
    return element.traverseAsync(async el => {
      const {id} = el;
      if (id) {
        el._emit('attribute', 'id', id);
      }

      if (el[symbols.runSymbol]) {
        const runResult = el[symbols.runSymbol]();
        if (runResult === 'syncLoad') {
          try {
            await _loadPromise(el)
              .catch(err => {
                console.warn(err);
              });
          } catch(err) {
            console.warn(err);
          }
        } else if (runResult === 'asyncLoad') {
          _loadPromise(el)
            .catch(err => {
              console.warn(err);
            });
        }
      }

      if (/\-/.test(el.tagName)) {
        const constructor = window.customElements.get(el.tagName);
        if (constructor) {
          window.customElements.upgrade(el, constructor);
        }
      }
    });
  } else {
    return Promise.resolve();
  }
};
GlobalContext._runHtml = _runHtml;

let rafCbs = [];
let timeouts = [];
let intervals = [];
let rafIndex = 0;
const localCbs = [];
const _cacheLocalCbs = cbs => {
  for (let i = 0; i < cbs.length; i++) {
    localCbs[i] = cbs[i];
  }
  for (let i = cbs.length; i < localCbs.length; i++) {
    localCbs[i] = null;
  }
};
const _clearLocalCbs = () => {
  for (let i = 0; i < localCbs.length; i++) {
    localCbs[i] = null;
  }
};
function tickAnimationFrame() {
  if (rafCbs.length > 0) {
    _cacheLocalCbs(rafCbs);
    const dateNow = Date.now();

    tickAnimationFrame.window = this;

    const performanceNow = performance.now();

    // hidden rafs
    for (let i = 0; i < localCbs.length; i++) {
      const rafCb = localCbs[i];
      if (rafCb && rafCb[symbols.windowSymbol].document.hidden) {
        rafCb(performanceNow);

        const index = rafCbs.indexOf(rafCb); // could have changed due to sorting
        if (index !== -1) {
          rafCbs[index] = null;
        }
      }
    }
    // visible rafs
    for (let i = 0; i < localCbs.length; i++) {
      const rafCb = localCbs[i];
      if (rafCb && !rafCb[symbols.windowSymbol].document.hidden) {
        rafCb(performanceNow);

        const index = rafCbs.indexOf(rafCb); // could have changed due to sorting
        if (index !== -1) {
          rafCbs[index] = null;
        }
      }
    }

    tickAnimationFrame.window = null;
    if (timeouts.length > 0) {
      _cacheLocalCbs(timeouts);
      for (let i = 0; i < localCbs.length; i++) {
        const timeout = localCbs[i];
        if (timeout) {
          const endTime = timeout[symbols.startTimeSymbol] + timeout[symbols.timeoutSymbol];
          if (dateNow >= endTime) {
            timeout();
            timeouts[i] = null;
          }
        }
      }
    }
    if (intervals.length > 0) {
      _cacheLocalCbs(intervals);
      for (let i = 0; i < localCbs.length; i++) {
        const interval = localCbs[i];
        if (interval) {
          const endTime = interval[symbols.startTimeSymbol] + interval[symbols.intervalSymbol];
          if (dateNow >= endTime) {
            interval();
            interval[symbols.startTimeSymbol] = dateNow;
          }
        }
      }
    }
  }

  _clearLocalCbs(); // release garbage
}
tickAnimationFrame.window = null;
const _findFreeSlot = a => {
  let i;
  for (i = 0; i < a.length; i++) {
    if (a[i] === null) {
      break;
    }
  }
  return i;
};
const _makeRequestAnimationFrame = window => (fn, priority = 0) => {
  fn = fn.bind(window);
  fn[symbols.windowSymbol] = window;
  fn[symbols.prioritySymbol] = priority;
  const id = ++rafIndex;
  fn[symbols.idSymbol] = id;
  rafCbs[_findFreeSlot(rafCbs)] = fn;
  rafCbs.sort((a, b) => (b ? b[symbols.prioritySymbol] : 0) - (a ? a[symbols.prioritySymbol] : 0));
  return id;
};
const _getFakeVrDisplay = window => {
  const {fakeVrDisplay} = window[symbols.mrDisplaysSymbol];
  return fakeVrDisplay.isActive ? fakeVrDisplay : null;
};
const _getVrDisplay = window => window[symbols.mrDisplaysSymbol].vrDisplay;
const _getMlDisplay = window => window[symbols.mrDisplaysSymbol].mlDisplay;
const _cloneMrDisplays = (mrDisplays, window) => {
  const result = {};
  for (const k in mrDisplays) {
    const mrDisplayClone = mrDisplays[k].clone();
    mrDisplayClone.onrequestanimationframe = _makeRequestAnimationFrame(window);
    result[k] = mrDisplayClone;
  }
  return result;
};

const _makeWindow = (options = {}, parent = null, top = null) => {
  const _normalizeUrl = utils._makeNormalizeUrl(options.baseUrl);

  const HTMLImageElementBound = (Old => class HTMLImageElement extends Old {
    constructor() {
      super(...arguments);

      // need to set owner document here because HTMLImageElement can be manually constructed via new Image()
      this.ownerDocument = window.document;
    }
  })(DOM.HTMLImageElement);

  const HTMLAudioElementBound = (Old => class HTMLAudioElement extends Old {
    constructor(src) {
      if (typeof src === 'string') {
        const audio = new HTMLAudioElementBound();
        audio.setAttribute('src', src);
        return audio;
      } else {
        super(...arguments);

        // need to set owner document here because HTMLAudioElement can be manually constructed via new Audio()
        this.ownerDocument = window.document;
      }
    }
  })(DOM.HTMLAudioElement);

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

  const windowStartScript = `(() => {
    ${!GlobalContext.args.require ? 'global.require = undefined;' : ''}

    const _logStack = err => {
      console.warn(err);
    };
    process.on('uncaughtException', _logStack);
    process.on('unhandledRejection', _logStack);

    global.process = undefined;
    global.setImmediate = undefined;
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
    userAgent: `MixedReality (Exokit ${GlobalContext.version})`,
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
    createVRDisplay() {
      const {fakeVrDisplay} = window[symbols.mrDisplaysSymbol];
      fakeVrDisplay.isActive = true;
      return fakeVrDisplay;
    },
    getGamepads,
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

  // WebVR enabled.
  if (['all', 'webvr'].includes(GlobalContext.args.xr)) {
    window.navigator.getVRDisplays = function() {
      return Promise.resolve(this.getVRDisplaysSync());
    }
  }

  // WebXR enabled.
  if (['all', 'webxr'].includes(GlobalContext.args.xr)) {
    window.navigator.xr = new XR.XR(window);
  }

  window.destroy = function() {
    this._emit('destroy', {window: this});
  };
  window.URL = URL;
  window.console = console;
  window.setTimeout = setTimeout;
  window.clearTimeout = clearTimeout;
  window.setInterval = setInterval;
  window.clearInterval = clearInterval;
  window.fetch = (url, options) => {
    const _boundFetch = (url, options) => fetch(url, options)
      .then(res => {
        res.arrayBuffer = (arrayBuffer => function() {
          return arrayBuffer.apply(this, arguments)
            .then(ab => utils._normalizeBuffer(ab, window));
        })(res.arrayBuffer);
        return res;
      });

    if (typeof url === 'string') {
      const blob = urls.get(url);
      if (blob) {
        return Promise.resolve(new Response(blob));
      } else {
        const oldUrl = url;
        url = _normalizeUrl(url);
        return _boundFetch(url, options);
      }
    } else {
      return _boundFetch(url, options);
    }
  };
  window.Request = Request;
  window.Response = (Old => class Response extends Old {
    constructor(body, opts) {
      super(utils._normalizeBuffer(body, global), opts);
    }
  })(Response);
  window.Headers = Headers;
  window.Blob = (Old => class Blob extends Old {
    constructor(parts, opts) {
      super(parts && parts.map(part => utils._normalizeBuffer(part, global)), opts);
    }
  })(Blob);
  window.FormData = FormData;
  window.XMLHttpRequest = (Old => {
    class XMLHttpRequest extends Old {
      open(method, url, async, username, password) {
        url = _normalizeUrl(url);
        return super.open(method, url, async, username, password);
      }
      get response() {
        return utils._normalizeBuffer(super.response, window);
      }
    }
    for (const k in XMLHttpRequestBase) {
      XMLHttpRequest[k] = XMLHttpRequestBase[k];
    }
    return XMLHttpRequest;
  })(XMLHttpRequest);
  window.WebSocket = (Old => {
    class WebSocket extends Old {
      emit(type, event) {
        if (type === 'message') {
          event = utils._normalizeBuffer(event, window);
        }
        return super.emit.apply(this, arguments);
      }
      send(data) {
        return super.send(utils._normalizeBuffer(data, global));
      }
    }
    for (const k in Old) {
      WebSocket[k] = Old[k];
    }
    return WebSocket;
  })(WebSocket);
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
  window[symbols.htmlTagsSymbol] = {
    DOCUMENT: Document,
    BODY: DOM.HTMLBodyElement,
    A: DOM.HTMLAnchorElement,
    STYLE: DOM.HTMLStyleElement,
    SCRIPT: DOM.HTMLScriptElement,
    LINK: DOM.HTMLLinkElement,
    IMG: HTMLImageElementBound,
    AUDIO: HTMLAudioElementBound,
    VIDEO: DOM.HTMLVideoElement,
    SOURCE: DOM.HTMLSourceElement,
    IFRAME: DOM.HTMLIFrameElement,
    CANVAS: DOM.HTMLCanvasElement,
    TEMPLATE: DOM.HTMLTemplateElement,
  };
  window[symbols.optionsSymbol] = options;
  window.DocumentFragment = DocumentFragment;

  // DOM.
  window.Element = DOM.Element;
  window.HTMLElement = DOM.HTMLElement;
  window.HTMLAnchorElement = DOM.HTMLAnchorElement;
  window.HTMLStyleElement = DOM.HTMLStyleElement;
  window.HTMLLinkElement = DOM.HTMLLinkElement;
  window.HTMLScriptElement = DOM.HTMLScriptElement;
  window.HTMLImageElement = HTMLImageElementBound,
  window.HTMLAudioElement = HTMLAudioElementBound;
  window.HTMLVideoElement = DOM.HTMLVideoElement;
  window.SVGElement = DOM.SVGElement;
  window.HTMLIFrameElement = DOM.HTMLIFrameElement;
  window.HTMLCanvasElement = DOM.HTMLCanvasElement;
  window.HTMLTemplateElement = DOM.HTMLTemplateElement;
  window.Node = Node;
  window.Text = DOM.Text;
  window.Comment = DOM.Comment;
  window.NodeList = NodeList;
  window.HTMLCollection = DOM.HTMLCollection;

  window.customElements = new CustomElementRegistry(window);
  window.CustomElementRegistry = CustomElementRegistry;
  window.MutationObserver = require('./src/MutationObserver').MutationObserver;
  window.DOMRect = DOMRect;
  window.getComputedStyle = el => {
    let styleSpec = el[symbols.computedStyleSymbol];
    if (!styleSpec || styleSpec.epoch !== GlobalContext.styleEpoch) {
      const style = el.style.clone();
      const stylesheetEls = el.ownerDocument.documentElement.getElementsByTagName('style')
        .concat(el.ownerDocument.documentElement.getElementsByTagName('link'));
      for (let i = 0; i < stylesheetEls.length; i++) {
        const {stylesheet} = stylesheetEls[i];
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
        styleEpoch: GlobalContext.styleEpoch,
      };
      el[symbols.computedStyleSymbol] = styleSpec;
    }
    return styleSpec.style;
  };
  window.requestPage = async (src, w = 1280, h = 1024) => {
    const browser = await _requestBrowser();
    const page = await browser.newPage();
    await page.setViewport({
      width: w,
      height: h,
    });
    src = new URL(src, options.baseUrl).href;
    await page.goto(src, {
      waitUntil: 'load',
    });
    /* await page.evaluate(async () => {
      const _eval = s => {
        try {
          eval(s);
        } catch (err) {
          console.warn(err);
        }
      };

      const scripts = Array.from(document.querySelectorAll('script'));
      await Promise.all(scripts.map(async script => {
        if (script.src) {
          const text = await fetch(script.src)
            .then(res => {
              if (res.ok) {
                return res.text();
              } else {
                return Promise.reject(new Error('script src got invalid status code: ' + res.status));
              }
            });
          _eval(text);
        } else {
          _eval(script.innerHTML);
        }
      }));
    }); */

    page.requestImage = async () => {
      const b = await page.screenshot({
        omitBackground: true,
      });
      const img = await new Promise((accept, reject) => {
        const blob = new window.Blob([b], {
          type: 'image/png',
        });
        const img = new window.Image();
        const u = URL.createObjectURL(blob);
        const _cleanup = () => {
          URL.revokeObjectURL(u);
        };
        img.onload = () => {
          _cleanup();

          accept(img);
        };
        img.onerror = err => {
          _cleanup();

          reject(err);
        };
        img.src = u;
      });
      return img;
    };

    /* page.on('pagerror', err => {
      console.warn(err);
    });
    page.on('error', err => {
      console.warn(err);
    });

    let metrics = [];
    const _log = args => {
      console.log(args);
    };
    page.on('console', msg => {
      const args = msg.args();
      if (args.length > 0) {
        const arg = args[0];
        const o = arg._remoteObject;
        if (o && o.type === 'string') {
          const j = parseJson(o.value);
          if (j !== null && j.metrics) {
            metrics = j.metrics;
          } else {
            _log(args);
          }
        } else {
          _log(args);
        }
      }
    }); */

    /* await page.addScriptTag({
      content: `
        function render() {
          const metrics = [].slice.call(document.querySelectorAll('a, button, input')).map(function(a) {
            const r = a.getBoundingClientRect();
            return {
              tagName: a.tagName || null,
              className: a.className || null,
              id: a.id || null,
              href: a.getAttribute('href') || null,
              click: a.getAttribute('click') || null,
              left: r.left,
              right: r.right,
              top: r.top,
              bottom: r.bottom,
              width: r.width,
              height: r.height
            };
          });
          console.warn(JSON.stringify({
            metrics,
          }));
        };
        if (document.readyState === 'complete') {
          render();
        }
        document.addEventListener('readystatechange', function() {
          if (document.readyState === 'complete') {
            render();
          }
        });
      `,
    }); */

   /*  return {
      img,
      metrics,
    }; */
    return page;
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
      return _parseDocumentAst(htmlAst, window, false);
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
  window.addEventListener = EventTarget.prototype.addEventListener.bind(window);
  window.removeEventListener = EventTarget.prototype.removeEventListener.bind(window);
  window.dispatchEvent = EventTarget.prototype.dispatchEvent.bind(window);
  window.Image = HTMLImageElementBound;
  window.ImageData = ImageData;
  window.ImageBitmap = ImageBitmap;
  window.Path2D = Path2D;
  window.CanvasGradient = CanvasGradient;
  window.CanvasRenderingContext2D = CanvasRenderingContext2D;
  window.WebGLRenderingContext = WebGLRenderingContext;
  if (GlobalContext.args.webgl !== '1') {
    window.WebGL2RenderingContext = WebGL2RenderingContext;
  }
  window.Audio = HTMLAudioElementBound;
  window.MediaRecorder = MediaRecorder;
  window.Document = Document;
  window.DocumentType = DocumentType;
  window.DOMImplementation = DOMImplementation;
  window.DataTransfer = DataTransfer;
  window.DataTransferItem = DataTransferItem;
  window.FileReader = FileReader;
  window.Screen = Screen;
  window.Gamepad = Gamepad;
  window.VRStageParameters = VRStageParameters;
  window.VRDisplay = VRDisplay;
  window.MLDisplay = MLDisplay;
  window.FakeVRDisplay = FakeVRDisplay;
  // window.ARDisplay = ARDisplay;
  window.VRFrameData = VRFrameData;
  window.XR = XR.XR;
  window.XRDevice = XR.XRDevice;
  window.XRSession = XR.XRSession;
  window.XRWebGLLayer = XR.XRWebGLLayer;
  window.XRPresentationFrame = XR.XRPresentationFrame;
  window.XRView = XR.XRView;
  window.XRViewport = XR.XRViewport;
  window.XRDevicePose = XR.XRDevicePose;
  window.XRInputSource = XR.XRInputSource;
  window.XRInputPose = XR.XRInputPose;
  window.XRInputSourceEvent = XR.XRInputSourceEvent;
  window.XRCoordinateSystem = XR.XRCoordinateSystem;
  window.XRFrameOfReference = XR.XRFrameOfReference;
  window.XRStageBounds = XR.XRStageBounds;
  window.XRStageBoundsPoint = XR.XRStageBoundsPoint;
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
      if (nativeBindings) {
        workerOptions.startScript = `
          (() => {
            ${windowStartScript}

            const bindings = requireNative("nativeBindings");
            const smiggles = require("smiggles");

            smiggles.bind({ImageBitmap: bindings.nativeImageBitmap});

            global.Image = bindings.nativeImage;
            global.ImageBitmap = bindings.nativeImageBitmap;
            global.createImageBitmap = ${createImageBitmap.toString()};
          })();
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
  window.cancelAnimationFrame = id => {
    const index = rafCbs.findIndex(r => r[symbols.idSymbol] === id);
    if (index !== -1) {
      rafCbs[index] = null;
    }
  };
  window.postMessage = function(data) {
    setImmediate(() => {
      window._emit('message', new MessageEvent(data));
    });
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
  window[symbols.disabledEventsSymbol] = {
    load: undefined,
    error: undefined,
  };
  window._emit = function(type) {
    if (!this[symbols.disabledEventsSymbol][type]) {
      Node.prototype._emit.apply(this, arguments);
    }
  };
  Object.defineProperty(window, 'onload', {
    get() {
      return window[symbols.disabledEventsSymbol]['load'] !== undefined ? window[symbols.disabledEventsSymbol]['load'] : _elementGetter(window, 'load');
    },
    set(onload) {
      if (nativeVm.isCompiling()) {
        this[symbols.disabledEventsSymbol]['load'] = onload;
      } else {
        if (window[symbols.disabledEventsSymbol]['load'] !== undefined) {
          this[symbols.disabledEventsSymbol]['load'] = onload;
        } else {
          _elementSetter(window, 'load', onload);
        }
      }
    },
  });
  Object.defineProperty(window, 'onerror', {
    get() {
      return window[symbols.disabledEventsSymbol]['error'] !== undefined ? window[symbols.disabledEventsSymbol]['error'] : _elementGetter(window, 'error');
    },
    set(onerror) {
      if (nativeVm.isCompiling()) {
        window[symbols.disabledEventsSymbol]['error'] = onerror;
      } else {
        if (window[symbols.disabledEventsSymbol]['error'] !== undefined) {
          window[symbols.disabledEventsSymbol]['error'] = onerror;
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

  vmo.run(windowStartScript, 'window-start-script.js');

  window.on('destroy', e => {
    const {window: destroyedWindow} = e;

    const _pred = fn => fn && fn[symbols.windowSymbol] !== destroyedWindow;
    for (let i = 0; i < rafCbs.length; i++) {
      if (!_pred(rafCbs[i])) {
        rafCbs[i] = null;
      }
    }
    for (let i = 0; i < timeouts.length; i++) {
      if (!_pred(timeouts[i])) {
        timeouts[i] = null;
      }
    }
    for (let i = 0; i < intervals.length; i++) {
      if (!_pred(intervals[i])) {
        intervals[i] = null;
      }
    }
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
      exokit.load(href, {
        dataPath: options.dataPath,
      })
        .then(newWindow => {
          window._emit('beforeunload');
          window._emit('unload');
          window._emit('navigate', newWindow);

          const _pred = fn => fn && fn[symbols.windowSymbol] !== window;
          for (let i = 0; i < rafCbs.length; i++) {
            if (!_pred(rafCbs[i])) {
              rafCbs[i] = null;
            }
          }
          for (let i = 0; i < timeouts.length; i++) {
            if (!_pred(timeouts[i])) {
              timeouts[i] = null;
            }
          }
          for (let i = 0; i < intervals.length; i++) {
            if (!_pred(intervals[i])) {
              intervals[i] = null;
            }
          }
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
    const fakeVrDisplay = new FakeVRDisplay();
    fakeVrDisplay.isActive = false;
    const vrDisplay = new VRDisplay();
    _bindMRDisplay(vrDisplay);
    vrDisplay.onrequestpresent = layers => nativeVr.requestPresent(layers);
    vrDisplay.onexitpresent = () => nativeVr.exitPresent();
    const xrDisplay = new XR.XRDevice();
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
    window[symbols.mrDisplaysSymbol] = {
      fakeVrDisplay,
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
    window[symbols.mrDisplaysSymbol] = _cloneMrDisplays(top[symbols.mrDisplaysSymbol], window);

    top.on('vrdisplaypresentchange', e => {
      window._emit('vrdisplaypresentchange', e);
    });
  }
  return window;
};
GlobalContext._makeWindow = _makeWindow;

const _makeWindowWithDocument = (s, options, parent, top) => {
  const window = _makeWindow(options, parent, top);
  window.document = _parseDocument(s, window);
  return window;
};

const exokit = (s = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1/';
  options.baseUrl = options.baseUrl || options.url;
  options.dataPath = options.dataPath || __dirname;
  return _makeWindowWithDocument(s, options);
};
exokit.load = (src, options = {}) => {
  if (!url.parse(src).protocol) {
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
      let baseUrl;
      if (options.baseUrl) {
        baseUrl = options.baseUrl;
      } else {
        baseUrl = utils._getBaseUrl(src);
      }

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
  GlobalContext.args = newArgs;
};
exokit.setVersion = newVersion => {
  GlobalContext.version = newVersion;
};

/**
 * Initialize classes and modules that require native bindings.
 * Required before creating any windows or documents.
 * Set rather than `require`d directly due to way `require` works with multithreading
 * (for `Worker`), use this route to make sure binaries only get initialized once.
 *
 * @param {string} nativeBindingsModule - Path to native bindings JS module.
 */
exokit.setNativeBindingsModule = nativeBindingsModule => {
  nativeBindings = true;

  const bindings = require(nativeBindingsModule);

  // Set in binding module to be referenced from other modules.
  for (const key in bindings) { BindingsModule[key] = bindings[key]; }

  nativeVm = GlobalContext.nativeVm = bindings.nativeVm;
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
  CanvasRenderingContext2D = GlobalContext.CanvasRenderingContext2D = bindings.nativeCanvasRenderingContext2D;
  WebGLRenderingContext = GlobalContext.WebGLRenderingContext = bindings.nativeGl;
  WebGL2RenderingContext = GlobalContext.WebGL2RenderingContext = bindings.nativeGl2;
  if (GlobalContext.args.frame || GlobalContext.args.minimalFrame) {
    WebGLRenderingContext = GlobalContext.WebGLRenderingContext = (OldWebGLRenderingContext => {
      function WebGLRenderingContext() {
        const result = Reflect.construct(bindings.nativeGl, arguments);
        for (const k in result) {
          if (typeof result[k] === 'function') {
            result[k] = (old => function() {
              if (GlobalContext.args.frame) {
                console.log(k, arguments);
              } else if (GlobalContext.args.minimalFrame) {
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

  const {nativeAudio} = bindings;
  AudioContext = class AudioContext extends nativeAudio.AudioContext {
    /**
     * Wrap AudioContext.DecodeAudioDataSync binding with promises and callbacks.
     */
    decodeAudioData(arrayBuffer, successCallback, errorCallback) {
      return new Promise((resolve, reject) => {
        try {
          let audioBuffer = this._decodeAudioDataSync(arrayBuffer);
          if (successCallback) {
            process.nextTick(() => {
              try {
                successCallback(audioBuffer);
              } catch(err) {
                console.warn(err);
              }
            });
          }
          resolve(audioBuffer);
        } catch(err) {
          console.warn(err);
          if (errorCallback) {
            process.nextTick(() => {
              try {
                errorCallback(err);
              } catch(err) {
                console.warn(err);
              }
            });
          }
          reject(err);
        }
      });
    }
  };
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

  MicrophoneMediaStream = nativeAudio.MicrophoneMediaStream;

  const {nativeVideo} = bindings;
  Video = nativeVideo.Video;
  VideoDevice = nativeVideo.VideoDevice;
  // Video.getDevices fails after opening a webcam, so in order to
  // open multiple webcams we need to call this once on startup.
  const devices = Video.getDevices();

  nativeVr = GlobalContext.nativeVr = bindings.nativeVr;
  nativeMl = GlobalContext.nativeMl = bindings.nativeMl;
};
module.exports = exokit;

if (require.main === module) {
  if (process.argv.length === 3) {
    const baseUrl = 'file://' + __dirname + '/';
    const u = new URL(process.argv[2], baseUrl).href;
    exokit.load(u);
  }
}
