const ClassList = require('window-classlist');
const css = require('css');
const he = require('he');
const parse5 = require('parse5');
const parseIntStrict = require('parse-int');
const selector = require('window-selector');
const url = require('url');
const util = require('util');

const bindings = require('./bindings');
const {defaultCanvasSize} = require('./constants');
const {Event, EventTarget, MouseEvent, ErrorEvent} = require('./Event');
const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');
const urls = require('./urls').urls;
const utils = require('./utils');
const {_elementGetter, _elementSetter} = require('./utils');

he.encode.options.useNamedReferences = true;

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
    el.removeEventListener('load', load);
    el.removeEventListener('error', error);
  };
  el.addEventListener('load', load);
  el.addEventListener('error', error);
});

const EMPTY_ARRAY = [];

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

class NodeList extends Array {
  constructor(nodes) {
    super();

    if (Array.isArray(nodes)) {
      this.push.apply(this, nodes);
    }
  }

  item(k) {
    const v = this[k];
    return v !== undefined ? v : null;
  }
}
module.exports.NodeList = NodeList;

class HTMLCollection extends Array {
  constructor(nodes) {
    super();

    if (Array.isArray(nodes)) {
      this.push.apply(this, nodes);
    }
  }
}
module.exports.HTMLCollection = HTMLCollection;

class Node extends EventTarget {
  constructor() {
    super();

    this.parentNode = null;
    this.childNodes = new NodeList();
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
    return _cloneNode(deep, this);
  }

  _dispatchEventOnDocumentReady() {
    if (this.ownerDocument.readyState === 'complete') {
      this.dispatchEvent.apply(this, arguments);
    } else {
      const args = Array.from(arguments);

      const _readystatechange = () => {
        if (this.ownerDocument.readyState === 'complete') {
          this.ownerDocument.removeEventListener('readystatechange', _readystatechange);

          process.nextTick(() => {
            this.dispatchEvent.apply(this, args);
          });
        }
      };
      this.ownerDocument.addEventListener('readystatechange', _readystatechange);
    }
  }
}

/**
 * Clone node. Internal function to not expose the `sourceNode` and `parentNode` args
 * used to facilitate recursive cloning.
 *
 * @param {boolean} deep - Recursive.
 * @param {object} sourceNode - Node to clone.
 * @param {parentNode} parentNode - Used for recursive cloning to attach parent.
 */
function _cloneNode(deep, sourceNode, parentNode) {
  const clone = new sourceNode.constructor();
  clone.attrs = sourceNode.attrs;
  clone.ownerDocument = sourceNode.ownerDocument;
  clone.tagName = sourceNode.tagName;
  clone.value = sourceNode.value;

  // Link the parent.
  if (parentNode) { clone.parentNode = parentNode; }

  // Copy children.
  if (deep) {
    clone.childNodes = new NodeList(
      sourceNode.childNodes.map(childNode =>
        _cloneNode(true, childNode, clone)
      )
    );
  }

  return clone;
}

Node.ELEMENT_NODE = 1;
Node.TEXT_NODE = 3;
Node.PROCESSING_INSTRUCTION_NODE = 7;
Node.COMMENT_NODE = 8;
Node.DOCUMENT_NODE = 9;
Node.DOCUMENT_TYPE_NODE = 10;
Node.DOCUMENT_FRAGMENT_NODE = 11;
module.exports.Node = Node;

