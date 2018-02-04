const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const url = require('url');
const {URL} = url;
const {performance} = require('perf_hooks');
const vm = require('vm');

const parse5 = require('parse5');

const fetch = require('window-fetch');
const {XMLHttpRequest} = require('xmlhttprequest');
const {Response, Blob} = fetch;
const WebSocket = require('ws/lib/websocket');
const {LocalStorage} = require('node-localstorage');
const WindowWorker = require('window-worker');
const THREE = require('./lib/three-min.js');

const windowSymbol = Symbol();
const htmlTagsSymbol = Symbol();
const htmlElementsSymbol = Symbol();
const optionsSymbol = Symbol();

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

class MessageEvent {
  constructor(data) {
    this.data = data;
  }
}
const ImageData = (() => {
  if (typeof nativeImageData !== 'undefined') {
    return nativeImageData;
  } else {
    // throw new Error('fail to bind native image data class');
    return class ImageData {
      constructor(width, height) {
        this.width = width;
        this.height = height;
        this.data = new Uint8ClampedArray(0);
      }
    };
  }
})();
const ImageBitmap = (() => {
  if (typeof nativeImageBitmap !== 'undefined') {
    return nativeImageBitmap;
  } else {
    // throw new Error('fail to bind native image bitmap class');
    class ImageBitmap {
      constructor(image) {
        this.width = image.width;
        this.height = image.height;
      }
    }
    ImageBitmap.createImageBitmap = image => new ImageBitmap(image.width, image.height);
    return ImageBitmap;
  }
})();
const Path2D = (() => {
  if (typeof nativePath2D !== 'undefined') {
    return nativePath2D;
  } else {
    // throw new Error('fail to bind native path 2d class');
    return class Path2D {
      moveTo() {}
      lineTo() {}
      quadraticCurveTo() {}
    };
  }
})();
const CanvasRenderingContext2D = (() => {
  if (typeof nativeCanvasRenderingContext2D !== 'undefined') {
    return nativeCanvasRenderingContext2D;
  } else {
    // throw new Error('fail to bind native canvas rendering context 2d class');
    return class CanvasRenderingContext2D {
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
    };
  }
})();
const WebGLContext = (() => {
  if (typeof nativeGl !== 'undefined') {
    // console.log('make gl proxy');

    return nativeGl;
    /* return function WebGLContext() {
      return new Proxy(new nativeGl(), {
        get(target, propKey, receiver) {
          const orig = target[propKey];
          if (typeof orig === 'function') {
            return function(a, b, c, d, e, f) {
              console.log('gl proxy method ' + propKey);
              return orig.apply(target, arguments);
            };
          } else {
            return orig;
          }
        }
      });
    }; */
  } else {
    const VERSION = Symbol();
    return class WebGLContext {
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
    };
  }
})();
class VRFrameData {
  constructor() {
    this.leftProjectionMatrix = new Float32Array(16);
    this.leftViewMatrix = new Float32Array(16);
    this.rightProjectionMatrix = new Float32Array(16);
    this.rightViewMatrix = new Float32Array(16);
    this.pose = new VRPose();
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
    this.depthFar = 1000.0;
    this.stageParameters = {
      // new THREE.Matrix4().compose(new THREE.Vector3(0, 1.6, 0), new THREE.Quaternion(), new THREE.Vector3(1, 1, 1)).toArray(new Float32Array(16))
      sittingToStandingTransform: Float32Array.from([1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1.6, 0, 1]),
    };

    this._width = window.innerWidth / 2;
    this._height = window.innerHeight;
    this._viewMatrix = new Float32Array(16);
    this._projectionMatrix = new Float32Array(16);

    const _resize = () => {
      this._width = window.innerWidth / 2;
      this._height = window.innerHeight;
    };
    window.on('resize', _resize);
    const _alignframe = (viewMatrix, projectionMatrix) => {
      this._viewMatrix.set(viewMatrix);
      this._projectionMatrix.set(projectionMatrix);
    };
    window.on('alignframe', _alignframe);

    this._cleanup = () => {
      window.removeListener('resize', _resize);
      window.removeListener('alignframe', _alignframe);
    };
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
    return {
      renderWidth: this._width,
      renderHeight: this._height,
    };
  }

  requestPresent(sources) {
    this.isPresenting = true;

    process.nextTick(() => {
      this[windowSymbol].emit('vrdisplaypresentchange');
    });

    return Promise.resolve();
  }

