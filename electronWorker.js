const {BrowserWindow} = require('electron');
const ipc = require('node-ipc');

ipc.config.id = process.argv[2];
// ipc.config.retry=1500;
ipc.config.rawBuffer=true;
ipc.config.encoding='ascii';
ipc.config.silent=true;

ipc.serve(
    function(){
        let browserWindow = null;

        ipc.server.on(
            'data',
            function(data, socket){
              if (data[0] === 0x7b) {
                data = JSON.parse(data.toString('utf8'));
                const {method, id, args} = data;
                switch (method) {
                  case 'createBrowserWindow': {
                    browserWindow = new BrowserWindow(args);

                    const b = Buffer.from(JSON.stringify({
                      method: 'response',
                      id,
                    }), 'utf8');
                    const lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                    new Uint32Array(lengthB.buffer, lengthB.byteOffset, 1)[0] = b.length;
                    ipc.server.emit(socket, lengthB);
                    ipc.server.emit(socket, b);

                    browserWindow.webContents.on('paint', (event, dirty, image) => {
                      let b = image.toBitmap();
                      let lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                      new Uint32Array(lengthB.buffer, lengthB.byteOffset, 1)[0] = b.length;
                      ipc.server.emit(socket, lengthB);
                      ipc.server.emit(socket, b);
                      
                      b = Buffer.from(JSON.stringify({
                        method: 'paint',
                      }), 'utf8');
                      lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                      new Uint32Array(lengthB.buffer, lengthB.byteOffset, 1)[0] = b.length;
                      ipc.server.emit(socket, lengthB);
                      ipc.server.emit(socket, b);
                    });
                    ['did-start-loading', 'did-stop-loading', 'did-fail-load', 'did-navigate', 'dom-ready'].forEach(e => {
                      browserWindow.webContents.on(e, () => {
                        const b = Buffer.from(JSON.stringify({
                          method: e,
                        }), 'utf8');
                        const lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                        new Uint32Array(lengthB.buffer, lengthB.byteOffset, 1)[0] = b.length;
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
                    const lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                    new Uint32Array(lengthB.buffer, lengthB.byteOffset, 1)[0] = b.length;
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
                    const lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                    new Uint32Array(lengthB.buffer, lengthB.byteOffset, 1)[0] = b.length;
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
                    const lengthB = Buffer.allocUnsafe(Uint32Array.BYTES_PER_ELEMENT);
                    new Uint32Array(lengthB.buffer, lengthB.byteOffset, 1)[0] = b.length;
                    ipc.server.emit(socket, lengthB);
                    ipc.server.emit(socket, b);
                    break;
                  }
                }
              } else {
                // XXX
              }
            }
        );
    }
);
ipc.server.start();

// process.exit();