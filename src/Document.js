const parse5 = require('parse5');

const DOM = require('./DOM');
const {Event} = require('./Event');
const GlobalContext = require('./GlobalContext');
const symbols = require('./symbols');
const utils = require('./utils');

const _parseDocument = (s, window) => {
  const ast = parse5.parse(s, {
    sourceCodeLocationInfo: true,
  });
  ast.tagName = 'document';
  return _parseDocumentAst(ast, window, true);
};
module.exports._parseDocument = _parseDocument;
GlobalContext._parseDocument = _parseDocument;

const _parseDocumentAst = (ast, window, uppercase) => {
  const document = GlobalContext._fromAST(ast, window, null, null, uppercase);
  return initDocument(document, window);
};
module.exports._parseDocumentAst = _parseDocumentAst;

/**
 * Initialize document instance with properties and methods.
 */
function initDocument (document, window) {
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
    const HTMLElementTemplate = window[symbols.htmlTagsSymbol][tagName];
    const element = HTMLElementTemplate ? new HTMLElementTemplate() : new DOM.HTMLElement(tagName);
    element.ownerDocument = document;
    return element;
  };
  document.createElementNS = (namespace, tagName) => document.createElement(tagName);
  document.createDocumentFragment = () => {
    const documentFragment = new DocumentFragment();
    documentFragment.ownerDocument = document;
    return documentFragment;
  };
  document.createRange = () => {
    const range = new Range();
    range.ownerDocument = document;
    return range;
  };
  document.createTextNode = text => new DOM.Text(text);
  document.createComment = comment => new DOM.Comment(comment);
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
  document.scripts = utils._makeHtmlCollectionProxy(document.documentElement, 'script');
  document.styleSheets = [];
  document.implementation = new DOMImplementation(window);
  document.resources = new GlobalContext.Resources(); // non-standard
  document.activeElement = body;
  document.open = () => {
    document.innerHTML = '';
  };
  document.close = () => {};
  document.write = htmlString => {
    const childNodes = parse5.parseFragment(htmlString, {
      locationInfo: true,
    }).childNodes.map(childNode => GlobalContext._fromAST(childNode, window, document.body, document, true));
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
  document[symbols.pointerLockElementSymbol] = null;
  document[symbols.fullscreenElementSymbol] = null;

  const runElQueue = [];
  const _addRun = fn => {
    (async () => {
      if (!document[symbols.runningSymbol]) {
        document[symbols.runningSymbol] = true;

        try {
          await fn();
        } catch(err) {
          console.warn(err.stack);
        }

        document[symbols.runningSymbol] = false;
        if (runElQueue.length > 0) {
          _addRun(runElQueue.shift());
        } else {
          document.emit('flush');
        }
      } else {
        runElQueue.push(fn);
      }
    })();
  };
  document[symbols.runningSymbol] = false;
  document[symbols.addRunSymbol] = _addRun;

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
    document.addEventListener('fullscreenchange', () => {
      const iframes = document.getElementsByTagName('iframe');
      for (let i = 0; i < iframes.length; i++) {
        const iframe = iframes[i];
        if (iframe.contentDocument) {
          iframe.contentDocument._emit('fullscreenchange');
        }
      }
    });
  }

  process.nextTick(async () => {
    if (body) {
      const bodyChildNodes = body.childNodes;
      body.childNodes = new DOM.NodeList();

      try {
        await GlobalContext._runHtml(document.head, window);
      } catch(err) {
        console.warn(err);
      }

      body.childNodes = bodyChildNodes;

      document.dispatchEvent(new Event('DOMContentLoaded', {target: document}));

      try {
        await GlobalContext._runHtml(document.body, window);
      } catch(err) {
        console.warn(err);
      }

      document.readyState = 'interactive';
      document.dispatchEvent(new Event('readystatechange', {target: document}));

      document.readyState = 'complete';
      document.dispatchEvent(new Event('readystatechange', {target: document}));

      document.dispatchEvent(new Event('load', {target: document}));
      window.dispatchEvent(new Event('load', {target: window}));

      const displays = window.navigator.getVRDisplaysSync();
      if (displays.length > 0) {
        const _initDisplays = () => {
          if (!_tryEmitDisplay()) {
            _delayFrames(() => {
              _tryEmitDisplay();
            }, 100);
          }
        };
        const _tryEmitDisplay = () => {
          const presentingDisplay = displays.find(display => display.isPresenting);
          if (presentingDisplay) {
            if (presentingDisplay.constructor.name === 'FakeVRDisplay') {
              _emitOneDisplay(presentingDisplay);
            }
            return true;
          } else {
            _emitOneDisplay(displays[0]);
            return false;
          }
        };
        const _emitOneDisplay = display => {
          const e = new window.Event('vrdisplayactivate');
          e.display = display;
          window.dispatchEvent(e);
        };
        const _delayFrames = (fn, n = 1) => {
          if (n === 0) {
            fn();
          } else {
            try {
            window.requestAnimationFrame(() => {
              _delayFrames(fn, n - 1);
            });
            } catch(err) {
              console.log(err.stack);
            }
          }
        };
        if (document.resources.resources.length === 0) {
          _initDisplays();
        } else {
          const _update = () => {
            if (document.resources.resources.length === 0) {
              _initDisplays();
              document.resources.removeEventListener('update', _update);
            }
          };
          document.resources.addEventListener('update', _update);
        }
      } else {
        await GlobalContext._runHtml(document, window);
      }
    }
  });

  return document;
}
module.exports.initDocument = initDocument;

