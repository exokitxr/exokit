const {Node, NodeList} = require('./DOM');
const {process} = global;

const emptyNodeList = new NodeList();

class MutationRecord {
  constructor(type, target, addedNodes, removedNodes, previousSibling, nextSibling,
              attributeName, attributeNamespace, oldValue) {
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
    this.callbacks = new WeakMap();
  }

  observe(el, options) {
    const oldOptions = this.bindings.get(el);
    if (oldOptions) {
      this.unbind(el, oldOptions);
      this.bindings.delete(el);
    }

    this.bind(el, options);
    this.bindings.set(el, options);
  }

  disconnect() {
    for (const [el, options] of this.bindings.entries()) {
      this.unbind(el, options);
    }
    this.bindings.clear();
  }

  takeRecords() {
    const oldQueue = this.queue.slice();
    this.queue.length = 0;
    return oldQueue;
  }

  bind(el, options) {
    const _bind = el => {
      if (el.nodeType === Node.ELEMENT_NODE) {
        let _attribute = null;
        let _children = null;
        let _value = null;

        if (options.attributes) {
          _attribute = (name, value, oldValue) => {
            this.handleAttribute(el, name, value, oldValue, options);
          };
          el.on('attribute', _attribute);
        }

        if (options.childList || options.subtree) {
          _children = (addedNodes, removedNodes, previousSibling, nextSibling) => {
            if (options.childList) {
              this.handleChildren(el, addedNodes, removedNodes, previousSibling, nextSibling);
            }

            if (options.subtree) {
              for (let i = 0; i < removedNodes.length; i++) {
                this.unbind(removedNodes[i], options);
              }
              for (let i = 0; i < addedNodes.length; i++) {
                this.bind(addedNodes[i], options);
              }
            }
          };
          el.on('children', _children);
        }

        if (options.characterData) {
          _value = () => {
            this.handleValue(el);
          };
          el.on('value', _value);
        }

        this.callbacks.set(el, [_attribute, _children, _value]);
      }
    };

    if (options.subtree) {
      el.traverse(_bind);
    } else {
      _bind(el);
    }
  }

  unbind(el, options) {
    const _unbind = el => {
      const callbacks = this.callbacks.get(el);
      if (callbacks) {
        const [
          _attribute,
          _children,
          _value,
        ] = callbacks;
        if (_attribute) {
          el.removeListener('attribute', _attribute);
        }
        if (_children) {
          el.removeListener('children', _children);
        }
        if (_value) {
          el.removeListener('value', _value);
        }
        this.callbacks.delete(el);
      }
    };

    if (options.subtree) {
      el.traverse(_unbind);
    } else {
      _unbind(el);
    }
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
