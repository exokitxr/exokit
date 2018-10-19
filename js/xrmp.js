(() => {

const xrmp = ((
  module = ({
    exports: {},
  })
) => {

const events = (() => {

const module = {
  exports: {},
};

// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

var objectCreate = Object.create || objectCreatePolyfill
var objectKeys = Object.keys || objectKeysPolyfill
var bind = Function.prototype.bind || functionBindPolyfill

function EventEmitter() {
  if (!this._events || !Object.prototype.hasOwnProperty.call(this, '_events')) {
    this._events = objectCreate(null);
    this._eventsCount = 0;
  }

  this._maxListeners = this._maxListeners || undefined;
}
module.exports = EventEmitter;

// Backwards-compat with node 0.10.x
EventEmitter.EventEmitter = EventEmitter;

EventEmitter.prototype._events = undefined;
EventEmitter.prototype._maxListeners = undefined;

// By default EventEmitters will print a warning if more than 10 listeners are
// added to it. This is a useful default which helps finding memory leaks.
var defaultMaxListeners = 10;

var hasDefineProperty;
try {
  var o = {};
  if (Object.defineProperty) Object.defineProperty(o, 'x', { value: 0 });
  hasDefineProperty = o.x === 0;
} catch (err) { hasDefineProperty = false }
if (hasDefineProperty) {
  Object.defineProperty(EventEmitter, 'defaultMaxListeners', {
    enumerable: true,
    get: function() {
      return defaultMaxListeners;
    },
    set: function(arg) {
      // check whether the input is a positive number (whose value is zero or
      // greater and not a NaN).
      if (typeof arg !== 'number' || arg < 0 || arg !== arg)
        throw new TypeError('"defaultMaxListeners" must be a positive number');
      defaultMaxListeners = arg;
    }
  });
} else {
  EventEmitter.defaultMaxListeners = defaultMaxListeners;
}

// Obviously not all Emitters should be limited to 10. This function allows
// that to be increased. Set to zero for unlimited.
EventEmitter.prototype.setMaxListeners = function setMaxListeners(n) {
  if (typeof n !== 'number' || n < 0 || isNaN(n))
    throw new TypeError('"n" argument must be a positive number');
  this._maxListeners = n;
  return this;
};

function $getMaxListeners(that) {
  if (that._maxListeners === undefined)
    return EventEmitter.defaultMaxListeners;
  return that._maxListeners;
}

EventEmitter.prototype.getMaxListeners = function getMaxListeners() {
  return $getMaxListeners(this);
};

// These standalone emit* functions are used to optimize calling of event
// handlers for fast cases because emit() itself often has a variable number of
// arguments and can be deoptimized because of that. These functions always have
// the same number of arguments and thus do not get deoptimized, so the code
// inside them can execute faster.
function emitNone(handler, isFn, self) {
  if (isFn)
    handler.call(self);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].call(self);
  }
}
function emitOne(handler, isFn, self, arg1) {
  if (isFn)
    handler.call(self, arg1);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].call(self, arg1);
  }
}
function emitTwo(handler, isFn, self, arg1, arg2) {
  if (isFn)
    handler.call(self, arg1, arg2);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].call(self, arg1, arg2);
  }
}
function emitThree(handler, isFn, self, arg1, arg2, arg3) {
  if (isFn)
    handler.call(self, arg1, arg2, arg3);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].call(self, arg1, arg2, arg3);
  }
}

function emitMany(handler, isFn, self, args) {
  if (isFn)
    handler.apply(self, args);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].apply(self, args);
  }
}

EventEmitter.prototype.emit = function emit(type) {
  var er, handler, len, args, i, events;
  var doError = (type === 'error');

  events = this._events;
  if (events)
    doError = (doError && events.error == null);
  else if (!doError)
    return false;

  // If there is no 'error' event listener then throw.
  if (doError) {
    if (arguments.length > 1)
      er = arguments[1];
    if (er instanceof Error) {
      throw er; // Unhandled 'error' event
    } else {
      // At least give some kind of context to the user
      var err = new Error('Unhandled "error" event. (' + er + ')');
      err.context = er;
      throw err;
    }
    return false;
  }

  handler = events[type];

  if (!handler)
    return false;

  var isFn = typeof handler === 'function';
  len = arguments.length;
  switch (len) {
      // fast cases
    case 1:
      emitNone(handler, isFn, this);
      break;
    case 2:
      emitOne(handler, isFn, this, arguments[1]);
      break;
    case 3:
      emitTwo(handler, isFn, this, arguments[1], arguments[2]);
      break;
    case 4:
      emitThree(handler, isFn, this, arguments[1], arguments[2], arguments[3]);
      break;
      // slower
    default:
      args = new Array(len - 1);
      for (i = 1; i < len; i++)
        args[i - 1] = arguments[i];
      emitMany(handler, isFn, this, args);
  }

  return true;
};

