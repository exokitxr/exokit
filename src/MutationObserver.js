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
    let bindings = [];

    const _recursiveBind = rootEl => {
      const _bind = el => {
        if (el.nodeType === Node.ELEMENT_NODE) {
          if (options.attributes) {
            const _attribute = (name, value, oldValue) => {
              this.handleAttribute(el, name, value, oldValue, options);
            };
            el.on('attribute', _attribute);
            bindings.push([el, 'attribute', _attribute]);
          }

          if (options.childList || options.subtree) {
            const _children = (addedNodes, removedNodes, previousSibling, nextSibling) => {
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
            };
            el.on('children', _children);
            bindings.push([el, 'children', _children]);
          }

          if (options.characterData) {
            const _value = () => {
              this.handleValue(el);
            };
            el.on('value', _value);
            bindings.push([el, 'value', _value]);
          }
        }
      };
      if (options.subtree) {
        rootEl.traverse(_bind);
      } else {
        _bind(rootEl);
      }
    };
    const _recursiveUnbind = rootEl => {
      const _unbind = el => {
        bindings = bindings.filter(binding => {
          const [el2, evt, fn] = binding;
          if (el2 === el) {
            el2.removeListener(evt, fn);
            return false;
          } else {
            return true;
          }
        });
      };
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