  exitPresent() {
    this.isPresenting = false;

    process.nextTick(() => {
      this[windowSymbol].emit('vrdisplaypresentchange');
    });

    return Promise.resolve();
  }

  requestAnimationFrame(fn) {
    return this[windowSymbol].requestAnimationFrame(fn);
  }

  cancelAnimationFrame(animationFrame) {
    return this[windowSymbol].cancelAnimationFrame(animationFrame);
  }

  submitFrame() {}

  destroy() {
    this.cleanup();
  }
}
class VRDisplay extends MRDisplay {
  constructor(window, displayId) {
    super('VR', window, displayId);
  }

  getFrameData(frameData) {
    const hmdMatrix = localMatrix.fromArray(this._viewMatrix);

    hmdMatrix.decompose(localVector, localQuaternion, localVector2);
    frameData.pose.set(localVector, localQuaternion);

    hmdMatrix.getInverse(hmdMatrix);

    localMatrix2.compose( // head to eye transform
      localVector.set(-0.02, 0, 0),
      localQuaternion.set(0, 0, 0, 1),
      localVector2.set(0, 0, 0),
    )
      .multiply(hmdMatrix)
      .toArray(frameData.leftViewMatrix);

    frameData.leftProjectionMatrix.set(this._projectionMatrix);

    localMatrix2.compose( // head to eye transform
      localVector.set(0.02, 0, 0),
      localQuaternion.set(0, 0, 0, 1),
      localVector2.set(0, 0, 0),
    )
      .multiply(hmdMatrix)
      .toArray(frameData.rightViewMatrix);

    frameData.rightProjectionMatrix.set(this._projectionMatrix);
  }
}
class ARDisplay extends MRDisplay {
  constructor(window, displayId) {
    super('AR', window, displayId);
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
}
class AudioNode {}
class AudioParam {
  constructor() {
    this.value = 0;
    this.minValue = 0;
    this.maxValue = 0;
    this.defaultValue = 0;
  }

  setValueAtTime() {}
}
class GainNode extends AudioNode {
  connect() {}
}
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
}
class AudioContext {
  constructor() {
    this.listener = new AudioListener();
  }

  createGain() {
    return new GainNode();
  }
}

class Node extends EventEmitter {
  constructor(nodeName = null) {
    super();

    this.nodeName = nodeName;
    this.parentNode = null;
  }
}
class HTMLElement extends Node {
  constructor(tagName = 'div', attributes = {}, value = '') {
    super(null);

    this.tagName = tagName;
    this.attributes = attributes;
    this.value = value;
    this.childNodes = [];

    this._innerHTML = '';
  }

  get attrs() {
    return _formatAttributes(this.attributes);
  }
  set attrs(attrs) {
    this.attributes = _parseAttributes(attrs);
  }

  get children() {
    return this.childNodes;
  }
  set children(children) {
    this.childNodes = children;
  }

  getAttribute(name) {
    return this.attributes[name];
  }
  setAttribute(name, value) {
    this.attributes[name] = value;

    this.emit('attribute', name, value);
  }

  appendChild(childNode) {
    this.childNodes.push(childNode);
    childNode.parentNode = this;
  }
  removeChild(childNode) {
    const index = this.childNodes.indexOf(childNode);
    if (index !== -1) {
      this.childNodes.splice(index, 1);
      childNode.parentNode = null;
    }
  }
  insertBefore(childNode, nextSibling) {
    const index = this.childNodes.indexOf(nextSibling);
    if (index !== -1) {
      this.childNodes.splice(index, 0, childNode);
      childNode.parentNode = this;
    }
  }
  insertAfter(childNode, nextSibling) {
    const index = this.childNodes.indexOf(nextSibling);
    if (index !== -1) {
      this.childNodes.splice(index + 1, 0, childNode);
      childNode.parentNode = this;
    }
  }

