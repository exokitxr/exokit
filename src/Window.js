const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const fs = require('fs');
const url = require('url');
const http = require('http');
const https = require('https');
const ws = require('ws');
const os = require('os');
const util = require('util');
const {URL} = url;
const {TextEncoder, TextDecoder} = util;
const {performance} = require('perf_hooks');
const {XMLHttpRequest: XMLHttpRequestBase, FormData} = require('window-xhr');

const fetch = require('window-fetch');
const {Request, Response, Headers, Blob} = fetch;

const WebSocket = require('ws/lib/websocket');

const {LocalStorage} = require('node-localstorage');
const indexedDB = require('fake-indexeddb');
const parseXml = require('@rgrove/parse-xml');
const THREE = require('../lib/three-min.js');
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

const BindingsModule = require('./bindings');
const {defaultCanvasSize} = require('./constants');
const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');
const {urls} = require('./urls');

// Class imports.
const {Document, DocumentFragment, DocumentType, DOMImplementation, initDocument} = require('./Document');
const DOM = require('./DOM');
const {DOMRect, Node, NodeList} = require('./DOM');
const {CustomEvent, DragEvent, ErrorEvent, Event, EventTarget, KeyboardEvent, MessageEvent, MouseEvent, WheelEvent, PromiseRejectionEvent} = require('./Event');
const {History} = require('./History');
const {Location} = require('./Location');
const {XMLHttpRequest} = require('./Network');
const XR = require('./XR');
const utils = require('./utils');
const {_elementGetter, _elementSetter} = require('./utils');

const btoa = s => Buffer.from(s, 'binary').toString('base64');
const atob = s => Buffer.from(s, 'base64').toString('binary');

GlobalContext.styleEpoch = 0;

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
    Object.setPrototypeOf(el, constructor.prototype);
    constructor.call(el);
  }
}

let nativeVm = GlobalContext.nativeVm = null;
let nativeWorker = null;

class MonitorManager {
  getList() {
    return nativeWindow.getMonitors();
  }