function _addListener(target, type, listener, prepend) {
  var m;
  var events;
  var existing;

  if (typeof listener !== 'function')
    throw new TypeError('"listener" argument must be a function');

  events = target._events;
  if (!events) {
    events = target._events = objectCreate(null);
    target._eventsCount = 0;
  } else {
    // To avoid recursion in the case that type === "newListener"! Before
    // adding it to the listeners, first emit "newListener".
    if (events.newListener) {
      target.emit('newListener', type,
          listener.listener ? listener.listener : listener);

      // Re-assign `events` because a newListener handler could have caused the
      // this._events to be assigned to a new object
      events = target._events;
    }
    existing = events[type];
  }

  if (!existing) {
    // Optimize the case of one listener. Don't need the extra array object.
    existing = events[type] = listener;
    ++target._eventsCount;
  } else {
    if (typeof existing === 'function') {
      // Adding the second element, need to change to array.
      existing = events[type] =
          prepend ? [listener, existing] : [existing, listener];
    } else {
      // If we've already got an array, just append.
      if (prepend) {
        existing.unshift(listener);
      } else {
        existing.push(listener);
      }
    }

    // Check for listener leak
    if (!existing.warned) {
      m = $getMaxListeners(target);
      if (m && m > 0 && existing.length > m) {
        existing.warned = true;
        var w = new Error('Possible EventEmitter memory leak detected. ' +
            existing.length + ' "' + String(type) + '" listeners ' +
            'added. Use emitter.setMaxListeners() to ' +
            'increase limit.');
        w.name = 'MaxListenersExceededWarning';
        w.emitter = target;
        w.type = type;
        w.count = existing.length;
        if (typeof console === 'object' && console.warn) {
          console.warn('%s: %s', w.name, w.message);
        }
      }
    }
  }

  return target;
}

EventEmitter.prototype.addListener = function addListener(type, listener) {
  return _addListener(this, type, listener, false);
};

EventEmitter.prototype.on = EventEmitter.prototype.addListener;

EventEmitter.prototype.prependListener =
    function prependListener(type, listener) {
      return _addListener(this, type, listener, true);
    };

function onceWrapper() {
  if (!this.fired) {
    this.target.removeListener(this.type, this.wrapFn);
    this.fired = true;
    switch (arguments.length) {
      case 0:
        return this.listener.call(this.target);
      case 1:
        return this.listener.call(this.target, arguments[0]);
      case 2:
        return this.listener.call(this.target, arguments[0], arguments[1]);
      case 3:
        return this.listener.call(this.target, arguments[0], arguments[1],
            arguments[2]);
      default:
        var args = new Array(arguments.length);
        for (var i = 0; i < args.length; ++i)
          args[i] = arguments[i];
        this.listener.apply(this.target, args);
    }
  }
}

function _onceWrap(target, type, listener) {
  var state = { fired: false, wrapFn: undefined, target: target, type: type, listener: listener };
  var wrapped = bind.call(onceWrapper, state);
  wrapped.listener = listener;
  state.wrapFn = wrapped;
  return wrapped;
}

EventEmitter.prototype.once = function once(type, listener) {
  if (typeof listener !== 'function')
    throw new TypeError('"listener" argument must be a function');
  this.on(type, _onceWrap(this, type, listener));
  return this;
};

EventEmitter.prototype.prependOnceListener =
    function prependOnceListener(type, listener) {
      if (typeof listener !== 'function')
        throw new TypeError('"listener" argument must be a function');
      this.prependListener(type, _onceWrap(this, type, listener));
      return this;
    };

// Emits a 'removeListener' event if and only if the listener was removed.
EventEmitter.prototype.removeListener =
    function removeListener(type, listener) {
      var list, events, position, i, originalListener;

      if (typeof listener !== 'function')
        throw new TypeError('"listener" argument must be a function');

      events = this._events;
      if (!events)
        return this;

      list = events[type];
      if (!list)
        return this;

      if (list === listener || list.listener === listener) {
        if (--this._eventsCount === 0)
          this._events = objectCreate(null);
        else {
          delete events[type];
          if (events.removeListener)
            this.emit('removeListener', type, list.listener || listener);
        }
      } else if (typeof list !== 'function') {
        position = -1;

        for (i = list.length - 1; i >= 0; i--) {
          if (list[i] === listener || list[i].listener === listener) {
            originalListener = list[i].listener;
            position = i;
            break;
          }
        }

        if (position < 0)
          return this;

        if (position === 0)
          list.shift();
        else
          spliceOne(list, position);

        if (list.length === 1)
          events[type] = list[0];

        if (events.removeListener)
          this.emit('removeListener', type, originalListener || listener);
      }

      return this;
    };

