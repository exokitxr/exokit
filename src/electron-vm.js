const path = require('path');
const {EventEmitter} = require('events');
const stream = require('stream');
const net = require('net');
const child_process = require('child_process');
const os = require('os');
const {TextEncoder} = require('util');

const bindings = require('./native-bindings');
const electron = !bindings.nativePlatform ? require('electron') : null;
const keycode = require('keycode');

const GlobalContext = require('./GlobalContext');

const {process} = global;

const TYPES = (() => {
  let iota = 0;
  return {
    CONSOLE: ++iota,
    LOAD: ++iota,
    EVENT: ++iota,
    MESSAGE: ++iota,
    IMAGEDATA: ++iota,
  };
})();
const PIPE_PREFIX = (() => {
  switch (os.platform()) {
    case 'win32': return '\\\\.\\pipe\\exokit-electron';
    default: return '/tmp/exokit-electron';
  }
})();

const _arrayifyModifiers = modifiers => {
  const result = [];
  if (modifiers && modifiers.shiftKey) {
    result.push('shift');
  }
  if (modifiers && modifiers.ctrlKey) {
    result.push('control');
  }
  if (modifiers && modifiers.altKey) {
    result.push('alt');
  }
  return result;
};
const _stringifyButton = button => {
  switch (button) {
    case 0: return 'left';
    case 1: return 'middle';
    case 2: return 'right';
    default: return '';
  }
};
class ElectronVm extends EventEmitter {
  constructor({url = 'http://google.com', width = 1280, height = 1024, devicePixelRatio = 1, inline = true, transparent = false, context = null} = {}) {
    super();
    
    const server = net.createServer(stream => {
      cp.stdin
        .pipe(stream)
        .pipe(cp.stdout);
    });
    const id = Atomics.add(GlobalContext.xrState.id, 0, 1) + 1;
    const pipePath = `${PIPE_PREFIX}-${process.pid}-${id}`;
    server.listen(pipePath);
    
    const cp = child_process.spawn(electron, [path.join(__dirname, 'electron-child.js'), pipePath]);
    cp.stdin = new stream.PassThrough();
    cp.stdout = new stream.PassThrough();
    this.childProcess = cp;
    
    const texture = context.createTexture();
    this.texture = texture;
    let textureWidth = 0;
    let textureHeight = 0;

    this._width = width;
    this._height = height;

    const bs = [];
    let bsLength = 0;
    const _pull  = l => {
      if (bsLength >= l) {
        const result = Buffer.allocUnsafe(l);
        let localLength = 0;

        while (localLength < l) {
          const need = l - localLength;
          let b = bs.shift();
          if (b.length >= need) {
            b.copy(result, localLength, 0, need);
            b = b.slice(need);
            if (b.length > 0) {
              bs.unshift(b);
            }
            localLength += need;
            bsLength -= need;
          } else {
            b.copy(result, localLength, 0, b.length);
            localLength += b.length;
            bsLength -= b.length;
          }
        }

        return result;
      } else {
        return null;
      }
    };
    cp.stdout.on('data', b => {
      // console.log('got data', b.slice(0, 40).toString('hex'), JSON.stringify(b.slice(0, 40).toString()));

      bs.push(b);
      bsLength += b.byteLength;

      while (bsLength >= Uint32Array.BYTES_PER_ELEMENT) {
        let [b] = bs;
        if ((b.byteOffset % Uint32Array.BYTES_PER_ELEMENT) !== 0) {
          const oldB = b;
          b = bs[0] = new Buffer(new ArrayBuffer(oldB.byteLength), 0, oldB.byteLength);
          oldB.copy(b, 0, 0, oldB.byteLength);
        }
        const [type] = new Uint32Array(b.buffer, b.byteOffset, 1);
        // console.log('got type', type);
        switch (type) {
          case TYPES.CONSOLE: {
            if (b.length < (1+1)*Uint32Array.BYTES_PER_ELEMENT) {
              console.warn(new Error('did not get full header from electron pipe').stack);
            }
            const header = new Uint32Array(b.buffer, b.byteOffset + Uint32Array.BYTES_PER_ELEMENT, 1);
            const [size] = header;
            b = _pull((1+1)*Uint32Array.BYTES_PER_ELEMENT + size);
            if (b) {
              b = Buffer.from(b.buffer, b.byteOffset + (1+1)*Uint32Array.BYTES_PER_ELEMENT, size);
              const s = new TextDecoder().decode(b);
              process.stdout.write(s);
            } else {
              // console.log('failed to pull', (1+1)*Uint32Array.BYTES_PER_ELEMENT + size, bsLength);
              return;
            }
            break;
          }
          case TYPES.LOAD: {
            if (b.length < (1+1)*Uint32Array.BYTES_PER_ELEMENT) {
              console.warn(new Error('did not get full header from electron pipe').stack);
            }
            const header = new Uint32Array(b.buffer, b.byteOffset + Uint32Array.BYTES_PER_ELEMENT, 1);
            const [status] = header;
            _pull((1+1)*Uint32Array.BYTES_PER_ELEMENT);
            if (status >= 200 && status < 300) {
              this.emit('load');
            } else {
              this.emit('error', new Error(`load got invalid status code ${status}`));
            }
            break;
          }
          case TYPES.EVENT: {
            if (b.length < (1+1)*Uint32Array.BYTES_PER_ELEMENT) {
              console.warn(new Error('did not get full header from electron pipe').stack);
            }
            const header = new Uint32Array(b.buffer, b.byteOffset + Uint32Array.BYTES_PER_ELEMENT, 1);
            const [size] = header;
            b = _pull((1+1)*Uint32Array.BYTES_PER_ELEMENT + size);
            if (b) {
              b = Buffer.from(b.buffer, b.byteOffset + (1+1)*Uint32Array.BYTES_PER_ELEMENT, size);
              const s = new TextDecoder().decode(b);
              const e = JSON.parse(s);
              const {type, event} = e;
              this.emit(type, event);
            } else {
              // console.log('failed to pull', (1+1)*Uint32Array.BYTES_PER_ELEMENT + size, bsLength);
              return;
            }
            break;
          }
          case TYPES.MESSAGE: {
            if (b.length < (1+1)*Uint32Array.BYTES_PER_ELEMENT) {
              console.warn(new Error('did not get full header from electron pipe').stack);
            }
            const header = new Uint32Array(b.buffer, b.byteOffset + Uint32Array.BYTES_PER_ELEMENT, 1);
            const [size] = header;
            b = _pull((1+1)*Uint32Array.BYTES_PER_ELEMENT + size);
            if (b) {
              b = Buffer.from(b.buffer, b.byteOffset + (1+1)*Uint32Array.BYTES_PER_ELEMENT, size);
              const s = new TextDecoder().decode(b);
              const m = JSON.parse(s);
              this.emit('message', m);
            } else {
              // console.log('failed to pull', (1+1)*Uint32Array.BYTES_PER_ELEMENT + size, bsLength);
              return;
            }
            break;
          }
          case TYPES.IMAGEDATA: {
            if (b.length < (1+6)*Uint32Array.BYTES_PER_ELEMENT) {
              console.warn(new Error('did not get full header from electron pipe').stack);
            }
            const header = new Uint32Array(b.buffer, b.byteOffset + Uint32Array.BYTES_PER_ELEMENT, 6);
            const [x, y, width, height, newTextureWidth, newTextureHeight] = header;
            // console.log('got header', x, y, width, height);
            b = _pull((1+6)*Uint32Array.BYTES_PER_ELEMENT + width*height*4);
            if (b) {
              const imageData = new Uint8Array(b.buffer, b.byteOffset + (1+6)*Uint32Array.BYTES_PER_ELEMENT, width*height*4);
              // console.log('got frame', new Buffer(b.buffer, b.byteOffset + (1+6)*Uint32Array.BYTES_PER_ELEMENT, width*height*4).slice(width*height*4/2, width*height*4/2+20).toString('hex'));
              context.loadSubTexture(texture.id, x, y, width, height, imageData, textureWidth, textureHeight, newTextureWidth, newTextureHeight);
              textureWidth = newTextureWidth;
              textureHeight = newTextureHeight;
            } else {
              return;
            }
            break;
          }
          default: {
            console.warn('electron parent got invalid type', type);
            return;
          }
        }
      }
    });
    cp.stdout.on('end', () => {
      console.log('stdout end');
    });
    cp.stderr.on('data', b => {
      process.stdout.write(b);
    });
    cp.stderr.on('end', () => {
      console.log('stderr end');
    });
    cp.on('exit', () => {
      console.log('child process exit');
    });

    this.runAsync({
      method: 'initialize',
      url,
      width,
      height,
      devicePixelRatio,
      inline,
      transparent,
    });
  }