  getElementById(id) {
    return this.traverse(node => {
      if (
        (node.getAttribute && node.getAttribute('id') === id) ||
        (node.attrs && node.attrs.some(attr => attr.name === 'id' && attr.value === id))
      ) {
        return node;
      }
    });
  }
  getElementByClassName(className) {
    return this.traverse(node => {
      if (
        (node.getAttribute && node.getAttribute('class') === className) ||
        (node.attrs && node.attrs.some(attr => attr.name === 'class' && attr.value === className))
      ) {
        return node;
      }
    });
  }
  getElementByTagName(tagName) {
    return this.traverse(node => {
      if (node.tagName === tagName) {
        return node;
      }
    });
  }
  querySelector(selector) {
    let match;
    if (match = selector.match(/^#(.+)$/)) {
      return this.getElementById(match[1]);
    } else if (match = selector.match(/^\.(.+)$/)) {
      return this.getElementByClassName(match[1]);
    } else {
      return this.getElementByTagName(selector);
    }
  }
  getElementsById(id) {
    const result = [];
    this.traverse(node => {
      if (
        (node.getAttribute && node.getAttribute('id') === id) ||
        (node.attrs && node.attrs.some(attr => attr.name === 'id' && attr.value === id))
      ) {
        result.push(node);
      }
    });
    return result;
  }
  getElementsByClassName(className) {
    const result = [];
    this.traverse(node => {
      if (
        (node.getAttribute && node.getAttribute('class') === className) ||
        (node.attrs && node.attrs.some(attr => attr.name === 'class' && attr.value === className))
      ) {
        result.push(node);
      }
    });
    return result;
  }
  getElementsByTagName(tagName) {
    const result = [];
    this.traverse(node => {
      if (node.tagName === tagName) {
        result.push(node);
      }
    });
    return result;
  }
  querySelectorAll(selector) {
    let match;
    if (match = selector.match(/^#(.+)$/)) {
      return this.getElementsById(match[1]);
    } else if (match = selector.match(/^\.(.+)$/)) {
      return this.getElementsByClassName(match[1]);
    } else {
      return this.getElementsByTagName(selector);
    }
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
      }
    };
    return _recurse(this);
  }

  addEventListener() {
    this.on.apply(this, arguments);
  }
  removeEventListener() {
    this.removeListener.apply(this, arguments);
  }

  get offsetWidth() {
    const style = _parseStyle(this.attributes['style'] || '');
    const fontFamily = style['font-family'];
    if (fontFamily) {
      return _hash(fontFamily) * _hash(this.innerHTML);
    } else {
      return 0;
    }
  }
  set offsetWidth(offsetWidth) {}
  get offsetHeight() {
    return 0;
  }
  set offsetHeight(offsetHeight) {}

  get className() {
    return this.attributes['class'] || '';
  }
  set className(className) {
    this.attributes['class'] = className;
  }

  get style() {
    const style = _parseStyle(this.attributes['style'] || '');
    Object.defineProperty(style, 'cssText', {
      get: () => this.attributes['style'],
      set: cssText => {
        this.attributes['style'] = cssText;
      },
    });
    return style;
  }
  set style(style) {
    this.attributes['style'] = _formatStyle(style);
  }

  get innerHTML() {
    return parse5.serialize(this);
  }
  set innerHTML(innerHTML) {
    const childNodes = parse5.parseFragment(innerHTML).childNodes.map(childNode => _fromAST(childNode, this[windowSymbol], this));
    this.childNodes = childNodes;

    _promiseSerial(childNodes.map(childNode => () => _runHtml(childNode, this[windowSymbol])))
      .catch(err => {
        console.warn(err);
      });

    this.emit('innerHTML', innerHTML);
  }

  requestPointerLock() {
    const {document} = this[windowSymbol];

    if (document.pointerLockElement === null) {
      document.pointerLockElement = this;

      if (typeof nativeWindow !== 'undefined') {
        nativeWindow.setCursorMode(false);
      }

      document.emit('pointerlockchange');
    }
  }
  exitPointerLock() {
    const {document} = this[windowSymbol];

    if (document.pointerLockElement !== null) {
      document.pointerLockElement = null;

      if (typeof nativeWindow !== 'undefined') {
        nativeWindow.setCursorMode(true);
      }

      document.emit('pointerlockchange');
    }
  }
}
class HTMLAnchorElement extends HTMLElement {
  constructor(attributes = {}, value = '') {
    super('a', attributes, value);
  }

  get href() {
    return this.getAttribute('href') || '';
  }
  set href(value) {
    this.setAttribute('href', value);
  }
}
class HTMLLoadableElement extends HTMLElement {
  constructor(tagName, attributes = {}, value = '') {
    super(tagName, attributes, value);
  }

