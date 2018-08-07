const {BrowserWindow} = require('electron');
const ipc = require('node-ipc');

ipc.config.id = process.argv[2];
// ipc.config.retry=1500;
ipc.config.rawBuffer=true;
// ipc.config.encoding='ascii';
ipc.config.silent=true;

const _flipImage = (width, height, stride, buffer) => {
  const buffer2 = new Buffer(buffer.length);
  for (let y = 0; y < height; y++) {
    const yBottom = height - y - 1;
    buffer2.set(buffer.slice(yBottom * width * stride, (yBottom + 1) * width * stride), y * width * stride);
  }
  /* for (let i = 0; i < width * height * 4; i++) {
    buffer2[i] *= buffer2 * 2;
  } */
  return buffer2;
};

ipc.serve(
    function(){
        let browserWindow = null;

        ipc.server.on(
            'data',
            function(data, socket){
              data = JSON.parse(data.toString('utf8'));
              const {method, id, args} = data;
              switch (method) {
                case 'createBrowserWindow': {
                  console.log('create window', args);
                  browserWindow = new BrowserWindow(args);

                  const b = Buffer.from(JSON.stringify({
                    method: 'response',
                    id,
                  }), 'utf8');
                  const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
                  typeB[0] = 0;
                  let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                  lengthB.writeUInt32LE(b.length, 0);
                  lengthB = Buffer.from(lengthB.toString('hex'), 'ascii');
                  ipc.server.emit(socket, typeB);
                  ipc.server.emit(socket, lengthB);
                  ipc.server.emit(socket, b);

                  browserWindow.webContents.on('paint', (event, dirty, image) => {
                    // message 1
                    let b = image.getBitmap();
                    // let b = image.toBitmap();
                    const {width, height} = dirty;
                    b = _flipImage(width, height, 4, b);
                    b = Buffer.from(b.toString('hex'), 'ascii');

                    let typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
                    typeB[0] = 1;

                    let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                    lengthB.writeUInt32LE(b.length, 0);
                    lengthB = Buffer.from(lengthB.toString('hex'), 'ascii');

                    ipc.server.emit(socket, typeB);
                    ipc.server.emit(socket, lengthB);
                    ipc.server.emit(socket, b);

                    // message 2
                    b = Buffer.from(JSON.stringify({
                      method: 'paint',
                      args: dirty,
                    }), 'utf8');

                    typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
                    typeB[0] = 0;

                    lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                    lengthB.writeUInt32LE(b.length, 0);
                    lengthB = Buffer.from(lengthB.toString('hex'), 'ascii');

                    ipc.server.emit(socket, typeB);
                    ipc.server.emit(socket, lengthB);
                    ipc.server.emit(socket, b);
                  });
                  ['did-start-loading', 'did-stop-loading', 'did-fail-load', 'did-navigate', 'dom-ready'].forEach(e => {
                    browserWindow.webContents.on(e, () => {
                      const b = Buffer.from(JSON.stringify({
                        method: e,
                      }), 'utf8');
                      const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
                      typeB[0] = 0;
                      let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                      lengthB.writeUInt32LE(b.length, 0);
                      lengthB = Buffer.from(lengthB.toString('hex'), 'ascii');
                      ipc.server.emit(socket, typeB);
                      ipc.server.emit(socket, lengthB);
                      ipc.server.emit(socket, b);
                    });
                  });
                  break;
                }
                case 'loadURL': {
                  browserWindow.webContents.loadURL(args);
                  const b = Buffer.from(JSON.stringify({
                    method: 'response',
                    id,
                  }), 'utf8');
                  const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
                  typeB[0] = 0;
                  let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                  lengthB.writeUInt32LE(b.length, 0);
                  lengthB = Buffer.from(lengthB.toString('hex'), 'ascii');
                  ipc.server.emit(socket, typeB);
                  ipc.server.emit(socket, lengthB);
                  ipc.server.emit(socket, b);
                  break;
                }
                case 'setFrameRate': {
                  browserWindow.webContents.setFrameRate(args);
                  const b = Buffer.from(JSON.stringify({
                    method: 'response',
                    id,
                  }), 'utf8');
                  const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
                  typeB[0] = 0;
                  let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                  lengthB.writeUInt32LE(b.length, 0);
                  lengthB = Buffer.from(lengthB.toString('hex'), 'ascii');
                  ipc.server.emit(socket, typeB);
                  ipc.server.emit(socket, lengthB);
                  ipc.server.emit(socket, b);
                  break;
                }
                case 'insertCSS': {
                  browserWindow.webContents.insertCSS(args);
                  const b = Buffer.from(JSON.stringify({
                    method: 'response',
                    id,
                  }), 'utf8');
                  const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
                  typeB[0] = 0;
                  let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                  lengthB.writeUInt32LE(b.length, 0);
                  lengthB = Buffer.from(lengthB.toString('hex'), 'ascii');
                  ipc.server.emit(socket, typeB);
                  ipc.server.emit(socket, lengthB);
                  ipc.server.emit(socket, b);
                  break;
                }
                case 'sendInputEvent': {
                  browserWindow.webContents.sendInputEvent(args);
                  const b = Buffer.from(JSON.stringify({
                    method: 'response',
                    id,
                  }), 'utf8');
                  const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
                  typeB[0] = 0;
                  let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                  lengthB.writeUInt32LE(b.length, 0);
                  lengthB = Buffer.from(lengthB.toString('hex'), 'ascii');
                  ipc.server.emit(socket, typeB);
                  ipc.server.emit(socket, lengthB);
                  ipc.server.emit(socket, b);
                  break;
                }
              }
            }
        );
    }
);
ipc.server.start();

// process.exit();