EventEmitter.prototype.removeAllListeners =
    function removeAllListeners(type) {
      var listeners, events, i;

      events = this._events;
      if (!events)
        return this;

      // not listening for removeListener, no need to emit
      if (!events.removeListener) {
        if (arguments.length === 0) {
          this._events = objectCreate(null);
          this._eventsCount = 0;
        } else if (events[type]) {
          if (--this._eventsCount === 0)
            this._events = objectCreate(null);
          else
            delete events[type];
        }
        return this;
      }

      // emit removeListener for all listeners on all events
      if (arguments.length === 0) {
        var keys = objectKeys(events);
        var key;
        for (i = 0; i < keys.length; ++i) {
          key = keys[i];
          if (key === 'removeListener') continue;
          this.removeAllListeners(key);
        }
        this.removeAllListeners('removeListener');
        this._events = objectCreate(null);
        this._eventsCount = 0;
        return this;
      }

      listeners = events[type];

      if (typeof listeners === 'function') {
        this.removeListener(type, listeners);
      } else if (listeners) {
        // LIFO order
        for (i = listeners.length - 1; i >= 0; i--) {
          this.removeListener(type, listeners[i]);
        }
      }

      return this;
    };

function _listeners(target, type, unwrap) {
  var events = target._events;

  if (!events)
    return [];

  var evlistener = events[type];
  if (!evlistener)
    return [];

  if (typeof evlistener === 'function')
    return unwrap ? [evlistener.listener || evlistener] : [evlistener];

  return unwrap ? unwrapListeners(evlistener) : arrayClone(evlistener, evlistener.length);
}

EventEmitter.prototype.listeners = function listeners(type) {
  return _listeners(this, type, true);
};

EventEmitter.prototype.rawListeners = function rawListeners(type) {
  return _listeners(this, type, false);
};

EventEmitter.listenerCount = function(emitter, type) {
  if (typeof emitter.listenerCount === 'function') {
    return emitter.listenerCount(type);
  } else {
    return listenerCount.call(emitter, type);
  }
};

EventEmitter.prototype.listenerCount = listenerCount;
function listenerCount(type) {
  var events = this._events;

  if (events) {
    var evlistener = events[type];

    if (typeof evlistener === 'function') {
      return 1;
    } else if (evlistener) {
      return evlistener.length;
    }
  }

  return 0;
}

EventEmitter.prototype.eventNames = function eventNames() {
  return this._eventsCount > 0 ? Reflect.ownKeys(this._events) : [];
};

// About 1.5x faster than the two-arg version of Array#splice().
function spliceOne(list, index) {
  for (var i = index, k = i + 1, n = list.length; k < n; i += 1, k += 1)
    list[i] = list[k];
  list.pop();
}

function arrayClone(arr, n) {
  var copy = new Array(n);
  for (var i = 0; i < n; ++i)
    copy[i] = arr[i];
  return copy;
}

function unwrapListeners(arr) {
  var ret = new Array(arr.length);
  for (var i = 0; i < ret.length; ++i) {
    ret[i] = arr[i].listener || arr[i];
  }
  return ret;
}

function objectCreatePolyfill(proto) {
  var F = function() {};
  F.prototype = proto;
  return new F;
}
function objectKeysPolyfill(obj) {
  var keys = [];
  for (var k in obj) if (Object.prototype.hasOwnProperty.call(obj, k)) {
    keys.push(k);
  }
  return k;
}
function functionBindPolyfill(context) {
  var fn = this;
  return function () {
    return fn.apply(context, arguments);
  };
}

return module.exports;
})();

// xrmp

const {EventEmitter} = events;

const MESSAGE_TYPES = (() => {
  let id = 0;
  return {
    PLAYER_MATRIX: id++,
    AUDIO: id++,
    OBJECT_MATRIX: id++,
    GEOMETRY: id++,
  };
})();
const _makeId = () => Math.floor(Math.random() * 0xFFFFFFFF);
const _elementGetter = (self, attribute) => self.listeners(attribute)[0];
const _elementSetter = (self, attribute, cb) => {
  if (typeof cb === 'function') {
    self.addEventListener(attribute, cb);
  } else {
    const listeners = self.listeners(attribute);
    for (let i = 0; i < listeners.length; i++) {
      self.removeEventListener(attribute, listeners[i]);
    }
  }
};