  get width() {
    return this._width;
  }
  set width(width) {
    this._width = width;

    this.runAsync({
      method: 'resize',
      width: this._width,
      height: this._height,
    });
  }
  get height() {
    return this._height;
  }
  set height(height) {
    this._height = height;

    this.runAsync({
      method: 'resize',
      width: this._width,
      height: this._height,
    });
  }

  runAsync(e) {
    const s = JSON.stringify(e);
    const uint8Array = new TextEncoder().encode(s);
    const buffer = new Buffer(uint8Array.buffer, uint8Array.byteOffset, uint8Array.byteLength);
    {
      const b = Uint32Array.from([buffer.length]);
      const b2 = new Buffer(b.buffer, b.byteOffset, b.byteLength);
      this.childProcess.stdin.write(b2);
    }
    this.childProcess.stdin.write(buffer);
  }
  
  postMessage(message) {
    this.runAsync({
      method: 'postMessage',
      message,
    });
  }

  runJs(jsString, scriptUrl, startLine) {
    this.runAsync({
      method: 'runJs',
      jsString,
      scriptUrl,
      startLine,
    });
  }

  setPosition(x, y) {
    this.runAsync({
      method: 'setPosition',
      x,
      y,
    });
  }
  show() {
    this.runAsync({
      method: 'show',
    });
  }
  hide() {
    this.runAsync({
      method: 'hide',
    });
  }
  setAlwaysOnTop(value) {
    this.runAsync({
      method: 'setAlwaysOnTop',
      value,
    });
  }