  get onload() {
    return this.listeners('load')[0];
  }
  set onload(onload) {
    if (typeof onload === 'function') {
      this.addEventListener('load', onload);
    } else {
      const listeners = this.listeners('load');
      for (let i = 0; i < listeners.length; i++) {
        this.removeEventListener('load', listeners[i]);
      }
    }
  }

  get onerror() {
    return this.listeners('error')[0];
  }
  set onerror(onerror) {
    if (typeof onerror === 'function') {
      this.addEventListener('error', onerror);
    } else {
      const listeners = this.listeners('error');
      for (let i = 0; i < listeners.length; i++) {
        this.removeEventListener('error', listeners[i]);
      }
    }
  }
}
class HTMLWindowElement extends HTMLLoadableElement {
  constructor() {
    super('window');
  }

  postMessage(data) {
    this.emit('message', new MessageEvent(data));
  }

  get onmessage() {
    return this.listeners('load')[0];
  }
  set onmessage(onmessage) {
    if (typeof onmessage === 'function') {
      this.addEventListener('message', onmessage);
    } else {
      const listeners = this.listeners('message');
      for (let i = 0; i < listeners.length; i++) {
        this.removeEventListener('message', listeners[i]);
      }
    }
  }
}
class HTMLScriptElement extends HTMLLoadableElement {
  constructor(attributes = {}, value = '') {
    super('script', attributes, value);

    this.readyState = null;

    this.on('attribute', (name, value) => {
      if (name === 'src') {
        this.readyState = null;

        const url = value;
        this[windowSymbol].fetch(url)
          .then(res => {
            if (res.status >= 200 && res.status < 300) {
              return res.text();
            } else {
              return Promise.reject(new Error('script src got invalid status code: ' + res.status + ' : ' + url));
            }
          })
          .then(jsString => {
            _runJavascript(jsString, this[windowSymbol], url);

            this.readyState = 'complete';

            this.emit('load');
          })
          .catch(err => {
            this.readyState = 'complete';

            this.emit('error', err);
          });
      }
    });
    this.on('innerHTML', innerHTML => {
      _runJavascript(innerHTML, this[windowSymbol]);

      this.readyState = 'complete';

      process.nextTick(() => {
        this.emit('load');
      });
    });
  }

  get src() {
    return this.getAttribute('src') || '';
  }
  set src(value) {
    this.setAttribute('src', value);
  }

  set innerHTML(innerHTML) {
    this.emit('innerHTML', innerHTML);
  }

  run() {
    let running = false;
    if (this.attributes.src) {
      this.src = this.attributes.src;
      running = true;
    }
    if (this.childNodes.length > 0) {
      this.innerHTML = this.childNodes[0].value;
      running = true;
    }
    return running;
  }
}
class HTMLMediaElement extends HTMLLoadableElement {
  constructor(tagName = null, attributes = {}, value = '') {
    super(tagName, attributes, value);
  }

  get src() {
    this.getAttribute('src');
  }
  set src(value) {
    this.setAttribute('src', value);
  }

