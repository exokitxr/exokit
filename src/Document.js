const {HTMLElement, HTMLLoadableElement, Node} = require('./DOM');
const symbols = require('./symbols');

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

// FIXME: Temporary until we can refactor out into modules more and not have circular dependencies.
require('./GlobalContext').Document = Document;

class DocumentFragment extends HTMLElement {
  constructor() {
    super('DOCUMENTFRAGMENT');
  }

  get nodeType() {
    return Node.DOCUMENT_FRAGMENT_NODE;
  }
}
module.exports.DocumentFragment = DocumentFragment;
