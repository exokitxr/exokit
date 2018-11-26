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
} = require('./VR.js');

const BindingsModule = require('./bindings');
const {defaultCanvasSize} = require('./constants');
const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');
const {urls} = require('./urls');

GlobalContext.args = {};
GlobalContext.version = '';

// Class imports.
const {_makeWindowVm} = require('./Window');
const {_parseDocument, _parseDocumentAst, Document, DocumentFragment, DocumentType, DOMImplementation, initDocument} = require('./Document');
const DOM = require('./DOM');
const {DOMRect, Node, NodeList} = require('./DOM');
const {CustomEvent, DragEvent, ErrorEvent, Event, EventTarget, KeyboardEvent, MessageEvent, MouseEvent, WheelEvent, PromiseRejectionEvent} = require('./Event');
const {History} = require('./History');
const {Location} = require('./Location');
const {XMLHttpRequest} = require('./Network');
const XR = require('./XR');
const utils = require('./utils');
const {_elementGetter, _elementSetter} = require('./utils');

GlobalContext.styleEpoch = 0;

const maxParallelResources = 8;
class Resource {
  constructor(getCb = (onprogress, cb) => cb(), value = 0.5, total = 1) {
    this.getCb = getCb;
    this.value = value;
    this.total = total;
    
    this.onupdate = null;
  }

  setProgress(value) {
    this.value = value;

    this.onupdate && this.onupdate();
  }
  
  get() {
    return new Promise((accept, reject) => {
      this.getCb(progress => {
        this.setValue(progress);
      }, err => {
        if (!err) {
          accept();
        } else {
          reject(err);
        }
      });
    });
  }
  
  destroy() {
    this.setProgress(1);
  }
}
class Resources extends EventTarget {
  constructor() {
    super();
    
    this.resources = [];
    this.queue = [];
    this.numRunning = 0;
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

  addResource(getCb) {
    return new Promise((accept, reject) => {
      const resource = new Resource(getCb);
      resource.onupdate = () => {
        if (resource.value >= resource.total) {
          this.resources.splice(this.resources.indexOf(resource), 1);
          
          resource.onupdate = null;
          
          accept();
        }

        const e = new Event('update');
        e.value = this.getValue();
        e.total = this.getTotal();
        e.progress = this.getProgress();
        this.dispatchEvent(e);
      };
      this.resources.push(resource);
      this.queue.push(resource);
      
      this.drain();
    });
  }
  
  drain() {
    if (this.queue.length > 0 && this.numRunning < maxParallelResources) {
      const resource = this.queue.shift();
      resource.get()
        .catch(err => {
          console.warn(err.stack);
        })
        .finally(() => {
          resource.destroy();
          
          this.numRunning--;
          
          this.drain();
        });
      
      this.numRunning++;
    }
  }
}
GlobalContext.Resources = Resources;

let nativeVm = GlobalContext.nativeVm = null;
let nativeWorker = null;
let nativeVr = GlobalContext.nativeVr = null;
let nativeMl = GlobalContext.nativeMl = null;
let nativeWindow = null;

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

// To "run" the HTML means to walk it and execute behavior on the elements such as <script src="...">.
// Each candidate element exposes a method on runSymbol which returns whether to await the element load or not.
const _runHtml = (element, window) => {
  if (element instanceof DOM.HTMLElement) {
    return new Promise((accept, reject) => {
      const {document} = window;

      element.traverse(el => {
        const {id} = el;
        if (id) {
          el._emit('attribute', 'id', id);
        }

        if (el[symbols.runSymbol]) {
          document[symbols.addRunSymbol](el[symbols.runSymbol].bind(el));
        }

        if (/\-/.test(el.tagName)) {
          const constructor = window.customElements.get(el.tagName);
          if (constructor) {
            window.customElements.upgrade(el, constructor);
          }
        }
      });
      if (document[symbols.runningSymbol]) {
        document.once('flush', () => {
          accept();
        });
      } else {
        accept();
      }
    });
  } else {
    return Promise.resolve();
  }
};
GlobalContext._runHtml = _runHtml;

const core = (htmlString = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1/';
  options.baseUrl = options.baseUrl || options.url;
  options.dataPath = options.dataPath || __dirname;
  return _makeWindowVm(htmlString, options);
};
core.load = (src, options = {}) => {
  if (!url.parse(src).protocol) {
    src = 'http://' + src;
  }
  let redirectCount = 0;
  const _fetchTextFollow = src => fetch(src, {
    redirect: 'manual',
  })
    .then(res => {
      if (res.status >= 200 && res.status < 300) {
        return res.text()
          .then(htmlString => ({
            src,
            htmlString,
          }));
      } else if (res.status >= 300 && res.status < 400) {
        const l = res.headers.get('Location');
        if (l) {
          if (redirectCount < 10) {
            redirectCount++;
            return _fetchTextFollow(l);
          } else {
            return Promise.reject(new Error('fetch got too many redirects: ' + res.status + ' : ' + src));
          }
        } else {
          return Promise.reject(new Error('fetch got redirect with no location header: ' + res.status + ' : ' + src));
        }
      } else {
        return Promise.reject(new Error('fetch got invalid status code: ' + res.status + ' : ' + src));
      }
    });
  return _fetchTextFollow(src)
    .then(({src, htmlString}) => {
      let baseUrl;
      if (options.baseUrl) {
        baseUrl = options.baseUrl;
      } else {
        baseUrl = utils._getBaseUrl(src);
      }

      return exokit(htmlString, {
      return core(htmlString, {
        url: options.url || src,
        baseUrl,
        dataPath: options.dataPath,
      });
    });
};
core.setArgs = newArgs => {
  GlobalContext.args = newArgs;
};
core.setVersion = newVersion => {
  GlobalContext.version = newVersion;
};
module.exports = core;

/**
 * Initialize classes and modules that require native bindings.
 * Required before creating any windows or documents.
 * Set rather than `require`d directly due to way `require` works with multithreading
 * (for `Worker`), use this route to make sure binaries only get initialized once.
 *
 * @param {string} nativeBindingsModule - Path to native bindings JS module.
 */
exokit.setNativeBindingsModule = nativeBindingsModule => {
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
module.exports = exokit;

if (require.main === module) {
  if (process.argv.length === 3) {
    const baseUrl = 'file://' + __dirname + '/';
    const u = new URL(process.argv[2], baseUrl).href;
    exokit.load(u);
  }
}
