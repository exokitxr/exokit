const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const fs = require('fs');
const url = require('url');
const http = require('http');
const https = require('https');
const crypto = require('crypto');
const os = require('os');
const util = require('util');
const {URL} = url;
const {TextEncoder, TextDecoder} = util;
const {performance} = require('perf_hooks');

const {FileReader} = require('./File.js');

const mkdirp = require('mkdirp');
const ws = require('ws');
const {XMLHttpRequest: XMLHttpRequestBase, FormData} = require('window-xhr');

const fetch = require('window-fetch');
const {Request, Response, Headers, Blob} = fetch;

const core = require('./core.js');

const WebSocket = require('ws/lib/websocket');
const {
  /* getUserMedia,
  MediaStream,
  MediaStreamTrack,
  RTCDataChannel, */
  RTCIceCandidate,
  RTCPeerConnection,
  /* RTCPeerConnectionIceEvent,
  RTCRtpReceiver,
  RTCRtpSender, */
  RTCRtpTransceiver,
  RTCSessionDescription,

  RTCPeerConnectionIceEvent,
  RTCDataChannelEvent,
  RTCDataChannelMessageEvent,
  RTCTrackEvent,
} = require('./RTC/index.js');

const nativeWorker = require('worker-native');

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
} = require('./VR.js');

const {defaultCanvasSize} = require('./constants');
const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');
const {urls} = require('./urls');

const bindings = require('./native-bindings');
const {
  nativeVm,
  nativeImage: Image,
  nativeImageData: ImageData,
  nativeImageBitmap: ImageBitmap,
  nativePath2D: Path2D,
  nativeCanvasGradient: CanvasGradient,
  nativeCanvasRenderingContext2D: CanvasRenderingContext2D,
  nativeGl: WebGLRenderingContext,
  nativeGl2: WebGL2RenderingContext,
  nativeAudio: {
    AudioContext,
    AudioNode,
    AudioBufferSourceNode,
    OscillatorNode,
    AudioDestinationNode,
    AudioParam,
    AudioListener,
    GainNode,
    AnalyserNode,
    PannerNode,
    StereoPannerNode,
    MicrophoneMediaStream,
  },
  nativeVideo: {
    Video,
    VideoDevice,
  },
  nativeOpenVR,
  nativeOculusMobileVr,
  nativeMl,
  nativeBrowser,
  nativeWindow,
  nativeOculusVR
} = bindings;

GlobalContext.args = {};
GlobalContext.version = '';

// Class imports.
const {_parseDocument, _parseDocumentAst, getBoundDocumentElements, DocumentType, DOMImplementation, initDocument} = require('./Document');
const {
  HTMLElement,
  getBoundDOMElements,
  NodeList,
  HTMLCollection,
  DOMRect,
  DOMPoint,
  createImageBitmap,
} = require('./DOM');
const {CustomEvent, DragEvent, ErrorEvent, Event, EventTarget, KeyboardEvent, MessageEvent, MouseEvent, WheelEvent, PromiseRejectionEvent} = require('./Event');
const {History} = require('./History');
const {Location} = require('./Location');
const {XMLHttpRequest} = require('./Network');
const XR = require('./XR');
const DevTools = require('./DevTools');
const utils = require('./utils');
const {_elementGetter, _elementSetter, _download} = utils;

const btoa = s => Buffer.from(s, 'binary').toString('base64');
const atob = s => Buffer.from(s, 'base64').toString('binary');

class CustomElementRegistry {
  constructor(window) {
    this._window = window;

    this.elements = {};
    this.extensions = {};
    this.elementPromises = {};
  }