const numPlayerMatrixElements =
  (3+4) + // hmd
  (1 + (3+4)) * 2 + // gamepads
  (1 + (5*4*(3+3))) * 2; // hands
const _makePlayerMatrix = () => {
  const playerMatrix = new ArrayBuffer(Uint32Array.BYTES_PER_ELEMENT*2 + numPlayerMatrixElements*Float32Array.BYTES_PER_ELEMENT);
  playerMatrix.setArrayBuffer = (() => {
    const uint8Array = new Uint8Array(playerMatrix);
    return newArrayBuffer => {
      uint8Array.set(new Uint8Array(newArrayBuffer));
    };
  })();
  let playerMatrixIndex = 0;
  const _getPlayerMatrixIndex = n => {
    const oldPlayerMatrixIndex = playerMatrixIndex;
    playerMatrixIndex += n;
    return oldPlayerMatrixIndex;
  };
  playerMatrix.type = new Uint32Array(playerMatrix, _getPlayerMatrixIndex(Uint32Array.BYTES_PER_ELEMENT), 1);
  playerMatrix.type[0] = MESSAGE_TYPES.PLAYER_MATRIX;
  playerMatrix.id = new Uint32Array(playerMatrix, _getPlayerMatrixIndex(Uint32Array.BYTES_PER_ELEMENT), 1);
  playerMatrix.hmd = {
    position: new Float32Array(playerMatrix, _getPlayerMatrixIndex(3*Float32Array.BYTES_PER_ELEMENT), 3),
    quaternion: new Float32Array(playerMatrix, _getPlayerMatrixIndex(4*Float32Array.BYTES_PER_ELEMENT), 4),
  };
  const _makePlayerMatrixGamepad = () => {
    const enabled = new Uint32Array(playerMatrix, _getPlayerMatrixIndex(Uint32Array.BYTES_PER_ELEMENT), 1);
    const position = new Float32Array(playerMatrix, _getPlayerMatrixIndex(3*Float32Array.BYTES_PER_ELEMENT), 3);
    const quaternion = new Float32Array(playerMatrix, _getPlayerMatrixIndex(4*Float32Array.BYTES_PER_ELEMENT), 4);

    return {
      enabled,
      position,
      quaternion,
    };
  };
  playerMatrix.gamepads = [
    _makePlayerMatrixGamepad(),
    _makePlayerMatrixGamepad(),
  ];
  const _makePlayerMatrixHand = () => {
    const enabled = new Uint32Array(playerMatrix, _getPlayerMatrixIndex(Uint32Array.BYTES_PER_ELEMENT), 1);
    const data = new Float32Array(playerMatrix, _getPlayerMatrixIndex(5*4*(3+3)*Float32Array.BYTES_PER_ELEMENT), 5*4*(3+3));
    return {
      enabled,
      data,
    };
  };
  playerMatrix.hands = (() => {
    const hands = Array(2);
    for (let i = 0; i < hands.length; i++) {
      hands[i] = _makePlayerMatrixHand();
    }
    return hands;
  })();
  return playerMatrix;
};
const playerMatrix = _makePlayerMatrix();
const numObjectMatrixElements = 3+4;
const _makeObjectMatrix = () => {
  const objectMatrix = new ArrayBuffer(Uint32Array.BYTES_PER_ELEMENT*2 + numObjectMatrixElements*Float32Array.BYTES_PER_ELEMENT);
  objectMatrix.setArrayBuffer = (() => {
    const uint8Array = new Uint8Array(objectMatrix);
    return newArrayBuffer => {
      uint8Array.set(new Uint8Array(newArrayBuffer));
    };
  })();
  let objectMatrixIndex = 0;
  const _getObjectMatrixIndex = n => {
    const oldPbjectMatrixIndex = objectMatrixIndex;
    objectMatrixIndex += n;
    return oldPbjectMatrixIndex;
  };
  objectMatrix.type = new Uint32Array(objectMatrix, _getObjectMatrixIndex(Uint32Array.BYTES_PER_ELEMENT), 1);
  objectMatrix.type[0] = MESSAGE_TYPES.OBJECT_MATRIX;
  objectMatrix.id = new Uint32Array(objectMatrix, _getObjectMatrixIndex(Uint32Array.BYTES_PER_ELEMENT), 1);
  objectMatrix.position = new Float32Array(objectMatrix, _getObjectMatrixIndex(3*Float32Array.BYTES_PER_ELEMENT), 3);
  objectMatrix.quaternion = new Float32Array(objectMatrix, _getObjectMatrixIndex(4*Float32Array.BYTES_PER_ELEMENT), 4);
  return objectMatrix;
};
const objectMatrix = _makeObjectMatrix();

