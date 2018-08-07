const events = require('events');
const {EventEmitter} = events;
const path = require('path');
const child_process = require('child_process');
const ipc = require('node-ipc');

const electronPath = path.join(require.resolve('electron'), '..', 'cli.js');
const electronWorkerPath = path.join(__dirname, 'electronWorker.js');

ipc.config.id = 'hello';
// ipc.config.retry=1500;
ipc.config.rawBuffer=true;
ipc.config.encoding='ascii';
ipc.config.silent=true;

let ids = 0;
const electron = () => new Promise((accept, reject) => {
  const id = String('client-' + (ids++));
  const cp = child_process.fork(electronPath, [electronWorkerPath, id], {
    stdio: 'pipe',
  });
  // cp.stdin.end();
  cp.stdout.pipe(process.stdout);
  cp.stderr.pipe(process.stderr);
  cp.on('exit', () => {
    console.log('exit');
  });

  setTimeout(() => {
    ipc.connectTo(
      id,
      function(){
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
            const datas = [];
            let i;
            for (i = 0; i < data.length;) {
              const length = data.readUInt32LE(i);
              const begin = i + Uint32Array.BYTES_PER_ELEMENT;
              const end = begin + length;
              if (end <= data.length) {
                datas.push(data.slice(begin, end));
                i = end;
              } else {
                break;
              }
            }
            if (i < data.length) {
              oldData = data.slice(i);
            }

            for (let i = 0; i < datas.length; i++) {
              let data = datas[i];
              if (data[0] === 0x7b) {
                data = JSON.parse(data);
                const {method, args} = data;
                if (method === 'response') {
                  const {id} = data;
                  cbEmitter.emit('response', {
                    id,
                    args,
                  });
                } else {
                  messageEmitter.emit('message', {
                    method,
                    args,
                  });
                }
              } else {
                buffer = data;
              }
            }
          });
          const _waitForResponse = (id, cb) => {
            const _response = res => {
              if (res.id === id) {
                cb(res.args);
              }
            };
            cbEmitter.on('response', _response);
          };

          localChannel.on(
              'connect',
              function(){
                  accept({
                    createBrowserWindow(args) {
                      return new Promise((accept, reject) => {
                        const id = ids++;
                        localChannel.emit(JSON.stringify({
                          method: 'createBrowserWindow',
                          id,
                          args,
                        }));

                        _waitForResponse(id, () => {
                          class BrowserWindow extends EventEmitter {
                            loadURL(u) {
                              return new Promise((accept, reject) => {
                                const id = ids++;
                                localChannel.emit(JSON.stringify({
                                  method: 'loadURL',
                                  args: u,
                                }));
                                _waitForResponse(id, () => {
                                  accept();
                                });
                              });
                            }
                            setFrameRate(frameRate) {
                              return new Promise((accept, reject) => {
                                const id = ids++;
                                localChannel.emit(JSON.stringify({
                                  method: 'setFrameRate',
                                  args: frameRate,
                                }));
                                _waitForResponse(id, () => {
                                  accept();
                                });
                              });
                            }
                          }
                          const browserWindow = new BrowserWindow();
                          messageEmitter.on('message', m => {
                            const {method} = m;
                            if (['did-start-loading', 'did-stop-loading', 'did-fail-load', 'did-navigate', 'dom-ready'].includes(method)) {
                              browserWindow.emit(method);
                            } else if (method === 'paint') {
                              browserWindow.emit('paint', buffer);
                            }
                          });
                          accept(browserWindow);
                        });
                      });
                    },
                  });
              }
          );
      }
    );
  }, 1000);
});

module.exports = electron;