const path = require('path');
const url = require('url');
const stream = require('stream');
const net = require('net');
const {Console} = require('console');
const electron = require('electron')
const app = electron.app;
const {BrowserWindow} = electron;

app.disableHardwareAcceleration(); // this makes for potentially faster transfer of pixels
app.commandLine.appendSwitch('force-device-scale-factor', '1');

let {console} = global;

const PIPE_PATH = process.argv[2];
const TYPES = (() => {
  let iota = 0;
  return {
    CONSOLE: ++iota,
    LOAD: ++iota,
    MESSAGE: ++iota,
    IMAGEDATA: ++iota,
  };
})();

let mainWindow = null;

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
const _consumeInput = () => {
  while (bsLength >= Uint32Array.BYTES_PER_ELEMENT) {
    // console.log('tick');
    let [b] = bs;
    if ((b.byteOffset % Uint32Array.BYTES_PER_ELEMENT) !== 0) {
      const oldB = b;
      b = bs[0] = new Buffer(new ArrayBuffer(oldB.byteLength), 0, oldB.byteLength);
      oldB.copy(b, 0, 0, oldB.byteLength);
    }
    const header = new Uint32Array(b.buffer, b.byteOffset, 1);
    const [size] = header;
    // console.warn('got header', x, y, width, height);
    b = _pull(Uint32Array.BYTES_PER_ELEMENT + size);
    if (b) {
      b = Buffer.from(b.buffer, b.byteOffset + Uint32Array.BYTES_PER_ELEMENT, size);
      const s = new TextDecoder().decode(b);
      const e = JSON.parse(s);
      // console.warn('child got message', e);
      
      switch (e.method) {
        case 'initialize': {
          const {url, width, height, devicePixelRatio} = e;

          mainWindow = new BrowserWindow({
            width,
            height,
            show: false,
            webPreferences: {
              offscreen: true,
              webSecurity: false,
              allowRunningInsecureContent: true,
              backgroundThrottling: false,
              preload: path.join(__dirname, 'electron-preload.js'),
              zoomFactor: devicePixelRatio,
            },
          })
          mainWindow.loadURL(url)
            .then(() => {
              const b = Uint32Array.from([TYPES.LOAD, 200]);
              const b2 = new Buffer(b.buffer, b.byteOffset, b.byteLength);
              parentPort.write(b2);

              loaded = true;
              _flushParentPort();
            });
          mainWindow.focusOnWebView();
          mainWindow.webContents.on('console-message', (event, level, message, line, sourceId) => {
            const match = message.match(/^<postMessage>(.+)$/);
            if (match) {
              const s = match[1];
              let m = new TextEncoder().encode(s);
              m = new Buffer(m.buffer, m.byteOffset, m.byteLength);
              const b = Uint32Array.from([TYPES.MESSAGE, m.length]);
              const b2 = new Buffer(b.buffer, b.byteOffset, b.byteLength);
              _parentPortWrite(b2);
              _parentPortWrite(m);
            } else {
              console.log(message);
            }
          });
          mainWindow.webContents.on('paint', (event, dirty, image) => {
            // console.warn('child got paint', dirty);
            const {width, height} = image.getSize();
            {
              const b = Uint32Array.from([TYPES.IMAGEDATA, dirty.x, dirty.y, dirty.width, dirty.height, width, height]);
              const b2 = new Buffer(b.buffer, b.byteOffset, b.byteLength);
              _parentPortWrite(b2);
              // process.stdout.write(b2);
            }
            {
              const bitmap = image.getBitmap();
              const clippedBitmap = Buffer.allocUnsafe(dirty.width * dirty.height * 4);
              // clippedBitmap.fill(0xFF);
              for (let y = 0; y < dirty.height; y++) {
                const srcY = dirty.y + y;
                const dstY = y;
                bitmap.copy(clippedBitmap, (dstY * dirty.width)*4, ((srcY * width) + dirty.x)*4, ((srcY * width) + dirty.x + dirty.width)*4);
              }
              _parentPortWrite(clippedBitmap);
              // process.stdout.write(i2);
              // console.warn('electron child got dirty', dirty, i2.byteLength);
            }
          });
          mainWindow.webContents.setFrameRate(30);

          mainWindow.on('closed', function () {
            console.warn('electron child  window closed');
          });
          
          break;
        }
        case 'resize': {
          const {width, height} = e;
          mainWindow.setSize(width, height);
          break;
        }
        case 'runJs': {
          mainWindow.webContents.executeJavaScript(e.jsString);
          break;
        }
        case 'postMessage': {
          break;
        }
        case 'sendInputEvent': {
          if (mainWindow) {
            mainWindow.webContents.sendInputEvent(e.event);
          }
          break;
        }
        default: {
          console.warn('electron child got invalid method', e.method);
          break;
        }
      }
    } else {
      break;
    }
  }
};

let ready = false;
app.once('ready', () => {
  ready = true;
  
  _consumeInput();
});

app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') {
    app.quit()
  }
});

/* app.on('activate', function () {
  if (mainWindow === null) {
    createWindow()
  }
}); */

let loaded = false;
const queue = [];
const _parentPortWrite = m => {
  if (loaded) {
    parentPort.write(m);
  } else {
    queue.push(m);
  }
};
const _flushParentPort = () => {
  for (let i = 0; i < queue.length; i++) {
    _parentPortWrite(queue[i]);
  }
  queue.length = 0;
};
const parentPort = net.connect(PIPE_PATH, err => {
  const consoleStream = new stream.Writable();
  const _logChunk = chunk => {
    const b = Uint32Array.from([TYPES.CONSOLE, chunk.length]);
    const b2 = new Buffer(b.buffer, b.byteOffset, b.byteLength);
    parentPort.write(b2);
    parentPort.write(chunk);
  };
  consoleStream._write = (chunk, encoding, callback) => {
    _logChunk(chunk);

    callback();
  };
  consoleStream._writev = (chunks, callback) => {
    for (let i = 0; i < chunks.length; i++) {
      _logChunk(chunks[i]);
    }
    callback();
  };
  console = new Console(consoleStream);

  parentPort.on('data', b => {
    bs.push(b);
    bsLength += b.byteLength;

    if (ready) {
      _consumeInput();
    }
  });
});