  run() {
    if (this.attributes.src) {
      this.src = this.attributes.src;
      return true;
    } else {
      return false;
    }
  }
}
const HTMLImageElement = (() => {
  if (typeof nativeImage !== 'undefined') {
    return class HTMLImageElement extends nativeImage {
      constructor(attributes = {}, value = '') {
        super();
        EventEmitter.call(this);
        this.tagName = 'image'
        this.attributes = attributes;
        this.value = value;

        this.stack = new Error().stack;

        this._src = '';
      }

      emit(event, data) {
        return EventEmitter.prototype.emit.call(this, event, data);
      }
      on(event, cb) {
        return EventEmitter.prototype.on.call(this, event, cb);
      }
      removeListener(event, cb) {
        return EventEmitter.prototype.removeListener.call(this, event, cb);
      }

      addEventListener(event, cb) {
        return HTMLElement.prototype.addEventListener.call(this, event, cb);
      }
      removeEventListener(event, cb) {
        return HTMLElement.prototype.removeEventListener.call(this, event, cb);
      }

      get src() {
        return this._src;
      }
      set src(src) {
        this._src = src;

        const srcError = new Error();

        this[windowSymbol].fetch(src)
          .then(res => {
            if (res.status >= 200 && res.status < 300) {
              return res.arrayBuffer();
            } else {
              return Promise.reject(new Error(`img src got invalid status code (url: ${JSON.stringify(src)}, code: ${res.status})`));
            }
          })
          .then(arrayBuffer => {
            if (this.load(arrayBuffer)) {
              return Promise.resolve();
            } else {
              console.warn('failed to decode image src', srcError.stack);
              return Promise.reject(new Error(`failed to decode image (url: ${JSON.stringify(src)}, size: ${arrayBuffer.byteLength})`));
            }
          })
          .then(() => {
            this.emit('load');
          })
          .catch(err => {
            this.emit('error', err);
          });
      }

      get onload() {
        return this.listeners('load')[0];
      }
      set onload(onload) {
        if (typeof onload === 'function') {
          this.addEventListener('load', onload);
        } else {
          const listeners = this.listeners('load');
          for (let i = 0; i < listeners.length; i++) {
            this.removeEventListener('load', listeners[i]);
          }
        }
      }

      get onerror() {
        return this.listeners('error')[0];
      }
      set onerror(onerror) {
        if (typeof onerror === 'function') {
          this.addEventListener('error', onerror);
        } else {
          const listeners = this.listeners('error');
          for (let i = 0; i < listeners.length; i++) {
            this.removeEventListener('error', listeners[i]);
          }
        }
      }
    };
  } else {
    return class HTMLImageElement extends HTMLMediaElement {
      constructor() {
        super('image');

        this.stack = new Error().stack;

        this.on('attribute', (name, value) => {
          if (name === 'src') {
            process.nextTick(() => { // XXX
              this.emit('load');
            });
          }
        });
      }

      get width() {
        return 0; // XXX
      }
      set width(width) {}
      get height() {
        return 0; // XXX
      }
      set height(height) {}
    };
  }
})();
class HTMLAudioElement extends HTMLMediaElement {
  constructor(attributes = {}, value = '') {
    super('audio', attributes, value);

    this.on('attribute', (name, value) => {
      if (name === 'src') {
        process.nextTick(() => { // XXX
          this.emit('load');
          this.emit('canplay');
        });
      }
    });
  }

  get oncanplay() {
    return this.listeners('canplay')[0];
  }
  set oncanplay(oncanplay) {
    if (typeof oncanplay === 'function') {
      this.addEventListener('canplay', oncanplay);
    } else {
      const listeners = this.listeners('canplay');
      for (let i = 0; i < listeners.length; i++) {
        this.removeEventListener('canplay', listeners[i]);
      }
    }
  }

  get oncanplaythrough() {
    return this.listeners('canplaythrough')[0];
  }
  set oncanplaythrough(oncanplaythrough) {
    if (typeof oncanplaythrough === 'function') {
      this.addEventListener('canplaythrough', oncanplaythrough);
    } else {
      const listeners = this.listeners('canplaythrough');
      for (let i = 0; i < listeners.length; i++) {
        this.removeEventListener('canplaythrough', listeners[i]);
      }
    }
  }
}
class HTMLVideoElement extends HTMLMediaElement {
  constructor(attributes = {}, value = '') {
    super('video', attributes, value);

    this.on('attribute', (name, value) => {
      if (name === 'src') {
        process.nextTick(() => { // XXX
          this.emit('load');
        });
      }
    });
  }
}
class HTMLIframeElement extends HTMLMediaElement {
  constructor(attributes = {}, value = '') {
    super('iframe', attributes, value);

    this.on('attribute', (name, value) => {
      if (name === 'src') {
        const url = value;
        this[windowSymbol].fetch(url)
          .then(res => {
            if (res.status >= 200 && res.status < 300) {
              return res.text();
            } else {
              return Promise.reject(new Error('iframe src got invalid status code: ' + res.status + ' : ' + url));
            }
          })
          .then(htmlString => {
            const contentDocument = _parseDocument(htmlString, this.contentWindow[optionsSymbol], this.contentWindow);
            this.contentDocument = contentDocument;

            contentDocument.once('readystatechange', () => {
              this.emit('load');
            });
          })
          .catch(err => {
            this.emit('error', err);
          });
      }
    });
    this.on('window', () => {
      const parentWindow = this[windowSymbol];
      this.contentWindow = _parseWindow('', parentWindow[optionsSymbol], parentWindow, parentWindow.top);
      this.contentDocument = this.contentWindow.document;
    });
  }
}
class HTMLCanvasElement extends HTMLElement {
  constructor(attributes = {}, value = '') {
    super('canvas', attributes, value);

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
     return this.getAttribute('width') || 1;
  }
  set width(value) {
    if (typeof value === 'number' && isFinite(value)) {
      this.setAttribute('width', value);
    }
  }

