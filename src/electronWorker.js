const events = require('events');
const {EventEmitter} = events;
const {BrowserWindow} = require('electron');
const ipc = require('node-ipc');

ipc.config.id = process.argv[2];
// ipc.config.retry=1500;
ipc.config.rawBuffer = true;
ipc.config.encoding = 'binary';
ipc.config.silent = true;

const _flipImage = (width, height, stride, buffer) => {
  const buffer2 = new Buffer(buffer.length);
  for (let y = 0; y < height; y++) {
    const yBottom = height - y - 1;
    buffer2.set(
      buffer.slice(yBottom * width * stride, (yBottom + 1) * width * stride),
      y * width * stride
    );
  }
  return buffer2;
};

ipc.serve(function() {
  let browserWindow = null;

  let oldData = null;
  const messageEmitter = new EventEmitter();
  ipc.server.on('data', function(data, socket) {
    if (oldData) {
      data = Buffer.concat([oldData, data]);
      oldData = null;
    }
    const datas = [];
    let i;
    for (i = 0; i < data.length; ) {
      if (data.length >= Uint8Array.BYTES_PER_ELEMENT) {
        const length = data.readUInt32LE(i);
        const begin = i + Uint32Array.BYTES_PER_ELEMENT;
        const end = begin + length;
        if (end <= data.length) {
          const d = JSON.parse(data.slice(begin, end).toString('utf8'));
          datas.push(d);
          i = end;
        } else {
          break;
        }
      } else {
        break;
      }
    }
    const tailLength = data.length - i;
    if (tailLength > 0) {
      oldData = Buffer.allocUnsafe(tailLength);
      data.copy(oldData, 0, i, data.length);
    }

    for (let i = 0; i < datas.length; i++) {
      messageEmitter.emit('message', datas[i], socket);
    }
  });

  messageEmitter.on('message', (m, socket) => {
    const {method, id, args} = m;
    switch (method) {
      case 'createBrowserWindow': {
        browserWindow = new BrowserWindow(args);

        const [width, height] = browserWindow.getSize();
        const b = Buffer.from(
          JSON.stringify({
            method: 'response',
            id,
            args: {
              width,
              height
            }
          }),
          'utf8'
        );
        const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
        typeB[0] = 0;
        let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
        lengthB.writeUInt32LE(b.length, 0);
        ipc.server.emit(socket, typeB);
        ipc.server.emit(socket, lengthB);
        ipc.server.emit(socket, b);

        browserWindow.webContents.on('paint', (event, dirty, image) => {
          // message 1
          let b = image.crop(dirty).getBitmap();
          // let b = image.crop(dirty).toBitmap();
          const {width, height} = dirty;
          b = _flipImage(width, height, 4, b);
          // b = Buffer.from(b.toString('hex'), 'ascii');

          let typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
          typeB[0] = 1;

          let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
          lengthB.writeUInt32LE(b.length, 0);

          ipc.server.emit(socket, typeB);
          ipc.server.emit(socket, lengthB);
          ipc.server.emit(socket, b);

          // message 2
          b = Buffer.from(
            JSON.stringify({
              method: 'paint',
              args: dirty
            }),
            'utf8'
          );

          typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
          typeB[0] = 0;

          lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
          lengthB.writeUInt32LE(b.length, 0);

          ipc.server.emit(socket, typeB);
          ipc.server.emit(socket, lengthB);
          ipc.server.emit(socket, b);
        });
        [
          'did-start-loading',
          'did-stop-loading',
          'did-fail-load',
          'did-navigate',
          'dom-ready'
        ].forEach(e => {
          browserWindow.webContents.on(e, () => {
            const b = Buffer.from(
              JSON.stringify({
                method: e
              }),
              'utf8'
            );
            const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
            typeB[0] = 0;
            let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
            lengthB.writeUInt32LE(b.length, 0);
            ipc.server.emit(socket, typeB);
            ipc.server.emit(socket, lengthB);
            ipc.server.emit(socket, b);
          });
        });
        break;
      }
      case 'loadURL': {
        browserWindow.webContents.loadURL(args);
        const b = Buffer.from(
          JSON.stringify({
            method: 'response',
            id
          }),
          'utf8'
        );
        const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
        typeB[0] = 0;
        let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
        lengthB.writeUInt32LE(b.length, 0);
        ipc.server.emit(socket, typeB);
        ipc.server.emit(socket, lengthB);
        ipc.server.emit(socket, b);
        break;
      }
      case 'setFrameRate': {
        browserWindow.webContents.setFrameRate(args);
        const b = Buffer.from(
          JSON.stringify({
            method: 'response',
            id
          }),
          'utf8'
        );
        const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
        typeB[0] = 0;
        let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
        lengthB.writeUInt32LE(b.length, 0);
        ipc.server.emit(socket, typeB);
        ipc.server.emit(socket, lengthB);
        ipc.server.emit(socket, b);
        break;
      }
      case 'insertCSS': {
        browserWindow.webContents.insertCSS(args);
        const b = Buffer.from(
          JSON.stringify({
            method: 'response',
            id
          }),
          'utf8'
        );
        const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
        typeB[0] = 0;
        let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
        lengthB.writeUInt32LE(b.length, 0);
        ipc.server.emit(socket, typeB);
        ipc.server.emit(socket, lengthB);
        ipc.server.emit(socket, b);
        break;
      }
      case 'sendInputEvent': {
        browserWindow.webContents.sendInputEvent(args);
        const b = Buffer.from(
          JSON.stringify({
            method: 'response',
            id
          }),
          'utf8'
        );
        const typeB = Buffer.allocUnsafe(Uint8Array.BYTES_PER_ELEMENT);
        typeB[0] = 0;
        let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
        lengthB.writeUInt32LE(b.length, 0);
        ipc.server.emit(socket, typeB);
        ipc.server.emit(socket, lengthB);
        ipc.server.emit(socket, b);
        break;
      }
      case 'close': {
        browserWindow.close();
        break;
      }
      case 'destroy': {
        browserWindow.destroy();
        break;
      }
      default: {
        console.warn('unknown electron api method:', method);
        break;
      }
    }
  });
});
ipc.server.start();

// process.exit();