  select(index) {
    nativeWindow.setMonitor(index);
  }
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
let nativeVr = GlobalContext.nativeVr = null;
let nativeMl = GlobalContext.nativeMl = null;
let nativeWindow = null;

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

class MLDisplay extends MRDisplay {
  constructor() {
    super('ML');

    this._frameData = new VRFrameData();
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

  getFrameData(frameData) {
    frameData.copy(this._frameData);

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
      handsArray,
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
    if (update.planesArray !== undefined) {
      this._planesArray.set(update.planesArray);
    }
    if (update.numPlanes !== undefined) {
      this._numPlanes = update.numPlanes;
    }
    if (update.context !== undefined) {
      this._context = update.context;
    }
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

  readAsText(file) {
    this.result = file.buffer.toString('utf8');

    process.nextTick(() => {
      this.dispatchEvent(new Event('load', {
        target: this,
      }));
    });
  }
}

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

    tickAnimationFrame.window = this;

    const performanceNow = performance.now();

    // hidden rafs
    for (let i = 0; i < localCbs.length; i++) {
      const rafCb = localCbs[i];
      if (rafCb && rafCb[symbols.windowSymbol].document.hidden) {
        try {
          rafCb(performanceNow);
        } catch (e) {
          console.warn(e);
        }

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
        try {
          rafCb(performanceNow);
        } catch (e) {
          console.warn(e);
        }
        const index = rafCbs.indexOf(rafCb); // could have changed due to sorting
        if (index !== -1) {
          rafCbs[index] = null;
        }
      }
    }

    tickAnimationFrame.window = null;
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
    mrDisplayClone.window = window;
    result[k] = mrDisplayClone;
  }
  return result;
};

const _makeWindowVm = (htmlString = '', options = {}) => {
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

    if (typeof x === 'object') {
      options = x;
      x = undefined;
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
  window.parent = window.top = {};

  window.innerWidth = defaultCanvasSize[0];
  window.innerHeight = defaultCanvasSize[1];
  window.devicePixelRatio = 1;
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
    vendor: 'MixedReality',
    platform: os.platform(),
    hardwareConcurrency: os.cpus().length,
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
      if (nativeVr && nativeVr.VR_IsHmdPresent()) {
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
  window.setTimeout = (fn, timeout, args) => {
    fn = fn.bind.apply(fn, [window].concat(args));
    fn[symbols.windowSymbol] = window;
    const id = ++rafIndex;
    fn[symbols.idSymbol] = id;
    timeouts[_findFreeSlot(timeouts)] = fn;
    fn[symbols.timeoutSymbol] = setTimeout(fn, timeout, args);
    return id;
  };
  window.clearTimeout = id => {
    const index = timeouts.findIndex(t => t && t[symbols.idSymbol] === id);
    if (index !== -1) {
      clearTimeout(timeouts[index][symbols.timeoutSymbol]);
      timeouts[index] = null;
    }
  };
  window.setInterval = (fn, interval, args) => {
    if (interval < 10) {
      interval = 10;
    }
    fn = fn.bind.apply(fn, [window].concat(args));
    fn[symbols.windowSymbol] = window;
    const id = ++rafIndex;
    fn[symbols.idSymbol] = id;
    intervals[_findFreeSlot(intervals)] = fn;
    fn[symbols.timeoutSymbol] = setInterval(fn, interval, args);
    return id;
  };
  window.clearInterval = id => {
    const index = intervals.findIndex(i => i && i[symbols.idSymbol] === id);
    if (index !== -1) {
      clearInterval(intervals[index][symbols.timeoutSymbol]);
      intervals[index] = null;
    }
  };
  window.fetch = (url, options) => {
    if (typeof url === 'string') {
      const blob = urls.get(url);
      if (blob) {
        return Promise.resolve(new Response(blob));
      } else {
        url = _normalizeUrl(url);
        return fetch(url, options);
      }
    } else {
      return fetch(url, options);
    }
  };
  window.Request = Request;
  window.Response = Response;
  window.Headers = Headers;
  window.Blob = Blob;
  window.FormData = FormData;
  window.XMLHttpRequest = (Old => {
    class XMLHttpRequest extends Old {
      open(method, url, async, username, password) {
        url = _normalizeUrl(url);
        return super.open(method, url, async, username, password);
      }
    }
    for (const k in XMLHttpRequestBase) {
      XMLHttpRequest[k] = XMLHttpRequestBase[k];
    }
    return XMLHttpRequest;
  })(XMLHttpRequest);
  window.WebSocket = (Old => {
    class WebSocket extends Old {
      constructor(url, protocols) {
        url = _normalizeUrl(url);
        super(url, protocols);
      }
    }
    for (const k in Old) {
      WebSocket[k] = Old[k];
    }
    return WebSocket;
  })(WebSocket);
  window.event = new Event(); // XXX this needs to track the current event
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
  window.MutationObserver = require('./MutationObserver').MutationObserver;
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
  window.browser = {
    http,
    // https,
    ws,
    createRenderTarget: nativeWindow.createRenderTarget, // XXX needed for reality tabs fakeDisplay
    magicleap: nativeMl ? {
      RequestMeshing: nativeMl.RequestMeshing,
      RequestPlaneTracking: nativeMl.RequestPlaneTracking,
      RequestHandTracking: nativeMl.RequestHandTracking,
      RequestEyeTracking: nativeMl.RequestEyeTracking,
      RequestDepthPopulation: nativeMl.RequestDepthPopulation,
      RequestCamera: nativeMl.RequestCamera,
    } : null,
    monitors: new MonitorManager(),
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
      return utils._parseDocumentAst(htmlAst, window, false);
    }
  };
  window.Event = Event;
  window.KeyboardEvent = KeyboardEvent;
  window.MouseEvent = MouseEvent;
  window.WheelEvent = WheelEvent;
  window.DragEvent = DragEvent;
  window.MessageEvent = MessageEvent;
  window.PromiseRejectionEvent = PromiseRejectionEvent;
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
  if (window.navigator.xr) {
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
  }
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
      workerOptions.startScript = `
        (() => {
          ${windowStartScript}

          const bindings = requireNative("nativeBindings");
          const smiggles = require("smiggles");
          const events = require("events");
          const {EventEmitter} = events;

          smiggles.bind({ImageBitmap: bindings.nativeImageBitmap});

          global.Image = bindings.nativeImage;
          global.ImageBitmap = bindings.nativeImageBitmap;
          global.createImageBitmap = ${createImageBitmap.toString()};
          global.EventEmitter = EventEmitter;
          global.EventTarget = ${EventTarget.toString()};
          global.FileReader = ${FileReader.toString()};
        })();
      `;

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
      window._emit('message', new MessageEvent('message', {data}));
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

  const _destroyTimeouts = window => {
    const _pred = fn => fn[symbols.windowSymbol] === window;
    for (let i = 0; i < rafCbs.length; i++) {
      const rafCb = rafCbs[i];
      if (rafCb && _pred(rafCb)) {
        rafCbs[i] = null;
      }
    }
    for (let i = 0; i < timeouts.length; i++) {
      const timeout = timeouts[i];
      if (timeout && _pred(timeout)) {
        clearTimeout(timeout[symbols.timeoutSymbol]);
        timeouts[i] = null;
      }
    }
    for (let i = 0; i < intervals.length; i++) {
      const interval = intervals[i];
      if (interval && _pred(interval)) {
        clearInterval(interval[symbols.timeoutSymbol]);
        intervals[i] = null;
      }
    }
  };

  window.on('destroy', e => {
    _destroyTimeouts(e.window);
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

          _destroyTimeouts(window);
        })
        .catch(err => {
          loading = false;

          const e = new ErrorEvent('error', {target: this});
          e.message = err.message;
          e.stack = err.stack;
          this.dispatchEvent(e);
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
    fakeVrDisplay.onlayers = layers => {
      GlobalContext.fakePresentState.layers = layers;
    };

    const vrDisplay = new VRDisplay();
    _bindMRDisplay(vrDisplay);
    vrDisplay.onrequestpresent = layers => nativeVr.requestPresent(layers);
    vrDisplay.onexitpresent = () => nativeVr.exitPresent();
    vrDisplay.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const xrDisplay = new XR.XRDevice('VR');
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
    xrDisplay.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const mlDisplay = new MLDisplay();
    _bindMRDisplay(mlDisplay);
    mlDisplay.onrequestpresent = layers => nativeMl.requestPresent(layers);
    mlDisplay.onexitpresent = () => nativeMl.exitPresent();
    mlDisplay.onlayers = layers => {
      GlobalContext.mlPresentState.layers = layers;
    };

    const xmDisplay = new XR.XRDevice('AR');
    xmDisplay.onrequestpresent = layers => nativeMl.requestPresent(layers);
    xmDisplay.onexitpresent = () => nativeMl.exitPresent();
    xmDisplay.onrequestanimationframe = _makeRequestAnimationFrame(window);
    xmDisplay.oncancelanimationframe = window.cancelAnimationFrame;
    xmDisplay.requestSession = (requestSession => function() {
      return requestSession.apply(this, arguments)
        .then(session => {
          mlDisplay.isPresenting = true;
          session.once('end', () => {
            mlDisplay.isPresenting = false;
          });
          return session;
        });
    })(xmDisplay.requestSession);
    xmDisplay.onlayers = layers => {
      GlobalContext.mlPresentState.layers = layers;
    };

    window[symbols.mrDisplaysSymbol] = {
      fakeVrDisplay,
      vrDisplay,
      xrDisplay,
      mlDisplay,
      xmDisplay,
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
      if (mlDisplay.isPresenting || update.force) {
        mlDisplay.update(update);
        updatedHmd = true;
      }
      if (xmDisplay.session || update.force) {
        xmDisplay.update(update);
        updatedHmd = true;
      }
      if (updatedHmd) {
        _updateGamepads(update.gamepads);
      }
    };
    /* window.updateArFrame = (viewMatrix, projectionMatrix) => {
      arDisplay.update(viewMatrix, projectionMatrix);
    }; */

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

  window.document = utils._parseDocument(htmlString, window);

  return window;
};

/**
 * Initialize classes and modules that require native bindings.
 * Required before creating any windows or documents.
 * Set rather than `require`d directly due to way `require` works with multithreading
 * (for `Worker`), use this route to make sure binaries only get initialized once.
 *
 * @param {string} nativeBindingsModule - Path to native bindings JS module.
 */
const setNativeBindingsModule = nativeBindingsModule => {
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
  nativeWindow = bindings.nativeWindow;
};