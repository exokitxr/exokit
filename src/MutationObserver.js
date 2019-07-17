const {Node, NodeList} = require('./DOM');
const {process} = global;

const emptyNodeList = new NodeList();

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
module.exports.MutationRecord = MutationRecord;

class MutationObserver {
  constructor(callback) {
    this.callback = callback;

    this.queue = [];
    this.bindings = new Map();
  }

  observe(el, options) {
    {
      const unbind = this.bindings.get(el);
      if (unbind) {
        unbind();
        this.bindings.delete(el);
      }
    }

    const unbind = this.bind(el, options);
    this.bindings.set(el, unbind);
  }

  disconnect() {
    for (const unbind of this.bindings.values()) {
      unbind();
    }
    this.bindings.clear();
  }

  takeRecords() {
    const oldQueue = this.queue.slice();
    this.queue.length = 0;
    return oldQueue;
  }

  bind(rootEl, options) {
    const bindings = new Map();
    
    const _addBinding = (el, evt, fn) => {
      el.on(evt, fn);

      let a = bindings.get(el);
      (a === undefined) && bindings.set(el, a = []);
      a.push([evt, fn]);
    };
    const _bind = el => {
      if (el.nodeType === Node.ELEMENT_NODE) {
        if (options.attributes) {
          _addBinding(el, 'attribute', (name, value, oldValue) => {
            this.handleAttribute(el, name, value, oldValue, options);
          });
        }

        if (options.childList || options.subtree) {
          _addBinding(el, 'children', (addedNodes, removedNodes, previousSibling, nextSibling) => {
            if (options.childList) {
              this.handleChildren(el, addedNodes, removedNodes, previousSibling, nextSibling);
            }

            if (options.subtree) {
              for (let i = 0; i < removedNodes.length; i++) {
                _recursiveUnbind(removedNodes[i]);
              }
              for (let i = 0; i < addedNodes.length; i++) {
                _recursiveBind(addedNodes[i]);
              }
            }
          });
        }

        if (options.characterData) {
          _addBinding(el, 'value', () => {
            this.handleValue(el);
          });
        }
      }
    };
    const _recursiveBind = rootEl => {
      if (options.subtree) {
        rootEl.traverse(_bind);
      } else {
        _bind(rootEl);
      }
    };
    const _unbind = el => {
      const a = bindings.get(el);
      if (a !== undefined) {
        for (let i = 0; i < a.length; i++) {
          const [evt, fn] = a[i];
          el.removeListener(evt, fn);
        }
        bindings.delete(el);
      }
    };
    const _recursiveUnbind = rootEl => {
      if (options.subtree) {
        rootEl.traverse(_unbind);
      } else {
        _unbind(rootEl);
      }
    };
    
    _recursiveBind(rootEl);
    
    return () => {
      for (let i = 0; i < bindings.length; i++) {
        const [el, evt, fn] = bindings[i];
        el.removeListener(evt, fn);
      }
      bindings.length = 0;
    };
  }

  flush() {
    if (this.queue.length > 0) {
      const oldQueue = this.queue.slice();
      this.queue.length = 0;
      this.callback(oldQueue, this);
    }
  }

  handleAttribute(el, name, value, oldValue, options) {
    // If observing document, only queue mutations if element is part of the DOM (#361).
    // if (this.element === el.ownerDocument && !el.ownerDocument.contains(el)) { return; }

    // Respect attribute filter.
    if (options.attributeFilter && !options.attributeFilter.includes(name)) {
      return;
    }

    this.queue.push(new MutationRecord('attributes', el, emptyNodeList, emptyNodeList, null, null, name, null, oldValue));
    process.nextTick(() => {
      this.flush();
    });
  }

  handleChildren(el, addedNodes, removedNodes, previousSibling, nextSibling) {
    // If observing document, only queue mutations if element is part of the DOM (#361).
    // if (this.element === el.ownerDocument && !el.ownerDocument.contains(el)) { return; }

    this.queue.push(new MutationRecord('childList', el, addedNodes, removedNodes, previousSibling, nextSibling, null, null, null));
    process.nextTick(() => {
      this.flush();
    });
  }

  handleValue(el) {
    // If observing document, only queue mutations if element is part of the DOM (#361).
    // if (this.element === el.ownerDocument && !el.ownerDocument.contains(el)) { return; }

    this.queue.push(new MutationRecord('characterData', el, emptyNodeList, emptyNodeList, null, null, null, null, null));
    process.nextTick(() => {
      this.flush();
    });
  }
}
module.exports.MutationObserver = MutationObserver;