class XRLocalPlayer extends EventEmitter {
  constructor(id = _makeId(), state = {}, xrmp) {
    super();

    this.id = id;
    this.state = state;
    this.xrmp = xrmp;

    this.position = playerMatrix.position;
    this.quaternion = playerMatrix.quaternion;
    this.gamepads = playerMatrix.gamepads;
    this.hands = playerMatrix.hands;

    this.playerMatrix = _makePlayerMatrix();
    this.playerMatrix.id[0] = id;

    xrmp.ws.send(JSON.stringify({
      type: 'playerEnter',
      id,
      state,
    }));
  }
  pushUpdate() {
    this.xrmp.ws.send(this.playerMatrix);
  }
  setState(update) {
    for (const k in update) {
      this.state[k] = update[k];
    }

    this.xrmp.ws.send(JSON.stringify({
      type: 'playerSetState',
      id: this.id,
      state: update,
    }));
  }
  pushAudio(sampleRate, float32Array) {
    const audioMessage = new ArrayBuffer(Uint32Array.BYTES_PER_ELEMENT*3 + float32Array.byteLength);
    const uint32Array = new Uint32Array(audioMessage, 0, 3);
    uint32Array[0] = MESSAGE_TYPES.AUDIO;
    uint32Array[1] = this.id;
    uint32Array[2] = sampleRate;
    new Float32Array(audioMessage, Uint32Array.BYTES_PER_ELEMENT*3, float32Array.length).set(float32Array);
    this.xrmp.ws.send(audioMessage);
  }
}
module.exports.XRLocalPlayer = XRLocalPlayer;

class XRRemotePlayer extends EventEmitter {
  constructor(id, state, xrmp) {
    super();

    this.id = id;
    this.state = state;
    this.xrmp = xrmp;
  }
  pullUpdate(playerMatrix) {
    const e = new XRMultiplayerEvent('update');
    e.player = this;
    e.matrix = playerMatrix;
    this.emit(e.type, e);
  }
  pullAudioUpdate(sampleRate, float32Array) {
    const e = new XRMultiplayerEvent('audio');
    e.player = this;
    e.sampleRate = sampleRate;
    e.buffer = float32Array;
    this.emit(e.type, e);
  }
  addEventListener(name, fn) {
    return this.on(name, fn);
  }
  removeEventListener(name, fn) {
    return this.removeListener(name, fn);
  }
  removeAllEventListeners(name) {
    return this.removeAllListeners(name);
  }
  get onupdate() {
    return _elementGetter(this, 'update');
  }
  set onupdate(onupdate) {
    _elementSetter(this, 'update', onupdate);
  }
  get onstateupdate() {
    return _elementGetter(this, 'stateupdate');
  }
  set onstateupdate(onstateupdate) {
    _elementSetter(this, 'stateupdate', onstateupdate);
  }
  get onaudio() {
    return _elementGetter(this, 'audio');
  }
  set onaudio(onaudio) {
    _elementSetter(this, 'audio', onaudio);
  }
}
module.exports.XRRemotePlayer = XRRemotePlayer;

class XRObject extends EventEmitter {
  constructor(id = _makeId(), state = {}, xrmp) {
    super();

    this.id = id;
    this.state = state;
    this.xrmp = xrmp;

    this.objectMatrix = _makeObjectMatrix();
    this.objectMatrix.id[0] = id;
  }
  sendAdd() {
    this.xrmp.ws.send(JSON.stringify({
      type: 'objectAdd',
      id: this.id,
    }));
  }
  sendRemove() {
    this.xrmp.ws.send(JSON.stringify({
      type: 'objectRemove',
      id: this.id,
    }));
  }
  setState(update) {
    for (const k in update) {
      this.state[k] = update[k];
    }

    this.xrmp.ws.send(JSON.stringify({
      type: 'objectSetState',
      id: this.id,
      state: update,
    }));
  }
  setUpdateExpression(expression) {
    this.xrmp.ws.send(JSON.stringify({
      type: 'objectSetUpdateExpression',
      id: this.id,
      expression,
    }));
  }
  pullUpdate(objectMatrix) {
    const e = new XRMultiplayerEvent('update');
    e.object = this;
    e.matrix = objectMatrix;
    this.emit(e.type, e);
  }
  pushUpdate() {
    this.xrmp.ws.send(this.objectMatrix);
  }
  addEventListener(name, fn) {
    return this.on(name, fn);
  }
  removeEventListener(name, fn) {
    return this.removeListener(name, fn);
  }
  removeAllEventListeners(name) {
    return this.removeAllListeners(name);
  }
  get onupdate() {
    return _elementGetter(this, 'update');
  }
  set onupdate(onupdate) {
    _elementSetter(this, 'update', onupdate);
  }
}
module.exports.XRObject = XRObject;