const _setAttributeRaw = (el, prop, value) => {
  if (prop === 'length') {
    el.attrs.length = value;
  } else {
    const attr = el.attrs.find(attr => attr.name === prop);
    if (!attr) {
      const attr = {
        name: prop,
        value,
      };
      el.attrs.push(attr);
      el.dispatchNodeEvent('attribute', prop, value, null);
    } else {
      const oldValue = attr.value;
      attr.value = value;
      el.dispatchNodeEvent('attribute', prop, value, oldValue);
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
      el.dispatchNodeEvent('attribute', prop, null, oldValue);
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
  const style = {};
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
      for (const k in style) {
        delete style[k];
      }
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
  const _getValue = (style, key) => {
    if (needsReset) {
      _reset();
    }
    return style[key];
  };
  const _setValue = (style, key, value) => {
    if (key === 'cssText') {
      el.setAttribute('style', value);
    } else {
      style[key] = value;
      el.setAttribute('style', _cssText(style));
    }
  };
  const _removeValue = (style, key, value) => {
    delete style[key];
    el.setAttribute('style', _cssText(style));
  };
  const _makeProxy = (style, getValue, setValue, removeValue) => new Proxy({}, {
    get(target, key) {
      if (key === 'reset') {
        return _reset;
      } else if (key === 'clone') {
        return () => {
          if (needsReset) {
            _reset();
          }

          const newStyle = {};
          for (const k in style) {
            const v = style[k];
            if (v !== undefined) {
              newStyle[k] = v;
            }
          }

          const _getValue = (style, key) => style[key];
          const _setValue = (style, key, value) => {
            style[key] = value;
          };
          const _removeValue = (style, key) => {
            delete style[value];
          };
          return _makeProxy(newStyle, _getValue, _setValue, _removeValue);
        };
      } else if (key === 'cssText') {
        return el.getAttribute('style') || '';
      } else if (key === 'length') {
        return Object.keys(style).length;
      } else if (key === 'getPropertyPriority') {
        return () => '';
      } else if (key === 'getPropertyValue') {
        return key => getValue(style, key);
      } else if (key === 'setProperty') {
        return (propertyName, value, priority) => {
          setValue(style, propertyName, value);
        };
      } else if (key === 'removeProperty') {
        return property => {
          removeValue(style, property);
        };
      } else if (key === 'item') {
        return k => {
          const n = parseInt(k, 10);
          if (!isNaN(n)) {
            const keys = Object.keys(style);
            const key = keys[n];
            return style[key];
          } else {
            return undefined;
          }
        };
      } else {
        return getValue(style, key);
      }
    },
    set(target, key, value) {
      setValue(style, key, value);
      return true;
    },
  });
  return _makeProxy(style, _getValue, _setValue, _removeValue);
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

    this.addEventListener('attribute', (name, value) => {
      if (name === 'id') {
        if (this.ownerDocument.defaultView[value] === undefined) {
          _defineId(this.ownerDocument.defaultView, value, this);
        }
      } else if (name === 'class' && this._classList) {
        this._classList.reset(value);
      }
    });
    this.addEventListener('children', (addedNodes, removedNodes, previousSibling, nextSiblings) => {
      for (let i = 0; i < addedNodes.length; i++) {
        addedNodes[i].dispatchNodeEvent('attached');
      }
      for (let i = 0; i < removedNodes.length; i++) {
        removedNodes[i].dispatchNodeEvent('removed');
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
    return attr !== undefined ? attr.value : null;
  }
  setAttribute(name, value) {
    value = value + '';
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

  getAttributeNames() {
    return this.attrs.map(attr => attr.name);
  }

  appendChild(childNode) {
    var newChildren = [];

    if (childNode.nodeType === Node.DOCUMENT_FRAGMENT_NODE) {
      // Appending DocumentFragment, append all children from node.
      let fragment = childNode;
      for (let i = 0; i < fragment.childNodes.length; i++) {
        let newChildNode = fragment.childNodes[i].cloneNode(true);
        this.childNodes.push(newChildNode);
        newChildNode.parentNode = this;
        newChildren.push(newChildNode);
      }
    } else {
      // Normal appendChild.
      if (childNode.parentNode) {
        childNode.parentNode.removeChild(childNode);
      }
      this.childNodes.push(childNode);
      childNode.parentNode = this;
      newChildren.push(childNode);
    }

    if (this._children) { this._children.update(); }

    // Notify observers.
    this.dispatchNodeEvent('children', newChildren, EMPTY_ARRAY, this.childNodes[this.childNodes.length - 2] || null, null);
    this.ownerDocument.dispatchNodeEvent('domchange');

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

      this.dispatchNodeEvent('children', [], [childNode], this.childNodes[index - 1] || null, this.childNodes[index] || null);
      this.ownerDocument.dispatchNodeEvent('domchange');

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

      this.dispatchNodeEvent('children', [newChild], [oldChild], this.childNodes[index - 1] || null, this.childNodes[index] || null);
      this.ownerDocument.dispatchNodeEvent('domchange');

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

      this.dispatchNodeEvent('children', [childNode], [], this.childNodes[index - 1] || null, this.childNodes[index + 1] || null);
      this.ownerDocument.dispatchNodeEvent('domchange');
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

      this.dispatchNodeEvent('children', [childNode], [], this.childNodes[index] || null, this.childNodes[index + 2] || null);
      this.ownerDocument.dispatchNodeEvent('domchange');
    }
  }
  insertAdjacentHTML(position, text) {
    const _getEls = text => parse5.parseFragment(text, {
      locationInfo: true,
    })
      .childNodes
      .map(childNode =>
        GlobalContext._fromAST(childNode, this.ownerDocument.defaultView, this, this.ownerDocument, true)
      );
    switch (position) {
      case 'beforebegin': {
        const index = this.parentNode.childNodes.indexOf(this);
        const newChildNodes = _getEls(text);
        this.parentNode.childNodes.splice.apply(this.parentNode.childNodes, [
          index,
          0,
        ].concat(newChildNodes));
        this.parentNode.dispatchNodeEvent('children', newChildNodes, [], null, null);
        break;
      }
      case 'afterbegin': {
        const newChildNodes = _getEls(text);
        this.childNodes.splice.apply(this.childNodes, [
          0,
          0,
        ].concat(newChildNodes));
        this.dispatchNodeEvent('children', newChildNodes, [], null, null);
        break;
      }
      case 'beforeend': {
        const newChildNodes = _getEls(text);
        this.childNodes.splice.apply(this.childNodes, [
          this.childNodes.length,
          0,
        ].concat(newChildNodes));
        this.dispatchNodeEvent('children', newChildNodes, [], null, null);
        break;
      }
      case 'afterend': {
        const index = this.parentNode.childNodes.indexOf(this);
        const newChildNodes = _getEls(text);
        this.parentNode.childNodes.splice.apply(this.parentNode.childNodes, [
          index + 1,
          0,
        ].concat(newChildNodes));
        this.parentNode.dispatchNodeEvent('children', newChildNodes, [], null, null);
        break;
      }
      default: {
        throw new Error('invalid position: ' + position);
        break;
      }
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
        _setAttributeRaw(this, 'class', className);
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
    for (let el = this; el; el = el.parentNode) {
      if (el.matches(s)) {
        return el;
      }
    }
    return null;
  }

  getBoundingClientRect() {
    return new DOMRect(0, 0, this.clientWidth, this.clientHeight);
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
    const fontFamily = style.fontFamily;
    if (fontFamily) {
       if (fontFamily === 'sans-serif') {
         return 0;
       } else {
         return _hash(fontFamily) * _hash(this.innerHTML);
       }
    } else {
      let result = 1;
      this.traverse(el => {
        if (el.tagName === 'CANVAS' || el.tagName === 'IMAGE' || el.tagName === 'VIDEO') {
          result = Math.max(el.width, result);
          return true;
        }
			});
      return result / this.ownerDocument.defaultView.devicePixelRatio;
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
    return result / this.ownerDocument.defaultView.devicePixelRatio;
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
    const newChildNodes = new NodeList(
      parse5.parseFragment(innerHTML, {
        locationInfo: true,
      })
        .childNodes
        .map(childNode =>
          GlobalContext._fromAST(childNode, this.ownerDocument.defaultView, this, this.ownerDocument, true)
        )
    );
    this.childNodes = newChildNodes;

    if (this._children) {
      this._children.update();
    }

    this.dispatchNodeEvent('children', newChildNodes, oldChildNodes, null, null);
    this.ownerDocument.dispatchNodeEvent('domchange');

    _promiseSerial(newChildNodes.map(childNode => () => GlobalContext._runHtml(childNode, this.ownerDocument.defaultView)))
      .catch(err => {
        console.warn(err);
      });

    this.dispatchNodeEvent('innerHTML', innerHTML);
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

  /**
   * Also the output when logging to console or debugger.
   */
  get outerHTML() {
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

  requestPointerLock() {
    const topDocument = this.ownerDocument.defaultView.top.document;

    if (topDocument[symbols.pointerLockElementSymbol] === null) {
      topDocument[symbols.pointerLockElementSymbol] = this;

      process.nextTick(() => {
        topDocument.dispatchNodeEvent('pointerlockchange');
      });
    }
  }

  requestFullscreen() {
    const topDocument = this.ownerDocument.defaultView.top.document;

    if (topDocument[symbols.fullscreenElementSymbol] === null) {
      topDocument[symbols.fullscreenElementSymbol] = this;

      process.nextTick(() => {
        topDocument.dispatchNodeEvent('fullscreenchange');
      });
    }
  }

  /**
   * For logging to console or debugger.
   */
  [util.inspect.custom]() {
    return this.outerHTML;
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
module.exports.Element = Element;

class HTMLElement extends Element {
  constructor(tagName = 'DIV', attrs = [], value = '', location = null) {
    super(tagName, attrs, value, location);

    this._style = null;
    this[symbols.computedStyleSymbol] = null;

    this.addEventListener('attribute', (name, value) => {
      if (name === 'class' && this._classList) {
        this._classList.reset(value);
      } else if (name === 'style') {
        if (this._style) {
          this._style.reset();
        }
        if (this[symbols.computedStyleSymbol]) {
          this[symbols.computedStyleSymbol] = null;
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
module.exports.HTMLElement = HTMLElement;

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
module.exports.HTMLAnchorElement = HTMLAnchorElement;

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
module.exports.HTMLLoadableElement = HTMLLoadableElement;

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
module.exports.HTMLBodyElement = HTMLBodyElement;

class HTMLStyleElement extends HTMLLoadableElement {
  constructor(attrs = [], value = '', location = null) {
    super('STYLE', attrs, value, location);

    this.stylesheet = null;

    this.addEventListener('innerHTML', innerHTML => {
      Promise.resolve()
        .then(() => css.parse(innerHTML).stylesheet)
        .then(stylesheet => {
          this.stylesheet = stylesheet;
          GlobalContext.styleEpoch++;
          this.dispatchEvent(new Event('load', {target: this}));
        })
        .catch(err => {
          const e = new ErrorEvent('error', {target: this});
          e.message = err.message;
          e.stack = err.stack;
          this.dispatchEvent(e);
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
    this.dispatchNodeEvent('innerHTML', innerHTML);
  }

  [symbols.runSymbol]() {
    let running = false;
    if (this.childNodes.length > 0) {
      this.innerHTML = this.childNodes[0].value;
      running = true;
    }
    return running ? _loadPromise(this) : Promise.resolve();
  }
}
module.exports.HTMLStyleElement = HTMLStyleElement;

class HTMLLinkElement extends HTMLLoadableElement {
  constructor(attrs = [], value = '', location = null) {
    super('LINK', attrs, value, location);

    this.stylesheet = null;

    this.addEventListener('attribute', (name, value) => {
      if (name === 'href' && this.isRunnable()) {
        const url = value;
        this.ownerDocument.defaultView.fetch(url)
          .then(res => {
            if (res.status >= 200 && res.status < 300) {
              return res.text();
            } else {
              return Promise.reject(new Error('link href got invalid status code: ' + res.status + ' : ' + url));
            }
          })
          .then(s => css.parse(s).stylesheet)
          .then(stylesheet => {
            this.stylesheet = stylesheet;
            GlobalContext.styleEpoch++;

            this.dispatchEvent(new Event('load', {target: this}));
          })
          .catch(err => {
            const e = new ErrorEvent('error', {target: this});
            e.message = err.message;
            e.stack = err.stack;
            this.dispatchEvent(e);
          });
      }
    });
  }

  get rel() {
    return this.getAttribute('rel') || '';
  }
  set rel(rel) {
    rel = rel + '';
    this.setAttribute('rel', rel);
  }

  get href() {
    return this.getAttribute('href') || '';
  }
  set href(href) {
    href = href + '';
    this.setAttribute('href', href);
  }

  get type() {
    type = type + '';
    return this.getAttribute('type') || '';
  }
  set type(type) {
    this.setAttribute('type', type);
  }

  isRunnable() {
    return this.rel === 'stylesheet';
  }

  [symbols.runSymbol]() {
    let running = false;
    if (this.isRunnable()) {
      const hrefAttr = this.attributes.href;
      if (hrefAttr) {
        this.dispatchNodeEvent('attribute', 'href', hrefAttr.value);
        running = true;
      }
    }
    return running ? _loadPromise(this) : Promise.resolve();
  }
}
module.exports.HTMLLinkElement = HTMLLinkElement;

class HTMLScriptElement extends HTMLLoadableElement {
  constructor(attrs = [], value = '', location = null) {
    super('SCRIPT', attrs, value, location);

    this.readyState = null;

    const _isAttached = () => {
      for (let el = this; el; el = el.parentNode) {
        if (el === el.ownerDocument) {
          return true;
        }
      }
      return false;
    };
    const _loadRun = async => {
      this.readyState = 'loading';

      if (!async) {
        this.ownerDocument[symbols.addRunSymbol](this.loadRunNow.bind(this));
      } else {
        this.loadRunNow();
      }
    };
    this.addEventListener('attribute', (name, value) => {
      if (name === 'src' && value && this.isRunnable() && _isAttached() && this.readyState === null) {
        const async = this.getAttribute('async');
        _loadRun(async !== null ? async !== 'false' : false);
      }
    });
    this.addEventListener('attached', () => {
      if (this.src && this.isRunnable() && _isAttached() && this.readyState === null) {
        const async = this.getAttribute('async');
        _loadRun(async !== null ? async !== 'false' : true);
      }
    });
    this.addEventListener('innerHTML', innerHTML => {
      if (this.isRunnable() && _isAttached() && this.readyState === null) {
        this.runNow();
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

  get async() {
    const async = this.getAttribute('async');
    return async === null || async !== 'false';
  }
  set async(async) {
    async = async + '';
    this.setAttribute('async', async);
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

  get innerHTML() {
    return parse5.serialize(this);
  }
  set innerHTML(innerHTML) {
    innerHTML = innerHTML + '';

    this.childNodes = new NodeList([new Text(innerHTML)]);
    this.dispatchNodeEvent('innerHTML', innerHTML);
  }

  isRunnable() {
    const {type} = this;
    return !type || /^(?:(?:text|application)\/javascript|application\/ecmascript)$/.test(type);
  }

  loadRunNow() {
    const resource = this.ownerDocument.resources.addResource();

    const url = this.src;
    return this.ownerDocument.defaultView.fetch(url)
      .then(res => {
        if (res.status >= 200 && res.status < 300) {
          return res.text();
        } else {
          return Promise.reject(new Error('script src got invalid status code: ' + res.status + ' : ' + url));
        }
      })
      .then(s => {
        utils._runJavascript(s, this.ownerDocument.defaultView, url);

        this.readyState = 'complete';

        this.dispatchEvent(new Event('load', {target: this}));
      })
      .catch(err => {
        this.readyState = 'complete';

        const e = new ErrorEvent('error', {target: this});
        e.message = err.message;
        e.stack = err.stack;
        this.dispatchEvent(e);
      })
      .finally(() => {
        setImmediate(() => {
          resource.setProgress(1);
        });
      });
  }

  runNow() {
    const innerHTML = this.childNodes[0].value;
    const window = this.ownerDocument.defaultView;
    utils._runJavascript(innerHTML, window, window.location.href, this.location && this.location.line !== null ? this.location.line - 1 : 0, this.location && this.location.col !== null ? this.location.col - 1 : 0);

    this.readyState = 'complete';

    const resource = this.ownerDocument.resources.addResource();

    setImmediate(() => {
      this.dispatchEvent(new Event('load', {target: this}));

      resource.setProgress(1);
    });
  }

  [symbols.runSymbol]() {
    if (this.isRunnable()) {
      const srcAttr = this.attributes.src;
      if (srcAttr) {
        return this.loadRunNow();
      } else if (this.childNodes.length > 0) {
        return this.runNow();
      }
    }
    return Promise.resolve();
  }
}
module.exports.HTMLScriptElement = HTMLScriptElement;

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

  [symbols.runSymbol]() {
    const srcAttr = this.attributes.src;
    if (srcAttr) {
      this.dispatchNodeEvent('attribute', 'src', srcAttr.value);
    }
    return Promise.resolve();
  }
}
module.exports.HTMLSrcableElement = HTMLSrcableElement;

class HTMLMediaElement extends HTMLSrcableElement {
  constructor(tagName = null, attrs = [], value = '', location = null) {
    super(tagName, attrs, value, location);

    this._startTime = 0;
    this._startTimestamp = null;
  }

  play() {
    this._startTimestamp = Date.now();
  }
  pause() {}
  load() {}

  get paused() {
    return true;
  }
  set paused(paused) {
    this._startTime = this.currentTime;
    this._startTimestamp = null;
  }
  get currentTime() {
    return this._startTime + (this._startTimestamp !== null ? (Date.now() - this._startTimestamp) : 0);
  }
  set currentTime(currentTime) {}
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
module.exports.HTMLMediaElement = HTMLMediaElement;

class HTMLSourceElement extends HTMLSrcableElement {
  constructor(attrs = [], value = '', location = null) {
    super('SOURCE', attrs, value, location);
  }
}
module.exports.HTMLSourceElement = HTMLSourceElement;

class SVGElement {}
module.exports.SVGElement = SVGElement;

class HTMLIFrameElement extends HTMLSrcableElement {
  constructor(attrs = [], value = '', location = null) {
    super('IFRAME', attrs, value, location);

    this.contentWindow = null;
    this.contentDocument = null;
    this.live = true;

    this.addEventListener('attribute', (name, value) => {
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
              const options = parentWindow[symbols.optionsSymbol];

              url = utils._makeNormalizeUrl(options.baseUrl)(url);
              const contentWindow = GlobalContext._makeWindow({
                url,
                baseUrl: url,
                dataPath: options.dataPath,
              }, parentWindow, parentWindow.top);
              const contentDocument = GlobalContext._parseDocument(htmlString, contentWindow);
              contentDocument.hidden = this.hidden;

              contentWindow.document = contentDocument;

              this.contentWindow = contentWindow;
              this.contentDocument = contentDocument;

              contentDocument.addEventListener('framebuffer', framebuffer => {
                this.dispatchNodeEvent('framebuffer', framebuffer);
              });
              contentWindow.addEventListener('destroy', e => {
                parentWindow.dispatchNodeEvent('destroy', e);
              });

              this.dispatchEvent(new Event('load', {target: this}));
            }
          })
          .catch(err => {
            console.warn(err);

            this.dispatchEvent(new Event('load', {target: this}));
          })
          .finally(() => {
            setImmediate(() => {
              resource.setProgress(1);
            });
          });
      } else if (name === 'hidden') {
        if (this.contentDocument) {
          this.contentDocument.hidden = value;
        }
      }
    });
    this.addEventListener('destroy', () => {
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
      this.dispatchNodeEvent('destroy');
      this.live = false;
    }
  }
}
module.exports.HTMLIFrameElement = HTMLIFrameElement;

class HTMLCanvasElement extends HTMLElement {
  constructor(attrs = [], value = '', location = null) {
    super('CANVAS', attrs, value, location);

    this._context = null;

    this.addEventListener('attribute', (name, value) => {
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
    return this.width / this.ownerDocument.defaultView.devicePixelRatio;
  }
  set clientWidth(clientWidth) {}
  get clientHeight() {
    return this.height / this.ownerDocument.defaultView.devicePixelRatio;
  }
  set clientHeight(clientHeight) {}

  getBoundingClientRect() {
    return new DOMRect(0, 0, this.clientWidth, this.clientHeight);
  }

  get data() {
    return (this._context && this._context.data) || null;
  }
  set data(data) {}

  getContext(contextType) {
    if (contextType === '2d') {
      if (this._context && this._context.constructor && this._context.constructor.name !== 'CanvasRenderingContext2D') {
        this._context.destroy();
        this._context = null;
      }
      if (this._context === null) {
        this._context = new GlobalContext.CanvasRenderingContext2D(this);
      }
    } else if (contextType === 'webgl' || contextType === 'webgl2' || contextType === 'xrpresent') {
      if (this._context && this._context.constructor && this._context.constructor.name !== 'WebGLRenderingContext' && this._context.constructor.name !== 'WebGL2RenderingContext') {
        this._context.destroy();
        this._context = null;
      }
      if (this._context === null) {
        if (GlobalContext.args.webgl === '1') {
          if (contextType === 'webgl' || contextType === 'xrpresent') {
            this._context = new WebGLRenderingContext(this);
          }
        } else {
          if (contextType === 'webgl') {
            this._context = new WebGLRenderingContext(this);
          } else {
            this._context = new WebGL2RenderingContext(this);
          }
        }
      }
    } else {
      if (this._context) {
        this._context.destroy();
        this._context = null;
      }
    }
    return this._context;
  }

  toDataURL() {
    if (!this._context) {
      this.getContext('2d');
    }
    return this._context.toDataURL();
  }

  captureStream(frameRate) {
    return {}; // XXX
  }
}
module.exports.HTMLCanvasElement = HTMLCanvasElement;

class HTMLTemplateElement extends HTMLElement {
  constructor(attrs = [], value = '', location = null) {
    super('TEMPLATE', attrs, value, location);

    this._childNodes = new NodeList();
  }

  get content() {
    const content = new GlobalContext.DocumentFragment();
    content.ownerDocument = this.ownerDocument;
    content.childNodes = new NodeList(this.childNodes);
    return content;
  }
  set content(content) {}

  get childNodes() {
    return new NodeList();
  }
  set childNodes(childNodes) {
    this._childNodes = childNodes; }

  get children() {
    return [];
  }
  set children(children) {}

  get innerHTML() {
    return parse5.serialize({
      tagName: this.tagName,
      attrs: this.attrs,
      value: this.value,
      childNodes: this._childNodes,
    });
  }
  set innerHTML(innerHTML) {
    super.innerHTML = innerHTML;
  }
}
module.exports.HTMLTemplateElement = HTMLTemplateElement;

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

    this.dispatchNodeEvent('value');
  }

  get data() {
    return this.value;
  }
  set data(data) {
    this.value = data;

    this.dispatchNodeEvent('value');
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
module.exports.CharacterNode = CharacterNode;

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
module.exports.Text = Text;

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
module.exports.Comment = Comment;

class HTMLImageElement extends HTMLSrcableElement {
  constructor(...args) {
    if (typeof arguments[0] === 'number') {
      const [width = 0, height = 0] = arguments;
      return new HTMLImageElement([
        {
          name: 'width',
          value: width + '',
        },
        {
          name: 'height',
          value: height + '',
        },
      ], '', null);
    }
    const [attrs = [], value = '', location = null] = arguments;
    super('IMG', attrs, value, location);

    this.image = new bindings.nativeImage();

    this.addEventListener('attribute', (name, value) => {
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
          .then(arrayBuffer => new Promise((accept, reject) => {
            this.image.load(arrayBuffer, err => {
              if (!err) {
                accept();
              } else {
                reject(new Error(`failed to decode image: ${err.message} (url: ${JSON.stringify(src)}, size: ${arrayBuffer.byteLength}, message: ${err})`));
              }
            });
          }))
          .then(() => {
            this._dispatchEventOnDocumentReady(new Event('load', {target: this}));
          })
          .catch(err => {
            console.warn('failed to load image:', src);

            const e = new ErrorEvent('error', {target: this});
            e.message = err.message;
            e.stack = err.stack;
            this._dispatchEventOnDocumentReady(e);
          })
          .finally(() => {
            setImmediate(() => {
              resource.setProgress(1);
            });
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
module.exports.HTMLImageElement = HTMLImageElement;

class TimeRanges {
  constructor(ranges) {
    this._ranges = ranges;
  }

  start(i) {
    return this._ranges[i][0];
  }

  end(i) {
    return this._ranges[i][1];
  }

  get length() {
    return this._ranges.length;
  }
  set length(length) {}
}
module.exports.TimeRanges = TimeRanges;

class HTMLAudioElement extends HTMLMediaElement {
  constructor(attrs = [], value = '') {
    super('AUDIO', attrs, value);

    this.readyState = HTMLMediaElement.HAVE_NOTHING;
    this.audio = new bindings.nativeAudio.Audio();

    this.addEventListener('attribute', (name, value) => {
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

            const progressEvent = new Event('progress', {target: this});
            progressEvent.loaded = 1;
            progressEvent.total = 1;
            progressEvent.lengthComputable = true;
            this.dispatchEvent(progressEvent);

            this._dispatchEventOnDocumentReady(new Event('canplay', {target: this}));
            this._dispatchEventOnDocumentReady(new Event('canplaythrough', {target: this}));
          })
          .catch(err => {
            console.warn('failed to load audio:', src);

            const e = new ErrorEvent('error', {target: this});
            e.message = err.message;
            e.stack = err.stack;
            this._dispatchEventOnDocumentReady(e);
          })
          .finally(() => {
            setImmediate(() => {
              resource.setProgress(1);
            });
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

  get buffered() {
    return new TimeRanges([0, this.duration]);
  }
  set buffered(buffered) {}
};
module.exports.HTMLAudioElement = HTMLAudioElement;

class HTMLVideoElement extends HTMLMediaElement {
  constructor(attrs = [], value = '', location = null) {
    super('VIDEO', attrs, value, location);

    this.readyState = HTMLMediaElement.HAVE_NOTHING;
    this.data = new Uint8Array(0);

    this.addEventListener('attribute', (name, value) => {
      if (name === 'src') {
        this.readyState = HTMLMediaElement.HAVE_ENOUGH_DATA;

        if (urls.has(value)) {
          const blob = urls.get(value);
          if (blob instanceof Bindings.bindings.nativeVideo.VideoDevice) {
            this.video = blob;
          }
        }

        const resource = this.ownerDocument.resources.addResource();

        setImmediate(() => {
          const progressEvent = new Event('progress', {target: this});
          progressEvent.loaded = 1;
          progressEvent.total = 1;
          progressEvent.lengthComputable = true;
          this.dispatchEvent(progressEvent);

          this._dispatchEventOnDocumentReady(new Event('canplay', {target: this}));
          this._dispatchEventOnDocumentReady(new Event('canplaythrough', {target: this}));

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

  get buffered() {
    return new TimeRanges([0, this.duration]);
  }
  set buffered(buffered) {}

  update() {
    if (this.video) {
      this.video.update();
    }
  }
}
module.exports.HTMLVideoElement = HTMLVideoElement;

/*
class HTMLVideoElement extends HTMLMediaElement {
  constructor(attrs = [], value = '') {
    super('VIDEO', attrs, value);

    this.readyState = HTMLMediaElement.HAVE_NOTHING;
    this.video = new bindings.nativeVideo.Video();

    this.addEventListener('attribute', (name, value) => {
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
            this.dispatchNodeEvent('canplay');
            this.dispatchNodeEvent('canplaythrough');
          })
          .catch(err => {
            this.dispatchNodeEvent('error', err);
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
            this.removeEventListener('canplay', canplay);
            this.removeEventListener('error', error);
          };
          this.addEventListener('canplay', canplay);
          this.addEventListener('error', error);
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

  run() {
    let running = false;

    let sources;
    const srcAttr = this.attributes.src;
    if (srcAttr) {
      this.dispatchNodeEvent('attribute', 'src', srcAttr.value);
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
}
*/

function _hash(s) {
  let result = 0;
  for (let i = 0; i < s.length; i++) {
    result += s.codePointAt(i);
  }
  return result;
}