  sendInputEvent(event) {
    this.runAsync({
      method: 'sendInputEvent',
      event,
    });
  }

  sendMouseMove(x, y) {
    this.sendInputEvent({
      type: 'mouseMove',
      x,
      y,
    });
  }
  sendMouseDown(x, y, button) {
    this.sendInputEvent({
      type: 'mouseDown',
      x,
      y,
      button: _stringifyButton(button),
      clickCount: 1,
    });
  }
  sendMouseUp(x, y, button) {
    this.sendInputEvent({
      type: 'mouseUp',
      x,
      y,
      button: _stringifyButton(button),
      clickCount: 1,
    });
  }
  sendMouseWheel(x, y, deltaX, deltaY) {
    this.sendInputEvent({
      type: 'mouseWheel',
      x,
      y,
      deltaX,
      deltaY,
    });
  }
  sendKeyDown(keyCode, modifiers) {
    this.sendInputEvent({
      type: 'keyDown',
      keyCode: keycode(keyCode),
      modifiers: _arrayifyModifiers(modifiers),
    });
  }
  sendKeyUp(keyCode, modifiers) {
    this.sendInputEvent({
      type: 'keyUp',
      keyCode: keycode(keyCode),
      modifiers: _arrayifyModifiers(modifiers),
    });
  }
  sendKeyPress(keyCode, modifiers) {
    keyCode = String.fromCharCode(keyCode);
    if (modifiers && modifiers.shiftKey) {
      if (/^[a-z]$/.test(keyCode)) {
        keyCode = keyCode.toUpperCase();
      } else if (keyCode === '1') {
        keyCode = '!';
      } else if (keyCode === '2') {
        keyCode = '@';
      } else if (keyCode === '3') {
        keyCode = '#';
      } else if (keyCode === '4') {
        keyCode = '$';
      } else if (keyCode === '5') {
        keyCode = '%';
      } else if (keyCode === '6') {
        keyCode = '^';
      } else if (keyCode === '7') {
        keyCode = '&';
      } else if (keyCode === '8') {
        keyCode = '*';
      } else if (keyCode === '9') {
        keyCode = '(';
      } else if (keyCode === '0') {
        keyCode = ')';
      } else if (keyCode === '-') {
        keyCode = '_';
      } else if (keyCode === '=') {
        keyCode = '+';
      } else if (keyCode === '`') {
        keyCode = '~';
      } else if (keyCode === '[') {
        keyCode = '{';
      } else if (keyCode === ']') {
        keyCode = '}';
      } else if (keyCode === '\\') {
        keyCode = '|';
      } else if (keyCode === ';') {
        keyCode = ':';
      } else if (keyCode === '\'') {
        keyCode = '"';
      } else if (keyCode === ',') {
        keyCode = '<';
      } else if (keyCode === '.') {
        keyCode = '>';
      } else if (keyCode === '/') {
        keyCode = '?';
      }
    }
    this.sendInputEvent({
      type: 'char',
      keyCode,
      modifiers: _arrayifyModifiers(modifiers),
    });
  }

  destroy() {
    this.childProcess.kill(9);
  }
}
module.exports.ElectronVm = ElectronVm;

/* const browser = new ElectronVm({
  url: 'https://google.com',
  width: 1920,
  height: 1080,
});
browser.postMessage({
  lol: 'zol',
}); */

/* {
  process.on('SIGINT', () => {
    // cp.kill(9);
    console.log('SIGINT...');
    process.exit();
  });
  process.on('SIGTERM', () => {
    // cp.kill(9);
    console.log('SIGTERM...');
    process.exit();
  });
  process.on('exit', () => {
    console.log('exiting...');
    cp.kill(9);
  });
} */
