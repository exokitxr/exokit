const USKeyboardLayout = require('./USKeyboardLayout');
const GlobalContext = require('./GlobalContext');

class EventTarget {
  constructor() {
    this._listeners = {};
  }

  addEventListener(event, listener) {
    if (typeof listener === 'function') {
      if (!this._listeners[event]) {
        this._listeners[event] = [];
      }
      this._listeners[event].push(listener);
    }
  }
  removeEventListener(event, listener) {
    if (typeof listener === 'function' && this._listeners[event]) {
      const index = this._listeners[event].indexOf(listener);
      if (index !== -1) {
        this._listeners[event].splice(index, 1);
      }
    }
  }

  dispatchEvent(event) {

    if (typeof event === 'string') {
      const listeners = this._listeners[event];


      if (listeners && listeners.length > 0) {
        const args = Array.from(arguments).slice(1);

        for (let i = 0; i < listeners.length; i++) {
          try {

            listeners[i].apply(this, args);
          } catch (err) {
            console.warn(err);
          }
        }
      }
    } else {
      event.target = this;

      const _emit = (node, event) => {
        const listeners = node._listeners[event.type];

        // console.log('dispatch event 2', this.tagName, event && event.type, listeners && listeners.length); // XXX

        if (listeners && listeners.length > 0) {
          event.currentTarget = this;

          for (let i = 0; i < listeners.length; i++) {
            try {
              listeners[i].call(this, event);
            } catch (err) {
              console.warn(err);
            }
          }

          event.currentTarget = null;
        }
      };
      const _recurse = (node, event) => {
        _emit(node, event);

        if (event.bubbles && node instanceof GlobalContext.Document) {
          _emit(node.defaultView, event);
        }

        if (event.bubbles && !event.propagationStopped && node.parentNode) {
          _recurse(node.parentNode, event);
        }
      };
      _recurse(this, event);
    }
  }
}
module.exports.EventTarget = EventTarget;

class Event {
  constructor(type, init = {}) {
    this.type = type;
    this.target = init.target !== undefined ? init.target : null;
    this.bubbles = init.bubbles !== undefined ? init.bubbles : false;
    this.cancelable = init.cancelable !== undefined ? init.cancelable : false;

    this.defaultPrevented = false;
    this.propagationStopped = false;
    this.currentTarget = null;
  }

  preventDefault() {
    this.defaultPrevented = true;
  }

  stopPropagation() {
    this.propagationStopped = true;
  }

  initEvent(type = '', bubbles = false, cancelable = false) {
    this.type = type;
    this.bubbles = bubbles;
    this.cancelable = cancelable;
  }
}
module.exports.Event = Event;

class KeyboardEvent extends Event {
  constructor(type, init = {}) {
    init.bubbles = true;
    init.cancelable = true;

    const findKeySpecByKeyCode = keyCode => {
      for (const k in USKeyboardLayout) {
        const keySpec = USKeyboardLayout[k];
        if (keySpec.keyCode === keyCode) {
          return keySpec;
        }
      }
      return null;
    };
    if (init.key === undefined || init.code === undefined) {
      const keySpec = findKeySpecByKeyCode(init.keyCode);
      if (keySpec) {
        init.key = keySpec.key;
        init.code = /^[a-z]$/i.test(keySpec.key) ? ('Key' + keySpec.key.toUpperCase()) : keySpec.key;
      }
    }

    super(type, init);

    KeyboardEvent.prototype.init.call(this, init);
  }

  init(init) {
    this.key = init.key !== undefined ? init.key : '';
    this.code = init.code !== undefined ? init.code : '';
    this.location = init.location !== undefined ? init.location : 0;
    this.ctrlKey = init.ctrlKey !== undefined ? init.ctrlKey : false;
    this.shiftKey = init.shiftKey !== undefined ? init.shiftKey : false;
    this.altKey = init.altKey !== undefined ? init.altKey : false;
    this.metaKey = init.metaKey !== undefined ? init.metaKey : false;
    this.repeat = init.repeat !== undefined ? init.repeat : false;
    this.isComposing = init.isComposing !== undefined ? init.isComposing : false;
    this.charCode = init.charCode !== undefined ? init.charCode : 0;
    this.keyCode = init.keyCode !== undefined ? init.keyCode : 0;
    this.which = init.which !== undefined ? init.which : 0;
  }