class DocumentType {}
module.exports.DocumentType = DocumentType;

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
    return _parseDocument('', this._window);
  }

  hasFeature() {
    return false;
  }
}
module.exports.DOMImplementation = DOMImplementation;

class Document extends DOM.HTMLLoadableElement {
  constructor() {
    super('DOCUMENT');

    this.hidden = false;
  }

  get nodeType() {
    return DOM.Node.DOCUMENT_NODE;
  }

  get pointerLockElement() {
    if (this.defaultView.top === this.defaultView) {
      return this[symbols.pointerLockElementSymbol];
    } else {
      return this.defaultView.top.document.pointerLockElement;
    }
  }
  set pointerLockElement(pointerLockElement) {}
  get fullscreenElement() {
    if (this.defaultView.top === this.defaultView) {
      return this[symbols.fullscreenElementSymbol];
    } else {
      return this.defaultView.top.document.fullscreenElement;
    }
  }
  set fullscreenElement(fullscreenElement) {}

  exitPointerLock() {
    const topDocument = this.defaultView.top.document;

    if (topDocument[symbols.pointerLockElementSymbol] !== null) {
      topDocument[symbols.pointerLockElementSymbol] = null;

      process.nextTick(() => {
        topDocument._emit('pointerlockchange');
      });
    }
  }
  exitFullscreen() {
    const topDocument = this.defaultView.top.document;

    if (topDocument[symbols.fullscreenElementSymbol] !== null) {
      topDocument[symbols.fullscreenElementSymbol] = null;

      process.nextTick(() => {
        topDocument._emit('fullscreenchange');
      });
    }
  }
}
module.exports.Document = Document;
// FIXME: Temporary until refactor out into modules more and not have circular dependencies.
GlobalContext.Document = Document;

class DocumentFragment extends DOM.HTMLElement {
  constructor() {
    super('DOCUMENTFRAGMENT');
  }

  get nodeType() {
    return DOM.Node.DOCUMENT_FRAGMENT_NODE;
  }
}
module.exports.DocumentFragment = DocumentFragment;

class Range extends DocumentFragment {
  constructor() {
    super('RANGE');
  }

  createContextualFragment(str) {
    var fragment = this.ownerDocument.createDocumentFragment();
    fragment.innerHTML = str;
    return fragment;
  }
}
module.exports.Range = Range;
