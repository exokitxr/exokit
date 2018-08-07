const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const child_process = require('child_process');
const ipc = require('node-ipc');

const electronPath = path.join(require.resolve('electron'), '..', 'cli.js');
const electronWorkerPath = path.join(__dirname, 'electronWorker.js');

ipc.config.id = 'hello';
// ipc.config.retry=1500;
ipc.config.rawBuffer = true;
ipc.config.encoding = 'binary';
ipc.config.silent = true;

let ids = 0;
const electron = () =>
  new Promise((accept, reject) => {
    const id = String('client-' + ids++);
    const cp = child_process.fork(electronPath, [electronWorkerPath, id], {
      stdio: 'pipe'
    });
    // cp.stdin.end();
    cp.stdout.pipe(process.stdout);
    cp.stderr.pipe(process.stderr);
    cp.on('exit', () => {
      console.log('exit');
    });

    setTimeout(() => {
      ipc.connectTo(id, function() {
        const localChannel = ipc.of[id];

        let ids = 0;
        let oldData = null;
        const cbEmitter = new EventEmitter();
        const messageEmitter = new EventEmitter();
        let buffer = null;
        localChannel.on('data', data => {
          if (oldData) {
            data = Buffer.concat([oldData, data]);
            oldData = null;
          }
          // console.log('got data', data.slice(0, 256));
          const datas = [];
          let i;
          for (i = 0; i < data.length; ) {
            if (
              data.length - i >=
              Uint8Array.BYTES_PER_ELEMENT + Uint32Array.BYTES_PER_ELEMENT
            ) {
              const type = data.readUInt8(i);
              const length = data.readUInt32LE(
                i + Uint8Array.BYTES_PER_ELEMENT
              );
              const begin =
                i +
                Uint8Array.BYTES_PER_ELEMENT +
                Uint32Array.BYTES_PER_ELEMENT;
              const end = begin + length;
              if (end <= data.length) {
                let d = data.slice(begin, end);
                if (type === 0) {
                  d = JSON.parse(d.toString('utf8'));
                } else if (type === 1) {
                  // d = Buffer.from(d.toString('ascii'), 'hex');
                } else {
                  console.warn('invalid message type', type);
                }
                datas.push(d);
                i = end;
              } else {
                break;
              }
            } else {
              break;
            }
          }
          if (i < data.length) {
            oldData = data.slice(i);
          }

          for (let i = 0; i < datas.length; i++) {
            let data = datas[i];
            if (data instanceof Buffer) {
              buffer = data;
            } else {
              // data = JSON.parse(data);
              const {method, args} = data;
              if (method === 'response') {
                const {id} = data;
                cbEmitter.emit('response', {
                  id,
                  args
                });
              } else {
                messageEmitter.emit('message', {
                  method,
                  args
                });
              }
            }
          }
        });
        const _waitForResponse = (id, cb) => {
          const _response = res => {
            if (res.id === id) {
              cb(res.args);

              cbEmitter.removeListener('response', _response);
            }
          };
          cbEmitter.on('response', _response);
        };

        localChannel.on('connect', function() {
          accept({
            createBrowserWindow(args) {
              return new Promise((accept, reject) => {
                const id = ids++;
                localChannel.emit(
                  JSON.stringify({
                    method: 'createBrowserWindow',
                    id,
                    args
                  })
                );

                _waitForResponse(id, ({width, height}) => {
                  class BrowserWindow extends EventEmitter {
                    constructor(width, height) {
                      super();

                      this.width = width;
                      this.height = height;
                    }
                    loadURL(u) {
                      return new Promise((accept, reject) => {
                        const id = ids++;
                        localChannel.emit(
                          JSON.stringify({
                            method: 'loadURL',
                            id,
                            args: u
                          })
                        );
                        _waitForResponse(id, () => {
                          accept();
                        });
                      });
                    }
                    setFrameRate(frameRate) {
                      return new Promise((accept, reject) => {
                        const id = ids++;
                        localChannel.emit(
                          JSON.stringify({
                            method: 'setFrameRate',
                            id,
                            args: frameRate
                          })
                        );
                        _waitForResponse(id, () => {
                          accept();
                        });
                      });
                    }
                    insertCSS(css) {
                      return new Promise((accept, reject) => {
                        const id = ids++;
                        localChannel.emit(
                          JSON.stringify({
                            method: 'insertCSS',
                            id,
                            args: css
                          })
                        );
                        _waitForResponse(id, () => {
                          accept();
                        });
                      });
                    }
                    sendInputEvent(event) {
                      return new Promise((accept, reject) => {
                        const id = ids++;
                        localChannel.emit(
                          JSON.stringify({
                            method: 'sendInputEvent',
                            id,
                            args: event
                          })
                        );
                        _waitForResponse(id, () => {
                          accept();
                        });
                      });
                    }
                  }
                  const browserWindow = new BrowserWindow(width, height);
                  messageEmitter.on('message', m => {
                    const {method, args} = m;
                    if (
                      [
                        'did-start-loading',
                        'did-stop-loading',
                        'did-fail-load',
                        'did-navigate',
                        'dom-ready'
                      ].includes(method)
                    ) {
                      browserWindow.emit(method);
                    } else if (method === 'paint') {
                      const {x, y, width, height} = args;
                      browserWindow.emit('paint', {
                        x,
                        y,
                        width,
                        height,
                        data: new Uint8Array(
                          buffer.buffer,
                          buffer.byteOffset,
                          buffer.byteLength
                        )
                      });
                    }
                  });
                  accept(browserWindow);
                });
              });
            }
          });
        });
      });
    }, 1000);
  });

module.exports = electron;