  get height() {
    return this.getAttribute('height') || 1;
  }
  set height(value) {
    if (typeof value === 'number' && isFinite(value)) {
      this.setAttribute('height', value);
    }
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
        console.log('gl context webgl');
        this._context = new WebGLContext();
      }
    }
    return this._context;
  }
}
class TextNode extends Node {
  constructor(value) {
    super('#text');

    this.value = value;
  }
}
class CommentNode extends Node {
  constructor(value) {
    super('#comment');

    this.value = value;
  }
}

const _fromAST = (node, window, parentNode = null) => {
  if (node.nodeName === '#text') {
    const textNode = new window[htmlElementsSymbol].TextNode(node.value);
    textNode.parentNode = parentNode;
    return textNode;
  } else if (node.nodeName === '#comment') {
    const commentNode = new window[htmlElementsSymbol].CommentNode(node.value);
    commentNode.parentNode = parentNode;
    return commentNode;
  } else {
    const {tagName, value} = node;
    const attributes = node.attrs && _parseAttributes(node.attrs);
    const HTMLElementTemplate = window[htmlTagsSymbol][tagName];
    const element = HTMLElementTemplate ?
      new HTMLElementTemplate(
        attributes,
        value
      )
    :
      new window[htmlElementsSymbol].HTMLElement(
        tagName,
        attributes,
        value
      );
    element.parentNode = parentNode;
    if (node.childNodes) {
      element.childNodes = node.childNodes.map(childNode => _fromAST(childNode, window, element));
    }
    return element;
  }
};
const _parseAttributes = attrs => {
  const result = {};
  for (let i = 0; i < attrs.length; i++) {
    const attr = attrs[i];
    result[attr.name] = attr.value;
  }
  return result;
};
const _formatAttributes = attributes => {
  const result = [];
  for (const name in attributes) {
    const value = attributes[name];
    result.push({
      name,
      value,
    });
  }
  return result;
};
const _parseStyle = styleString => {
  const style = {};
  const split = styleString.split(/;\s*/);
  for (let i = 0; i < split.length; i++) {
    const split2 = split[i].split(/:\s*/);
    if (split2.length === 2) {
      style[split2[0]] = split2[1];
    }
  }
  return style;
};
const _formatStyle = style => {
  let styleString = '';
  for (const k in style) {
    styleString += (styleString.length > 0 ? ' ' : '') + k + ': ' + style[k] + ';';
  }
  return styleString;
};
const _hash = s => {
  let result = 0;
  for (let i = 0; i < s.length; i++) {
    result += s.codePointAt(i);
  }
  return result;
};
const _promiseSerial = async promiseFns => {
  for (let i = 0; i < promiseFns.length; i++) {
    await promiseFns[i]();
  }
};
const _loadPromise = el => new Promise((accept, reject) => {
  el.on('load', () => {
    accept();
  });
  el.on('error', err => {
    reject(err);
  });
});
const _runHtml = async (element, window) => {
  if (element instanceof HTMLElement) {
    const scripts = element.querySelectorAll('script');
    for (let i = 0; i < scripts.length; i++) {
      const script = scripts[i];
      if (script.run()) {
        if (script.attributes.async) {
          _loadPromise(script)
            .catch(err => {
              console.warn(err);
            });
        } else {
          try {
            await _loadPromise(script);
          } catch(err) {
            console.warn(err);
          }
        }
      }
    }

    const images = element.querySelectorAll('image');
    for (let i = 0; i < images.length; i++) {
      const image = images[i];
      if (image.run()) {
        await _loadPromise(image);
      }
    }

    const audios = element.querySelectorAll('audio');
    for (let i = 0; i < audios.length; i++) {
      const audio = audios[i];
      if (audio.run()) {
        await _loadPromise(audioEl);
      }
    }

    const videos = element.querySelectorAll('video');
    for (let i = 0; i < videos.length; i++) {
      const video = videos[i];
      if (video.run()) {
        await _loadPromise(videoEl);
      }
    }
  }
};
const _runJavascript = (jsString, window, filename = 'script') => {
  try {
    vm.runInContext(jsString, window, {
      filename,
    });
  } catch (err) {
    console.warn(err);
  }
};
const _makeWindow = (options = {}, parent = null, top = null) => {
  const _normalizeUrl = src => new URL(src, options.baseUrl).href;

  const window = new HTMLWindowElement();
  window.window = window;
  window.self = window;
  window.parent = parent || window;
  window.top = top || window;
  window.innerWidth = 1280;
  window.innerHeight = 1024;
  window.devicePixelRatio = 1;
  window.console = console;
  window.setTimeout = setTimeout;
  window.clearTimeout = clearTimeout;
  window.setInterval = setInterval;
  window.clearInterval = clearInterval;
  window.Date = Date;
  window.performance = performance;
  window.location = url.parse(options.url);
  let vrDisplays = [];
  const gamepads = [];
  let vrMode = null;
  let vrTexture = null;
  window.navigator = {
    userAgent: 'exokit',
    getVRDisplays: () => vrDisplays,
    getGamepads: () => gamepads,
    getVRMode: () => vrMode,
    setVRMode: newVrMode => {
      for (let i = 0; i < vrDisplays.length; i++) {
        vrDisplays[i].destroy();
      }

      if (newVrMode === 'vr') {
        vrDisplays = [new VRDisplay(window, 0)];
      } else if (newVrMode === 'ar') {
        vrDisplays = [new ARDisplay(window, 1)];
      }
      vrMode = newVrMode;
    },
    getVRTexture: () => vrTexture,
    setVRTexture: newVrTexture => {
      vrTexture = newVrTexture;
    },
  };
  window.localStorage = new LocalStorage(path.join(options.dataPath, '.localStorage'));
  window.document = null;
  window.URL = URL;
  window[htmlElementsSymbol] = {
    Node: (Old => class Node extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(Node),
    HTMLElement: (Old => class HTMLElement extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(HTMLElement),
    HTMLAnchorElement: (Old => class HTMLAnchorElement extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(HTMLAnchorElement),
    HTMLScriptElement: (Old => class HTMLScriptElement extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(HTMLScriptElement),
    HTMLImageElement: (Old => class HTMLImageElement extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(HTMLImageElement),
    HTMLAudioElement: (Old => class HTMLAudioElement extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(HTMLAudioElement),
    HTMLVideoElement: (Old => class HTMLVideoElement extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(HTMLVideoElement),
    HTMLIframeElement: (Old => class HTMLIframeElement extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(HTMLIframeElement),
    HTMLCanvasElement: (Old => class HTMLCanvasElement extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(HTMLCanvasElement),
    TextNode: (Old => class TextNode extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(TextNode),
    CommentNode: (Old => class CommentNode extends Old { constructor() { super(...arguments); this[windowSymbol] = window; this.emit('window'); } })(CommentNode),
  };
  window[htmlTagsSymbol] = {
    a: window[htmlElementsSymbol].HTMLAnchorElement,
    script: window[htmlElementsSymbol].HTMLScriptElement,
    img: window[htmlElementsSymbol].HTMLImageElement,
    audio: window[htmlElementsSymbol].HTMLAudioElement,
    video: window[htmlElementsSymbol].HTMLVideoElement,
    iframe: window[htmlElementsSymbol].HTMLIframeElement,
    canvas: window[htmlElementsSymbol].HTMLCanvasElement,
  };
  window[optionsSymbol] = options;
  window.Image = window[htmlElementsSymbol].HTMLImageElement;
  window.HTMLElement = window[htmlElementsSymbol].HTMLElement;
  window.HTMLAnchorElement = window[htmlElementsSymbol].HTMLAnchorElement;
  window.HTMLScriptElement = window[htmlElementsSymbol].HTMLScriptElement;
  window.HTMLImageElement = window[htmlElementsSymbol].HTMLImageElement;
  window.HTMLAudioElement = window[htmlElementsSymbol].HTMLAudioElement;
  window.HTMLVideoElement = window[htmlElementsSymbol].HTMLVideoElement;
  window.HTMLIframeElement = window[htmlElementsSymbol].HTMLIframeElement;
  window.HTMLCanvasElement = window[htmlElementsSymbol].HTMLCanvasElement;
  window.ImageData = ImageData;
  window.ImageBitmap = ImageBitmap;
  window.Path2D = Path2D;
  window.CanvasRenderingContext2D = CanvasRenderingContext2D;
  window.VRDisplay = VRDisplay;
  // window.ARDisplay = ARDisplay;
  window.VRFrameData = VRFrameData;
  window.btoa = s => new Buffer(s, 'binary').toString('base64');
  window.atob = s => new Buffer(s, 'base64').toString('binary');
  window.fetch = (url, options) => {
    const blob = urls.get(url);
    if (blob) {
      return Promise.resolve(new Response(blob));
    } else {
      return fetch(_normalizeUrl(url), options);
    }
  };
  window.XMLHttpRequest = XMLHttpRequest;
  window.WebSocket = WebSocket;
  window.Worker = class Worker extends WindowWorker {
    constructor(src, workerOptions = {}) {
      workerOptions.baseUrl = options.baseUrl;

      if (src instanceof Blob) {
        super('data:application/javascript,' + src[Blob.BUFFER].toString('utf8'), workerOptions);
      } else {
        super(_normalizeUrl(src), workerOptions);
      }
    }
  };
  window.Blob = Blob;
  window.AudioContext = AudioContext;
  window.Path2D = Path2D;
  window.createImageBitmap = image => Promise.resolve(ImageBitmap.createImageBitmap(image));
  const rafCbs = [];
  window.requestAnimationFrame = fn => {
    rafCbs.push(fn);
    return fn;
  };
  window.cancelAnimationFrame = fn => {
    const index = rafCbs.indexOf(fn);
    if (index !== -1) {
      rafCbs.splice(index, 1);
    }
  };
  window.tickAnimationFrame = () => {
    const localRafCbs = rafCbs.slice();
    rafCbs.length = 0;
    for (let i = 0; i < localRafCbs.length; i++) {
      localRafCbs[i]();
    }
  };
  window.alignFrame = (viewMatrix, projectionMatrix) => {
    window.emit('alignframe', viewMatrix, projectionMatrix);
  };
  vm.createContext(window);
  return window;
};
const _parseDocument = (s, options, window) => {
  const document = _fromAST(parse5.parse(s), window);
  const html = document.childNodes.find(element => element.tagName === 'html');
  const head = html.childNodes.find(element => element.tagName === 'head');
  const body = html.childNodes.find(element => element.tagName === 'body');

  document.documentElement = document;
  document.readyState = null;
  document.head = head;
  document.body = body;
  document.location = url.parse(options.url);
  document.createElement = tagName => {
    const HTMLElementTemplate = window[htmlTagsSymbol][tagName];
    return HTMLElementTemplate ? new HTMLElementTemplate() : new window[htmlElementsSymbol].HTMLElement(tagName);
  };
  document.createElementNS = (namespace, tagName) => document.createElement(tagName);
  document.createDocumentFragment = () => document.createElement();
  document.createTextNode = text => new TextNode(text);
  document.createComment = comment => new CommentNode(comment);
  document.styleSheets = [];
  document.write = htmlString => {
    const childNodes = parse5.parseFragment(htmlString).childNodes.map(childNode => _fromAST(childNode, window, this));
    for (let i = 0; i < childNodes.length; i++) {
      document.body.appendChild(childNodes[i]);
    }
  };
  document.pointerLockElement = null;
  window.document = document;

  process.nextTick(async () => {
    document.readyState = 'complete';

    try {
      await _runHtml(document, window);
    } catch(err) {
      console.warn(err);
    }

    document.emit('readystatechange');
  });

  return document;
};
const _parseWindow = (s, options, parent, top) => {
  const window = _makeWindow(options, parent, top);
  const document = _parseDocument(s, options, window);
  window.document = document;
  return window;
};

const exokit = (s = '', options = {}) => {
  options.url = options.url || 'http://127.0.0.1/';
  options.baseUrl = options.baseUrl || options.url;
  options.dataPath = options.dataPath || __dirname;
  return _parseWindow(s, options);
};
exokit.fetch = src => fetch(src)
  .then(res => {
    if (res.status >= 200 && res.status < 300) {
      return res.text();
    } else {
      return Promise.reject(new Error('fetch got invalid status code: ' + res.status + ' : ' + src));
    }
  })
  .then(htmlString => {
    const parsedUrl = url.parse(src);
    return exokit(htmlString, {
      url: src,
      baseUrl: url.format({
        protocol: parsedUrl.protocol || 'http:',
        host: parsedUrl.host || '127.0.0.1',
      }),
    });
  });
exokit.THREE = THREE;
module.exports = exokit;

if (require.main === module) {
  if (process.argv.length === 3) {
    exokit.fetch(process.argv[2]);
  }
}