  define(name, constructor, options = {}) {
    name = name.toUpperCase();

    this.elements[name] = constructor;
    if (options.extends) {
      this.extensions[options.extends.toUpperCase()] = name.toLowerCase();
    }

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
    if (el instanceof HTMLElement) {
      let wasConnected = el.isConnected;
      el.ownerDocument.on('domchange', () => {
        const newConnected = el.isConnected;
        if (newConnected && !wasConnected) {
          el.connectedCallback && el.connectedCallback();
          wasConnected = true;
        } else if (wasConnected && !newConnected) {
          el.disconnectedCallback && el.disconnectedCallback();
          wasConnected = false;
        }
      });

      const observedAttributes = constructor.observedAttributes ? constructor.observedAttributes() : [];
      if (observedAttributes.length > 0) {
        el.on('attribute', (name, value, oldValue) => {
          if (el.attributeChangedCallback && observedAttributes.includes(name)) {
            el.attributeChangedCallback(name, value, oldValue);
          }
        });
      }

      Object.setPrototypeOf(el, constructor.prototype);
      HTMLElement.upgradeElement = el;
      let error = null;
      try {
        Object.setPrototypeOf(el, constructor.prototype);
        Reflect.construct(constructor, []);
      } catch(err) {
        error = err;
      }
      HTMLElement.upgradeElement = null;

      if (!error) {
        if (wasConnected) {
          setImmediate(() => {
            el.connectedCallback && el.connectedCallback();
          });
        }
      } else {
        throw error;
      }
    } else {
      throw new Error('cannot upgrade non-subclass of HTMLElement');
    }
  }
}

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

class Worker {
  constructor(src) {
    this.worker = nativeWorker.make({
      initModule: path.join(__dirname, 'Worker.js'),
      args: {
        src,
      },
    });
  }

  postMessage(message, transferList) {
    this.worker.postMessage(message, transferList);
  }

  get onmessage() {
    return this.worker.onmessage;
  }
  set onmessage(onmessage) {
    this.worker.onmessage = onmessage;
  }
  get onerror() {
    return this.worker.onerror;
  }
  set onerror(onerror) {
    this.worker.onerror = onerror;
  }
}

let rafIndex = 0;
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
  const rafCbs = window[symbols.rafCbsSymbol];
  rafCbs[_findFreeSlot(rafCbs)] = fn;
  rafCbs.sort((a, b) => (b ? b[symbols.prioritySymbol] : 0) - (a ? a[symbols.prioritySymbol] : 0));
  return id;
};
const _makeOnRequestHitTest = window => (origin, direction, cb) => nativeMl.RequestHitTest(origin, direction, cb, window);

GlobalContext.fakeVrDisplayEnabled = false;