class XRMultiplayerEvent {
  constructor(type) {
    this.type = type;
  }
}
module.exports.XRMultiplayerEvent = XRMultiplayerEvent;

class XRMultiplayer extends EventEmitter {
  constructor(url) {
    super();

    const id = Math.floor(Math.random() * 0xFFFFFFFF);
    this.id = id;

    this.open = false;

    this.localPlayers = [];
    this.remotePlayers = [];
    this.objects = [];
    this.state = {};

    const ws = new WebSocket(url + '?id=' + id);
    ws.binaryType = 'arraybuffer';
    ws.onopen = () => {
      const e = new XRMultiplayerEvent('open');
      this.emit(e.type, e);
    }
    ws.onclose = err => {
      this.open = false;

      const oldRemotePlayers = this.remotePlayers.slice();
      this.remotePlayers.length = 0;
      for (let i = 0; i < oldRemotePlayers.length; i++) {
        const remotePlayer = oldRemotePlayers[i];
        if (remotePlayer) {
          const e = new XRMultiplayerEvent('playerleave');
          e.player = remotePlayer;
          this.emit(e.type, e);
        }
      }

      const oldObjects = this.objects.slice();
      this.objects.length = 0;
      for (let i = 0; i < oldObjects.length; i++) {
        const object = oldObjects[i];
        if (object) {
          const e = new XRMultiplayerEvent('objectremove');
          e.object = object;
          this.emit(e.type, e);
        }
      }

      const e = new XRMultiplayerEvent('close');
      this.emit(e.type, e);
    };
    ws.onerror = error => {
      const e = new XRMultiplayerEvent('error');
      e.error = error;
      this.emit(e.type, e);
    };
    ws.onmessage = m => {
      const {data} = m;
      if (typeof data === 'string') {
        const j = JSON.parse(data);
        const {type} = j;

        switch (type) {
          case 'playerEnter': {
            const {id, state} = j;

            if (id !== undefined && state !== undefined) {
              const remotePlayer = new XRRemotePlayer(id, state, this);
              this.remotePlayers.push(remotePlayer);

              const e = new XRMultiplayerEvent('playerenter');
              e.player = remotePlayer;
              this.emit(e.type, e);
            } else {
              console.warn('got invalid playerEnter message', j);
            }
            break;
          }
          case 'playerLeave': {
            const {id} = j;

            if (id !== undefined) {
              const remotePlayerIndex = this.remotePlayers.findIndex(remotePlayer => remotePlayer.id === id);
              if (remotePlayerIndex !== -1) {
                const remotePlayer = this.remotePlayers[remotePlayerIndex];
                this.remotePlayers.splice(remotePlayerIndex, 1);

                const e = new XRMultiplayerEvent('playerleave');
                e.player = remotePlayer;
                this.emit(e.type, e);
              } else {
                console.warn('got event for unknown remote player', {id});
              }
            } else {
              console.warn('got invalid playerLeave message', j);
            }
            break;
          }
          case 'playerSetState': {
            const {id, state: update} = j;

            if (id !== undefined && update !== undefined) {
              const remotePlayer = this.remotePlayers.find(player => player.id === id);
              if (remotePlayer) {
                for (const k in update) {
                  remotePlayer.state[k] = update[k];
                }

                const e = new XRMultiplayerEvent('stateupdate');
                e.state = remotePlayer.state;
                e.update = update;
                remotePlayer.emit(e.type, e);
              } else {
                console.warn('got event for unknown remote player', {id});
              }
            } else {
              console.warn('got invalid playerSetState message', j);
            }
            break;
          }
          case 'objectAdd': {
            const {id, state} = j;

            if (id !== undefined && state !== undefined) {
              const object = new XRObject(id, state, this);
              this.objects.push(object);

              const e = new XRMultiplayerEvent('objectadd');
              e.object = object;
              this.emit(e.type, e);
            } else {
              console.warn('got invalid objectAdd message', j);
            }
            break;
          }
          case 'objectRemove': {
            const {id} = j;

            if (id !== undefined) {
              const index = this.objects.findIndex(object => object.id === id);
              if (index !== -1) {
                const object = this.objects[index];
                this.objects.splice(index, 1);

                const e = new XRMultiplayerEvent('objectremove');
                e.object = object;
                this.emit(e.type, e);
              } else {
                console.warn('got event for unknown object', {id});
              }
            } else {
              console.warn('got invalid objectRemove message', j);
            }
            break;
          }
          case 'objectSetState': {
            const {id, state: update} = j;

            if (id !== undefined && update !== undefined) {
              const object = this.objects.find(object => object.id === id);
              if (object) {
                for (const k in update) {
                  object.state[k] = update[k];
                }

                const e = new XRMultiplayerEvent('stateupdate');
                e.state = object.state;
                e.update = update;
                object.emit(e.type, e);
              } else {
                console.warn('got event for unknown object', {id});
              }
            } else {
              console.warn('got invalid objectSetState message', j);
            }
            break;
          }
          case 'setState': {
            const {state: update} = j;

            if (update !== undefined) {
              for (const k in update) {
                this.state[k] = update[k];
              }

              const e = new XRMultiplayerEvent('stateupdate');
              e.state = this.state;
              e.update = update;
              this.emit(e.type, e);
            } else {
              console.warn('got invalid setState message', j);
            }
            break;
          }
          case 'sync': {
            this.open = true;

            const e = new XRMultiplayerEvent('sync');
            this.emit(e.type, e);
            break;
          }
          default: {
            console.warn('got invalid json message type', type);
            break;
          }
        }
      } else {
        const type = new Uint32Array(data, 0, 1)[0];
        if (type === MESSAGE_TYPES.PLAYER_MATRIX) {
          const id = new Uint32Array(data, Uint32Array.BYTES_PER_ELEMENT, 1)[0];
          const player = this.remotePlayers.find(player => player.id === id);

          if (player) {
            playerMatrix.setArrayBuffer(data);

            player.pullUpdate(playerMatrix);
          } else {
            console.warn('got unknown player update message', {id});
          }
        } else if (type === MESSAGE_TYPES.AUDIO) {
          const uint32Array = new Uint32Array(data, 0, 3);
          const id = uint32Array[1];
          const player = this.remotePlayers.find(player => player.id === id);

          if (player) {
            const sampleRate = uint32Array[2];
            const float32Array = new Float32Array(data, Uint32Array.BYTES_PER_ELEMENT*3, (data.byteLength - Uint32Array.BYTES_PER_ELEMENT*3) / Float32Array.BYTES_PER_ELEMENT);
            player.pullAudioUpdate(sampleRate, float32Array);
          } else {
            console.warn('got unknown player update message', {id});
          }
        } else if (type === MESSAGE_TYPES.OBJECT_MATRIX) {
          const id = new Uint32Array(data, Uint32Array.BYTES_PER_ELEMENT, 1)[0];
          const object = this.objects.find(object => object.id === id);

          if (object) {
            objectMatrix.setArrayBuffer(data);
            object.pullUpdate(objectMatrix);
          } else {
            console.warn('got unknown object update message', {id});
          }
        } else if (type === MESSAGE_TYPES.GEOMETRY) {
          const header = new Uint32Array(data, Uint32Array.BYTES_PER_ELEMENT, 3);
          const numPositions = header[0];
          const numNormals = header[1];
          const numIndices = header[2];

          const positions = new Float32Array(data, Uint32Array.BYTES_PER_ELEMENT + 3*Uint32Array.BYTES_PER_ELEMENT, numPositions);
          const normals = new Float32Array(data, Uint32Array.BYTES_PER_ELEMENT + 3*Uint32Array.BYTES_PER_ELEMENT + positions.byteLength, numNormals);
          const indices = new Uint32Array(data, Uint32Array.BYTES_PER_ELEMENT + 3*Uint32Array.BYTES_PER_ELEMENT + positions.byteLength + normals.byteLength, numIndices);

          const e = new XRMultiplayerEvent('geometry');
          e.positions = positions;
          e.normals = normals;
          e.indices = indices;
          this.emit(e.type, e);
        } else {
          console.warn('unknown binary message type', {type});
        }
      }
    };
    this.ws = ws;
  }
  close() {
    this.ws.close();
  }
  isOpen() {
    // return this.ws.readyState === WebSocket.OPEN
    return this.open;
  }
  addPlayer(id, state) {
    const localPlayer = new XRLocalPlayer(id, state, this);
    this.localPlayers.push(localPlayer);
    return localPlayer;
  }
  removePlayer(localPlayer) {
    const {id} = localPlayer;
    const index = this.localPlayers.findIndex(localPlayer => localPlayer.id === id);
    if (index !== -1) {
      this.localPlayers.splice(index, 1);
    } else {
      throw new Error('player not added');
    }
  }
  addObject(id, state) {
    const object = new XRObject(id, state, this);
    object.sendAdd();
    this.objects.push(object);
    return object;
  }
  removeObject(object) {
    const {id} = object;
    const index = this.objects.findIndex(object => object.id === id);
    if (index !== -1) {
      const object = this.objects[index];
      object.sendRemove();

      this.objects.splice(index, 1);
    } else {
      throw new Error('object not removed');
    }
  }
  setState(update) {
    this.ws.send(JSON.stringify({
      type: 'setState',
      state: update,
    }));

    for (const k in update) {
      this.state[k] = update[k];
    }
  }
  pushGeometry(positions, normals, indices) {
    const geometryBuffer = new ArrayBuffer(Uint32Array.BYTES_PER_ELEMENT + 3*Uint32Array.BYTES_PER_ELEMENT + positions.byteLength + normals.byteLength + indices.byteLength);

    new Uint32Array(geometryBuffer, 0, 1)[0] = MESSAGE_TYPES.GEOMETRY;

    const header = new Uint32Array(geometryBuffer, Uint32Array.BYTES_PER_ELEMENT, 3);
    header[0] = positions.length;
    header[1] = normals.length;
    header[2] = indices.length;

    new Float32Array(geometryBuffer, Uint32Array.BYTES_PER_ELEMENT + 3*Uint32Array.BYTES_PER_ELEMENT, positions.length).set(positions);
    new Float32Array(geometryBuffer, Uint32Array.BYTES_PER_ELEMENT + 3*Uint32Array.BYTES_PER_ELEMENT + positions.byteLength, normals.length).set(normals);
    new Uint32Array(geometryBuffer, Uint32Array.BYTES_PER_ELEMENT + 3*Uint32Array.BYTES_PER_ELEMENT + positions.byteLength + normals.byteLength, indices.length).set(indices);

    this.ws.send(geometryBuffer);
  }
  addEventListener(name, fn) {
    return this.on(name, fn);
  }
  removeEventListener(name, fn) {
    return this.removeListener(name, fn);
  }
  removeAllEventListeners(name) {
    return this.removeAllListeners(name);
  }
  get onopen() {
    return _elementGetter(this, 'open');
  }
  set onopen(onopen) {
    _elementSetter(this, 'open', onopen);
  }
  get onclose() {
    return _elementGetter(this, 'close');
  }
  set onclose(onclose) {
    _elementSetter(this, 'close', onclose);
  }
  get onerror() {
    return _elementGetter(this, 'error');
  }
  set onerror(onerror) {
    _elementSetter(this, 'error', onerror);
  }
  get onplayerenter() {
    return _elementGetter(this, 'playerenter');
  }
  set onplayerenter(onplayerenter) {
    _elementSetter(this, 'playerenter', onplayerenter);
  }
  get onplayerleave() {
    return _elementGetter(this, 'playerleave');
  }
  set onplayerleave(onplayerleave) {
    _elementSetter(this, 'playerleave', onplayerleave);
  }
  get onobjectadd() {
    return _elementGetter(this, 'objectadd');
  }
  set onobjectadd(onobjectadd) {
    _elementSetter(this, 'objectadd', onobjectadd);
  }
  get onobjectremove() {
    return _elementGetter(this, 'objectremove');
  }
  set onobjectremove(onobjectremove) {
    _elementSetter(this, 'objectremove', onobjectremove);
  }
  get onstateupdate() {
    return _elementGetter(this, 'stateupdate');
  }
  set onstateupdate(onstateupdate) {
    _elementSetter(this, 'stateupdate', onstateupdate);
  }
  get ongeometry() {
    return _elementGetter(this, 'geometry');
  }
  set ongeometry(ongeometry) {
    _elementSetter(this, 'geometry', ongeometry);
  }
  get onsync() {
    return _elementGetter(this, 'sync');
  }
  set onsync(onsync) {
    _elementSetter(this, 'sync', onsync);
  }
}
module.exports.XRMultiplayer = XRMultiplayer;

return module.exports;

})(typeof module !== 'undefined' ? module : undefined);

if (typeof window !== 'undefined') {
  window.XRLocalPlayer = xrmp.XRLocalPlayer;
  window.XRRemotePlayer = xrmp.XRRemotePlayer;
  window.XRObject = xrmp.XRObject;
  window.XRMultiplayer = xrmp.XRMultiplayer;
}

})();