  initKeyboardEvent(type, canBubble, cancelable, view, charCode, keyCode, location, modifiersList, repeat) {
    this.type = type;

    const modifiers = modifiers.split(/\s/);
    const ctrlKey = modifiers.includes('Control') || modifiers.includes('AltGraph');
    const altKey = modifiers.includes('Alt') || modifiers.includes('AltGraph');
    const metaKey = modifiers.includes('Meta');

    this.init({
      charCode,
      keyCode,
      ctrlKey,
      altKey,
      metaKey,
      repeat,
    });
  }
}
module.exports.KeyboardEvent = KeyboardEvent;

class MouseEvent extends Event {
  constructor(type, init = {}) {
    init.bubbles = true;
    init.cancelable = true;
    super(type, init);

    MouseEvent.prototype.init.call(this, init);
  }

  init(init = {}) {
    this.screenX = init.screenX !== undefined ? init.screenX : 0;
    this.screenY = init.screenY !== undefined ? init.screenY : 0;
    this.clientX = init.clientX !== undefined ? init.clientX : 0;
    this.clientY = init.clientY !== undefined ? init.clientY : 0;
    this.pageX = init.pageX !== undefined ? init.pageX : 0;
    this.pageY = init.pageY !== undefined ? init.pageY : 0;
    this.offsetX = init.offsetX !== undefined ? init.offsetX : 0;
    this.offsetY = init.offsetY !== undefined ? init.offsetY : 0;
    this.movementX = init.movementX !== undefined ? init.movementX : 0;
    this.movementY = init.movementY !== undefined ? init.movementY : 0;
    this.ctrlKey = init.ctrlKey !== undefined ? init.ctrlKey : false;
    this.shiftKey = init.shiftKey !== undefined ? init.shiftKey : false;
    this.altKey = init.altKey !== undefined ? init.altKey : false;
    this.metaKey = init.metaKey !== undefined ? init.metaKey : false;
    this.button = init.button !== undefined ? init.button : 0;
    this.relatedTarget = init.relatedTarget !== undefined ? init.relatedTarget : null;
    this.region = init.region !== undefined ? init.region : null;
  }

  initMouseEvent(type, canBubble, cancelable, view, detail, screenX, screenY, clientX, clientY, ctrlKey, altKey, shiftKey, metaKey, button, relatedTarget) {
    this.type = type;

    this.init({
      screenX,
      screenY,
      clientX,
      clientY,
      ctrlKey,
      altKey,
      shiftKey,
      metaKey,
      button,
      relatedTarget,
    });
  }
}
module.exports.MouseEvent = MouseEvent;

class WheelEvent extends MouseEvent {
  constructor(type, init = {}) {
    init.bubbles = true;
    init.cancelable = true;
    super(type, init);

    this.deltaX = init.deltaX !== undefined ? init.deltaX : 0;
    this.deltaY = init.deltaY !== undefined ? init.deltaY : 0;
    this.deltaZ = init.deltaZ !== undefined ? init.deltaZ : 0;
    this.deltaMode = init.deltaMode !== undefined ? init.deltaMode : 0;
  }
}
WheelEvent.DOM_DELTA_PIXEL = 0x00;
WheelEvent.DOM_DELTA_LINE = 0x01;
WheelEvent.DOM_DELTA_PAGE = 0x02;
module.exports.WheelEvent = WheelEvent;

class DragEvent extends MouseEvent {
  constructor(type, init = {}) {
    super(type, init);

    DragEvent.prototype.init.call(this, init);
  }

  init(init = {}) {
    this.dataTransfer = init.dataTransfer !== undefined ? init.dataTransfer : null;
  }
}
module.exports.DragEvent = DragEvent;

class MessageEvent extends Event {
  constructor(data) {
    super('message');

    this.data = data;
  }
}
module.exports.MessageEvent = MessageEvent;

class ErrorEvent extends Event {
  constructor(type, init = {}) {
    super(type, init);
  }
}
module.exports.ErrorEvent = ErrorEvent;

class PromiseRejectionEvent extends Event {
  constructor(type, init = {}) {
    super(type, init);
  }
}
module.exports.PromiseRejectionEvent = PromiseRejectionEvent;

class CustomEvent extends Event {
  constructor(type, init = {}) {
    super(type, init);

    this.detail = init.detail !== undefined ? init.detail : null;
  }
}
module.exports.CustomEvent = CustomEvent;