const _makeWindow = (options = {}, parent = null, top = null) => {
  const _normalizeUrl = utils._makeNormalizeUrl(options.baseUrl);

  const vmo = nativeVm.make();
  const window = vmo.getGlobal();
  window.vm = vmo;

  // Store original prototypes for converting to and from native and JS.
  utils._storeOriginalWindowPrototypes(window, symbols.prototypesSymbol);

  const windowStartScript = `(() => {
    ${!options.args.require ? 'global.require = undefined;' : ''}

    const _logStack = err => {
      console.warn(err);
    };
    process.on('uncaughtException', _logStack);
    process.on('unhandledRejection', _logStack);

    global.process = undefined;
    global.setImmediate = undefined;
    window.global = undefined;
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
  Object.defineProperty(window, 'devicePixelRatio', {
    get() {
      if (!GlobalContext.xrState.devicePixelRatio[0]) {
        GlobalContext.xrState.devicePixelRatio[0] = nativeWindow.getDevicePixelRatio();
      }
      return GlobalContext.xrState.devicePixelRatio[0];
    },
    set(devicePixelRatio) {},
  });
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
  function getUserMedia(constraints) {
    if (constraints.audio) {
      return Promise.resolve(new MicrophoneMediaStream());
    } else if (constraints.video) {
      const dev = new VideoDevice();
      dev.constraints = constraints.video;
      return Promise.resolve(dev);
    } else {
      return Promise.reject(new Error('constraints not met'));
    }
  }
  window.navigator = {
    userAgent: `Mozilla/5.0 (OS) AppleWebKit/999.0 (KHTML, like Gecko) Chrome/999.0.0.0 Safari/999.0 Exokit/${GlobalContext.version}`,
    vendor: 'Exokit',
    platform: os.platform(),
    hardwareConcurrency: os.cpus().length,
    appCodeName: 'Mozilla',
    appName: 'Netscape',
    appVersion: '5.0',
    language: 'en-US',
    mediaDevices: {
      getUserMedia,
      enumerateDevices() {
        let deviceIds = 0;
        let groupIds = 0;
        return Promise.resolve([
          {
            deviceId: (++deviceIds) + '',
            groupId: (++groupIds) + '',
            kind: 'audioinput',
            label: 'Microphone',
          },
        ]);
      },
    },
    webkitGetUserMedia: getUserMedia, // for feature detection
    getVRDisplaysSync() {
      const result = [];
      if (GlobalContext.fakeVrDisplayEnabled) {
        result.push(window[symbols.mrDisplaysSymbol].fakeVrDisplay);
      }

      // Oculus runtime takes precedence over OpenVR for Oculus headsets.
      if (nativeOculusVR && nativeOculusVR.Oculus_IsHmdPresent()) {
        result.push(window[symbols.mrDisplaysSymbol].oculusVRDisplay);
      } else if (nativeOpenVR && nativeOpenVR.VR_IsHmdPresent()) {
        result.push(window[symbols.mrDisplaysSymbol].openVRDisplay);
      }

      if (nativeOculusMobileVr && nativeOculusMobileVr.OculusMobile_IsHmdPresent()) {
        result.push(window[symbols.mrDisplaysSymbol].oculusMobileVrDisplay);
      }

      if (nativeMl && nativeMl.IsPresent()) {
        result.push(window[symbols.mrDisplaysSymbol].mlDisplay);
      }

      result.sort((a, b) => +b.isPresenting - +a.isPresenting);
      return result;
    },
    createVRDisplay() {
      GlobalContext.fakeVrDisplayEnabled = true;
      return window[symbols.mrDisplaysSymbol].fakeVrDisplay;
    },
    getGamepads: getGamepads.bind(null, window),
    clipboard:{
      read:() => Promise.resolve(), // Not implemented yet
      readText: () => new Promise(resolve => {
        resolve(nativeWindow.getClipboard().slice(0, 256));// why do we slice this?
      }),
      write:() => Promise.resolve(), // Not implemented yet
      writeText: clipboardContents => new Promise(resolve => {
        nativeWindow.setClipboard(clipboardContents);
        resolve();
      })
    }
  };

  // WebVR enabled.
  if (['all', 'webvr'].includes(options.args.xr)) {
    window.navigator.getVRDisplays = function() {
      return Promise.resolve(this.getVRDisplaysSync());
    }
  }

  // WebXR enabled.
  if (['all', 'webxr'].includes(options.args.xr)) {
    window.navigator.xr = new XR.XR(window);
  }

  window.destroy = function() {
    this._emit('destroy', {window: this});
  };
  window.URL = URL;
  window.console = console;
  window.alert = console.log;
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
  const _maybeDownload = (m, u, data, bufferifyFn) => options.args.download ? _download(m, u, data, bufferifyFn, options.args.download) : data;
  window.fetch = (u, options) => {
    const _boundFetch = (u, options) => {
      const req = utils._normalizePrototype(
        fetch(u, options),
        window
      );
      return req
        .then(res => {
          res.arrayBuffer = (fn => function() {
            return utils._normalizePrototype(
              fn.apply(this, arguments),
              window
            );
          })(res.arrayBuffer);
          res.blob = (fn => function() {
            return utils._normalizePrototype(
              fn.apply(this, arguments),
              window
            );
          })(res.blob);
          res.json = (fn => function() {
            return utils._normalizePrototype(
              fn.apply(this, arguments),
              window
            );
          })(res.json);
          res.text = (fn => function() {
            return utils._normalizePrototype(
              fn.apply(this, arguments),
              window
            );
          })(res.text);

          const method = (options && options.method) || 'GET';
          res.arrayBuffer = (fn => function() {
            return fn.apply(this, arguments)
              .then(ab => _maybeDownload(method, u, utils._normalizePrototype(ab, window), ab => Buffer.from(ab)));
          })(res.arrayBuffer);
          res.blob = (fn => function() {
            return fn.apply(this, arguments)
              .then(blob => _maybeDownload(method, u, utils._normalizePrototype(blob, window), blob => blob.buffer));
          })(res.blob);
          res.json = (fn => function() {
            return fn.apply(this, arguments)
              .then(j => _maybeDownload(method, u, j, j => Buffer.from(JSON.stringify(j))));
          })(res.json);
          res.text = (fn => function() {
            return fn.apply(this, arguments)
              .then(t => _maybeDownload(method, u, t, t => Buffer.from(t, 'utf8')));
          })(res.text);

          return res;
        });
    };

    if (typeof u === 'string') {
      const blob = urls.get(u);
      if (blob) {
        return Promise.resolve(new Response(blob));
      } else {
        u = _normalizeUrl(u);
        return _boundFetch(u, options);
      }
    } else {
      return _boundFetch(u, options);
    }
  };
  window.Request = Request;
  window.Response = (Old => class Response extends Old {
    constructor(body, opts) {
      super(utils._normalizePrototype(body, global), opts);
    }
  })(Response);
  window.Headers = Headers;
  window.Blob = (Old => class Blob extends Old {
    constructor(parts, opts) {
      super(parts && parts.map(part => utils._normalizePrototype(part, global)), opts);
    }
  })(Blob);
  window.FormData = (Old => class FormData extends Old {
    append(field, value, options) {
      super.append(field, utils._normalizePrototype(value, global), options);
    }
  })(FormData);
  window.XMLHttpRequest = (Old => {
    class XMLHttpRequest extends Old {
      open(method, url, async, username, password) {
        url = _normalizeUrl(url);
        return super.open(method, url, async, username, password);
      }
      get response() {
        return _maybeDownload(this._properties.method, this._properties.uri, utils._normalizePrototype(super.response, window), o => {
          switch (this.responseType) {
            case 'arraybuffer': return Buffer.from(o);
            case 'blob': return o.buffer;
            case 'json': return Buffer.from(JSON.stringify(o), 'utf8');
            case 'text': return Buffer.from(o, 'utf8');
            default: throw new Error(`cannot download responseType ${responseType}`);
          }
        });
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
        super(url, protocols, {
          origin: location.origin,
        });
      }
      emit(type, event) {
        if (type === 'message') {
          event = utils._normalizePrototype(event, window);
        }
        return super.emit.apply(this, arguments);
      }
      send(data) {
        return super.send(utils._normalizePrototype(data, global));
      }
    }
    for (const k in Old) {
      WebSocket[k] = Old[k];
    }
    return WebSocket;
  })(WebSocket);
  window.crypto = {
    getRandomValues(typedArray) {
      crypto.randomFillSync(Buffer.from(typedArray.buffer, typedArray.byteOffset, typedArray.byteLength));
      return typedArray;
    },

    subtle: {
      digest(algo, bytes) {
        switch (algo) {
          case 'SHA-1': {
            algo = 'sha1';
            break;
          }
          case 'SHA-256': {
            algo = 'sha256';
            break;
          }
          case 'SHA-384': {
            algo = 'sha384';
            break;
          }
          case 'SHA-512': {
            algo = 'sha512';
            break;
          }
          default: throw new Error(`unknown algorithm: ${algo}`);
        }
        const hash = crypto.createHash(algo).update(bytes).digest();
        const result = new ArrayBuffer(hash.byteLength);
        new Buffer(result).set(hash);
        return Promise.resolve(result);
      },
    },
  };
  window.event = new Event(); // XXX this needs to track the current event
  window.localStorage = new LocalStorage(path.join(options.dataPath, '.localStorage'));
  window.sessionStorage = new LocalStorage(path.join(options.dataPath, '.sessionStorage'));
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
  window[symbols.optionsSymbol] = options;
  window[symbols.styleEpochSymbol] = 0;

  // DOM.
  const {
    Document,
    DocumentFragment,
    Range,
  } = getBoundDocumentElements(window);
  window.Document = Document;
  window.DocumentFragment = DocumentFragment;
  window.DocumentType = DocumentType;
  window.DOMImplementation = DOMImplementation;

  const {
    Element,
    HTMLElement,
    HTMLHeadElement,
    HTMLBodyElement,
    HTMLAnchorElement,
    HTMLStyleElement,
    HTMLLinkElement,
    HTMLScriptElement,
    HTMLImageElement,
    HTMLAudioElement,
    HTMLVideoElement,
    HTMLSourceElement,
    SVGElement,
    HTMLIFrameElement,
    HTMLCanvasElement,
    HTMLTextareaElement,
    HTMLTemplateElement,
    HTMLDivElement,
    HTMLUListElement,
    HTMLLIElement,
    HTMLTableElement,
    Node,
    Text,
    Comment,
  } = getBoundDOMElements(window);
  window.Element = Element;
  window.HTMLElement = HTMLElement;
  window.HTMLHeadElement = HTMLHeadElement;
  window.HTMLBodyElement = HTMLBodyElement;
  window.HTMLAnchorElement = HTMLAnchorElement;
  window.HTMLStyleElement = HTMLStyleElement;
  window.HTMLLinkElement = HTMLLinkElement;
  window.HTMLScriptElement = HTMLScriptElement;
  window.HTMLImageElement = HTMLImageElement,
  window.HTMLAudioElement = HTMLAudioElement;
  window.HTMLVideoElement = HTMLVideoElement;
  window.SVGElement = SVGElement;
  window.HTMLIFrameElement = HTMLIFrameElement;
  window.HTMLCanvasElement = HTMLCanvasElement;
  window.HTMLTextareaElement = HTMLTextareaElement;
  window.HTMLTemplateElement = HTMLTemplateElement;
  window.HTMLDivElement = HTMLDivElement;
  window.HTMLUListElement = HTMLUListElement;
  window.HTMLLIElement = HTMLLIElement;
  window.HTMLTableElement = HTMLTableElement;
  window.Node = Node;
  window.Text = Text;
  window.Comment = Comment;
  window[symbols.htmlTagsSymbol] = {
    DOCUMENT: Document,
    HEAD: HTMLHeadElement,
    BODY: HTMLBodyElement,
    A: HTMLAnchorElement,
    STYLE: HTMLStyleElement,
    SCRIPT: HTMLScriptElement,
    LINK: HTMLLinkElement,
    IMG: HTMLImageElement,
    AUDIO: HTMLAudioElement,
    VIDEO: HTMLVideoElement,
    SOURCE: HTMLSourceElement,
    IFRAME: HTMLIFrameElement,
    CANVAS: HTMLCanvasElement,
    TEXTAREA: HTMLTextareaElement,
    TEMPLATE: HTMLTemplateElement,
    DIV: HTMLDivElement,
    ULIST: HTMLUListElement,
    LI: HTMLLIElement,
    TABLE: HTMLTableElement,
  };
  window.NodeList = NodeList;
  window.HTMLCollection = HTMLCollection;

  /* window.MediaStreamTrack = MediaStreamTrack;
  window.RTCRtpReceiver = RTCRtpReceiver;
  window.RTCRtpSender = RTCRtpSender; */
  window.MediaStream = class MediaStream {};

  window.RTCPeerConnection = RTCPeerConnection;
  window.webkitRTCPeerConnection = RTCPeerConnection; // for feature detection
  window.RTCSessionDescription = RTCSessionDescription;
  window.RTCIceCandidate = RTCIceCandidate;

  window.RTCPeerConnectionIceEvent = RTCPeerConnectionIceEvent;
  window.RTCDataChannelEvent = RTCDataChannelEvent;
  window.RTCDataChannelMessageEvent = RTCDataChannelMessageEvent;
  window.RTCTrackEvent = RTCTrackEvent;

  window.RTCRtpTransceiver = RTCRtpTransceiver;

  window.customElements = new CustomElementRegistry(window);
  window.CustomElementRegistry = CustomElementRegistry;
  window.MutationObserver = require('./MutationObserver').MutationObserver;
  window.DOMRect = DOMRect;
  window.DOMPoint = DOMPoint;
  window.getComputedStyle = el => {
    let styleSpec = el[symbols.computedStyleSymbol];
    if (!styleSpec || styleSpec.epoch !== window[symbols.styleEpochSymbol]) {
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
        styleEpoch: window[symbols.styleEpochSymbol],
      };
      el[symbols.computedStyleSymbol] = styleSpec;
    }
    return styleSpec.style;
  };
  window.browser = {
    devTools: DevTools,
    http: (() => {
      const httpProxy = {};
      for (const k in http) {
        httpProxy[k] = http[k];
      }
      httpProxy.createServer = (createServer => function(cb) {
        if (typeof cb === 'function') {
          cb = (cb => function(req, res) {
            res.write = (write => function(d) {
              if (typeof d === 'object') {
                d = utils._normalizePrototype(d, global);
              }
              return write.apply(this, arguments);
            })(res.write);
            res.end = (end => function(d) {
              if (typeof d === 'object') {
                d = utils._normalizePrototype(d, global);
              }
              return end.apply(this, arguments);
            })(res.end);

            return cb.apply(this, arguments);
          })(cb);
        }
        return createServer.apply(this, arguments);
      })(httpProxy.createServer);
      return httpProxy;
    })(),
    // https,
    ws: (() => {
      const wsProxy = {};
      for (const k in ws) {
        wsProxy[k] = ws[k];
      }
      wsProxy.Server = (OldServer => function Server() {
        const server = Reflect.construct(OldServer, arguments);
        server.on = (on => function(e, cb) {
          if (e === 'connection' && cb) {
            cb = (cb => function(c) {
              c.on = (on => function(e, cb) {
                if (e === 'message' && cb) {
                  cb = (cb => function(m) {
                    m = utils._normalizePrototype(m, window);
                    return cb.apply(this, arguments);
                  })(cb);
                }
                return on.apply(this, arguments);
              })(c.on);
              c.send = (send => function(d) {
                d = utils._normalizePrototype(d, global);
                return send.apply(this, arguments);
              })(c.send);
              return cb.apply(this, arguments);
            })(cb);
          }
          return on.apply(this, arguments);
        })(server.on);
        return server;
      })(wsProxy.Server);
      return wsProxy;
    })(),
    createRenderTarget(context) { // XXX needed for reality tabs fakeDisplay
      nativeWindow.setCurrentWindowContext(context.getWindowHandle());
      return nativeWindow.createRenderTarget.apply(nativeWindow, arguments);
    },
    magicleap: nativeMl ? {
      RequestMeshing() {
        const mesher = nativeMl.RequestMeshing(window);
        return {
          get onmesh() {
            return mesher.onmesh;
          },
          set onmesh(cb) {
            mesher.onmesh = cb ? updates => {
              for (let i = 0; i < updates.length; i++) {
                const update = updates[i];
                if (update.positionArray) {
                  update.positionArray = utils._normalizePrototype(update.positionArray, window);
                }
                if (update.normalArray) {
                  update.normalArray = utils._normalizePrototype(update.normalArray, window);
                }
                if (update.indexArray) {
                  update.indexArray = utils._normalizePrototype(update.indexArray, window);
                }
              }
              cb(updates);
            } : null;
          },
        };
      },
      RequestPlaneTracking: () => nativeMl.RequestPlaneTracking(window),
      RequestHandTracking: () => nativeMl.RequestHandTracking(window),
      RequestEyeTracking: () => nativeMl.RequestEyeTracking(window),
      RequestImageTracking: (img, size) => nativeMl.RequestImageTracking(window, img, size),
      RequestDepthPopulation: nativeMl.RequestDepthPopulation,
      RequestCamera(cb) {
        if (typeof cb === 'function') {
          cb = (cb => function(datas) {
            for (let i = 0; i < datas.length; i++) {
              const data = datas[i];
              data.data = utils._normalizePrototype(data.data, window);
            }
            return cb.apply(this, arguments);
          })(cb);
        }
        return nativeMl.RequestCamera.apply(nativeMl, arguments);
      },
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
  window.PromiseRejectionEvent = PromiseRejectionEvent;
  window.CustomEvent = CustomEvent;
  window.EventTarget = EventTarget;
  window.addEventListener = EventTarget.prototype.addEventListener.bind(window);
  window.removeEventListener = EventTarget.prototype.removeEventListener.bind(window);
  window.dispatchEvent = EventTarget.prototype.dispatchEvent.bind(window);
  window.Image = HTMLImageElement;
  window.ImageData = ImageData;
  window.ImageBitmap = ImageBitmap;
  window.Path2D = Path2D;
  window.CanvasGradient = CanvasGradient;
  window.CanvasRenderingContext2D = CanvasRenderingContext2D;
  window.WebGLRenderingContext = WebGLRenderingContext;
  if (options.args.webgl !== '1') {
    window.WebGL2RenderingContext = WebGL2RenderingContext;
  }
  window.Audio = HTMLAudioElement;
  window.MediaRecorder = MediaRecorder;
  window.DataTransfer = DataTransfer;
  window.DataTransferItem = DataTransferItem;
  window.FileReader = FileReader;
  window.Screen = Screen;
  window.Gamepad = Gamepad;
  window.VRStageParameters = VRStageParameters;
  window.VRDisplay = VRDisplay;
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
    window.XRRay = XR.XRRay;
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
  window.Worker = class extends Worker {
    constructor(src) {
      if (src instanceof Blob) {
        super('data:application/javascript,' + src.buffer.toString('utf8'));
      } else {
        const blob = urls.get(src);
        const normalizedSrc = blob ?
          'data:application/octet-stream;base64,' + blob.buffer.toString('base64')
        :
          _normalizeUrl(src);
        super(normalizedSrc);
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
      core.load(href, {
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
          window.dispatchEvent(e);
        });
      loading = true;
    }
  });

  const rafCbs = [];
  window[symbols.rafCbsSymbol] = rafCbs;
  const timeouts = [];
  const intervals = [];
  window.tickAnimationFrame = (() => {
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

        // tickAnimationFrame.window = this;

        const performanceNow = performance.now();

        // hidden rafs
        for (let i = 0; i < localCbs.length; i++) {
          const rafCb = localCbs[i];
          if (rafCb && rafCb[symbols.windowSymbol].document.hidden) {
            try {
              // console.log('tick raf', rafCb.stack);
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
              // console.log('tick raf', rafCb.stack);
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

        // tickAnimationFrame.window = null;
      }

      _clearLocalCbs(); // release garbage
    }
    // tickAnimationFrame.window = null;
    return tickAnimationFrame;
  })();

  const _makeMrDisplays = () => {
    const _bindMRDisplay = display => {
      display.onrequestanimationframe = _makeRequestAnimationFrame(window);
      display.oncancelanimationframe = window.cancelAnimationFrame;
      display.onvrdisplaypresentchange = () => {
        const e = new Event('vrdisplaypresentchange');
        e.display = display;
        window.dispatchEvent(e);
      };
    };

    const fakeVrDisplay = new FakeVRDisplay(window);
    fakeVrDisplay.onrequestpresent = layers => {
      if (!GlobalContext.fakePresentState.fakeVrDisplay) {
        GlobalContext.fakePresentState.fakeVrDisplay = fakeVrDisplay;
      }

      const [{source: canvas}] = layers;
      const {_context: context} = canvas;
      return {
        width: context.drawingBufferWidth,
        height: context.drawingBufferHeight,
        msFbo: null,
      };
    };
    fakeVrDisplay.onexitpresent = () => {
      GlobalContext.fakePresentState.fakeVrDisplay = null;
    };
    fakeVrDisplay.onlayers = layers => {
      GlobalContext.fakePresentState.layers = layers;
    };

    const openVRDisplay = new VRDisplay('OpenVR');
    _bindMRDisplay(openVRDisplay);
    openVRDisplay.onrequestpresent = layers => nativeOpenVR.requestPresent(layers);
    openVRDisplay.onexitpresent = () => nativeOpenVR.exitPresent();
    openVRDisplay.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const oculusVRDisplay = new VRDisplay('OculusVR');
    _bindMRDisplay(oculusVRDisplay);
    oculusVRDisplay.onrequestpresent = layers => nativeOculusVR.requestPresent(layers);
    oculusVRDisplay.onexitpresent = () => nativeOculusVR.exitPresent();
    oculusVRDisplay.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const openVRDevice = new XR.XRDevice('OpenVR', window);
    openVRDevice.onrequestpresent = layers => nativeOpenVR.requestPresent(layers);
    openVRDevice.onexitpresent = () => nativeOpenVR.exitPresent();
    openVRDevice.onrequestanimationframe = _makeRequestAnimationFrame(window);
    openVRDevice.oncancelanimationframe = window.cancelAnimationFrame;
    openVRDevice.requestSession = (requestSession => function() {
      return requestSession.apply(this, arguments)
        .then(session => {
          openVRDisplay.isPresenting = true;
          session.once('end', () => {
            openVRDisplay.isPresenting = false;
          });
          return session;
        });
    })(openVRDevice.requestSession);
    openVRDevice.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const oculusVRDevice = new XR.XRDevice('OculusVR', window);
    oculusVRDevice.onrequestpresent = layers => nativeOculusVR.requestPresent(layers);
    oculusVRDevice.onexitpresent = () => nativeOculusVR.exitPresent();
    oculusVRDevice.onrequestanimationframe = _makeRequestAnimationFrame(window);
    oculusVRDevice.oncancelanimationframe = window.cancelAnimationFrame;
    oculusVRDevice.requestSession = (requestSession => function() {
      return requestSession.apply(this, arguments)
        .then(session => {
          oculusVRDisplay.isPresenting = true;
          session.once('end', () => {
            oculusVRDisplay.isPresenting = false;
          });
          return session;
        });
    })(oculusVRDevice.requestSession);
    oculusVRDevice.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const oculusMobileVrDisplay = new VRDisplay('OculusMobileVR');
    _bindMRDisplay(oculusMobileVrDisplay);
    oculusMobileVrDisplay.onrequestpresent = layers => nativeOculusMobileVr.requestPresent(layers);
    oculusMobileVrDisplay.onexitpresent = () => nativeOculusMobileVr.exitPresent();
    oculusMobileVrDisplay.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const oculusMobileVrDevice = new XR.XRDevice('OculusMobileVR', window);
    oculusMobileVrDevice.onrequestpresent = layers => nativeOculusMobileVr.requestPresent(layers);
    oculusMobileVrDevice.onexitpresent = () => nativeOculusMobileVr.exitPresent();
    oculusMobileVrDevice.onrequestanimationframe = _makeRequestAnimationFrame(window);
    oculusMobileVrDevice.oncancelanimationframe = window.cancelAnimationFrame;
    oculusMobileVrDevice.requestSession = (requestSession => function() {
      return requestSession.apply(this, arguments)
        .then(session => {
          oculusMobileVrDisplay.isPresenting = true;
          session.once('end', () => {
            oculusMobileVrDisplay.isPresenting = false;
          });
          return session;
        });
    })(oculusMobileVrDevice.requestSession);
    oculusMobileVrDevice.onlayers = layers => {
      GlobalContext.vrPresentState.layers = layers;
    };

    const magicLeapARDisplay = new VRDisplay('AR');
    _bindMRDisplay(magicLeapARDisplay);
    magicLeapARDisplay.onrequestpresent = layers => nativeMl.requestPresent(layers);
    magicLeapARDisplay.onexitpresent = () => nativeMl.exitPresent();
    magicLeapARDisplay.onrequesthittest = _makeOnRequestHitTest(window);
    magicLeapARDisplay.onlayers = layers => {
      GlobalContext.mlPresentState.layers = layers;
    };

    const magicLeapARDevice = new XR.XRDevice('AR', window);
    magicLeapARDevice.onrequestpresent = layers => nativeMl.requestPresent(layers);
    magicLeapARDevice.onexitpresent = () => nativeMl.exitPresent();
    magicLeapARDevice.onrequestanimationframe = _makeRequestAnimationFrame(window);
    magicLeapARDevice.oncancelanimationframe = window.cancelAnimationFrame;
    magicLeapARDevice.requestSession = (requestSession => function() {
      return requestSession.apply(this, arguments)
        .then(session => {
          magicLeapARDisplay.isPresenting = true;
          session.once('end', () => {
            magicLeapARDisplay.isPresenting = false;
          });
          return session;
        });
    })(magicLeapARDevice.requestSession);
    magicLeapARDevice.onrequesthittest = _makeOnRequestHitTest(window);
    magicLeapARDevice.onlayers = layers => {
      GlobalContext.mlPresentState.layers = layers;
    };

    return {
      fakeVrDisplay,
      openVRDisplay,
      oculusVRDisplay,
      openVRDevice,
      oculusVRDevice,
      oculusMobileVrDisplay,
      oculusMobileVrDevice,
      magicLeapARDisplay,
      magicLeapARDevice,
    };
  };
  window[symbols.mrDisplaysSymbol] = _makeMrDisplays();

  GlobalContext.windows.push(window);
  window.on('destroy', () => {
    GlobalContext.windows.splice(GlobalContext.windows.indexOf(window), 1);
  });

  return window;
};
module.exports._makeWindow = _makeWindow;
GlobalContext._makeWindow = _makeWindow;

const _makeWindowWithDocument = (s, options, parent, top) => {
  const window = _makeWindow(options, parent, top);
  _parseDocument(s, window); // attaches window.document
  return window;
};
module.exports._makeWindowWithDocument = _makeWindowWithDocument;
