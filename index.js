const path = require('path');
const fs = require('fs');
const {http, https} = require('follow-redirects');
const childProcess = require('child_process');

const mkdirp = require('mkdirp');
const touch = require('touch');
const tar = require('tar');
const spog = require('spog');
const electron = require('electron');
const {app, ipcMain, remote, BrowserWindow, Menu, MenuItem} = electron;

const log = (() => {
  for (let i = 2; i < process.argv.length; i++) {
    const arg = process.argv[i];
    const match = arg.match(/^log=(.+)$/);
    if (match) {
      return match[1];
    }
  }
  return null;
})();
const console = log !== null ? spog.createConsole(log) : global.console;

const serverLib = require('./lib/server.js')({console});

const command = (() => {
  for (let i = 2; i < process.argv.length; i++) {
    const arg = process.argv[i];
    const match = arg.match(/^(install)$/);
    if (match) {
      return match[1];
    }
  }
  return null;
})();

if (command === null) {
  const url = (() => {
    for (let i = 2; i < process.argv.length; i++) {
      const arg = process.argv[i];
      const match = arg.match(/^url=(.+)$/);
      if (match) {
        return match[1];
      }
    }
    return null;
  })();
  if (url) {
    app.commandLine.appendSwitch('high-dpi-support', 'true');
    app.commandLine.appendSwitch('force-device-scale-factor', '1');

    const _requestAppReady = () => new Promise((accept, reject) => {
      app.on('ready', () => {
        accept();
      });
      app.on('error', err => {
        reject(err);
      });
    });

    _requestAppReady()
      .then(() => {
        const logStreams = {};
        ipcMain.on('ipc', (event, e) => {
          const {method} = e;
          switch (method) {
            case 'back': {
              if (win.webContents.canGoBack()) {
                win.webContents.goBack();
              }
              break;
            }
            case 'forward': {
              if (win.webContents.canGoForward()) {
                win.webContents.goForward();
              }
              break;
            }
            case 'stop': {
              win.webContents.stop();
              break;
            }
            case 'refresh': {
              win.webContents.reloadIgnoringCache();
              break;
            }
            case 'show': {
              win.show();
              break;
            }
            case 'hide': {
              win.hide();
              break;
            }
            case 'subclose': {
              event.sender.emit('subclose');
              break;
            }
            case 'contextmenu': {
              event.sender.emit('contextmenu');
              break;
            }
            case 'requestHistoryState': {
              const {args: [id]} = e;

              const historyState = {
                canGoBack: event.sender.canGoBack(),
                canGoForward: event.sender.canGoForward(),
              };
              win.webContents.send('ipc', {
                method: 'response',
                args: [id, null, historyState],
              });
              break;
            }
            case 'requestLocalServers': {
              const {args: [id]} = e;

              serverLib.requestLocalServers()
                .then(serverSpecs => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, null, serverSpecs],
                  });
                })
                .catch(err => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, err.stack, null],
                  });
                });
              break;
            }
            case 'createLocalServer': {
              const {args: [id, name]} = e;

              serverLib.spawnCreateLocalServer({
                name,
              })
                .then(serverSpec => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, null, serverSpec],
                  });
                })
                .catch(err => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, err.stack, null],
                  });
                });
              break;
            }
            case 'removeLocalServer': {
              const {args: [id, name]} = e;

              serverLib.removeLocalServer({
                name,
              })
                .then(() => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, null],
                  });
                })
                .catch(err => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, err.stack],
                  });
                });
              break;
            }
            case 'reinstallLocalServer': {
              const {args: [id, name]} = e;

              serverLib.spawnReinstallLocalServer({
                name,
              })
                .then(serverSpec => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, null, serverSpec],
                  });
                })
                .catch(err => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, err.stack, null],
                  });
                });
              break;
            }
            case 'startServer': {
              const {args: [id, name]} = e;

              serverLib.startServer({
                name,
              })
                .then(serverSpec => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, null],
                  });
                })
                .catch(err => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, err.stack],
                  });
                });
              break;
            }
            case 'stopServer': {
              const {args: [id, name]} = e;

              serverLib.stopServer({
                name,
              })
                .then(serverSpec => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, null],
                  });
                })
                .catch(err => {
                  win.webContents.send('ipc', {
                    method: 'response',
                    args: [id, err.stack],
                  });
                });
              break;
            }
            case 'createLogStream': {
              const {args: [id, name]} = e;

              const s = serverLib.createLogStream({
                name,
              });
              s.setEncoding('base64');
              s.on('data', data => {
                win.webContents.send('ipc', {
                  method: 'data',
                  args: [id, data],
                });
              });
              s.on('end', data => {
                win.webContents.send('ipc', {
                  method: 'response',
                  args: [id, null],
                });

                logStreams[id] = null; // XXX can be delete
              });
              s.on('error', err => {
                win.webContents.send('ipc', {
                  method: 'response',
                  args: [id, err.stack],
                });

                logStreams[id] = null; // XXX can be delete
              });
              logStreams[id] = s;
              break;
            }
            case 'closeLogStream': {
              const {args: [id]} = e;

              logStreams[id].destroy();
              logStreams[id] = null; // XXX can be delete
              break;
            }
            default: {
              console.warn('got invalid ipc method: ' + method);
              break;
            }
          }
        });

        const zoomFactor = 1.5;
        const win = new BrowserWindow({
          width: 1280 * zoomFactor,
          height: 1024 * zoomFactor,
          // show: false,
          icon: path.join(__dirname, 'icon.png'),
          // frame: false,
          title: 'ZEO',
          titleBarStyle: 'hidden',
          autoHideMenuBar: true,
          // thickFrame: false,
          // backgroundThrottling: false,
          // darkTheme: true,
          webPreferences: {
            preload: path.join(__dirname, 'lib', 'api-site.js'),
            zoomFactor,
            // webSecurity: false,
          },
        });
        /* const devtools = new BrowserWindow({
          webPreferences: {
            zoomFactor,
          },
        }); */
        win.loadURL(url);
        // win.webContents.setDevToolsWebContents(devtools.webContents);
        win.on('page-title-updated', e => {
          e.preventDefault();
        });
        win.on('app-command', (e, cmd) => {
          if (cmd === 'browser-backward' && win.webContents.canGoBack()) {
            win.webContents.goBack();
          } else if (cmd === 'browser-forward' && win.webContents.canGoForward()) {
            win.webContents.goForward();
          }
        });
        win.webContents.on('new-window', (e, url) => {
          e.preventDefault();

          const subwin = new BrowserWindow({
            width: 1280 * zoomFactor,
            height: 1024 * zoomFactor,
            // show: false,
            icon: path.join(__dirname, 'icon.png'),
            // frame: false,
            title: url + ' - ZEO server',
            // titleBarStyle: 'hidden',
            autoHideMenuBar: true,
            // thickFrame: false,
            backgroundThrottling: false,
            // darkTheme: true,
            webPreferences: {
              preload: path.join(__dirname, 'lib', 'api-server.js'),
              zoomFactor,
              // webSecurity: false,
            },
          });
          subwin.loadURL(url);
          subwin.webContents.openDevTools({
            mode: 'bottom',
          });

          subwin.webContents.on('did-fail-load', () => {
            console.warn('subwin failed load', {url});
          });
          subwin.webContents.on('subclose', () => {
            subwin.close();
          });

          e.newGuest = subwin;
        });
        win.webContents.openDevTools({
          mode: 'bottom',
        });
        /* win.webContents.on('did-fail-load', () => {
          process.exit(1);
        }); */
        win.on('page-title-updated', e => {
          e.preventDefault();
        });
        win.webContents.on('did-start-loading', () => {
          win.webContents.send('ipc', {
            method: 'loadstart',
          });
        });
        win.webContents.on('did-stop-loading', () => {
          win.webContents.send('ipc', {
            method: 'loadstop',
          });
        });
        win.webContents.on('contextmenu', () => {
          const menu = new Menu();
          menu.append(new MenuItem({label: 'Cut', click() {
            win.webContents.send('ipc', {
              method: 'cut',
            });
          }}));
          menu.append(new MenuItem({label: 'Copy', click() {
            win.webContents.send('ipc', {
              method: 'copy',
            });
          }}));
          menu.append(new MenuItem({label: 'Paste', click() {
            win.webContents.send('ipc', {
              method: 'paste',
            });
          }}));
          menu.append(new MenuItem({type: 'separator'}));
          menu.append(new MenuItem({label: 'Undo', click() {
            win.webContents.send('ipc', {
              method: 'undo',
            });
          }}));
          menu.append(new MenuItem({label: 'Redo', click() {
            win.webContents.send('ipc', {
              method: 'redo',
            });
          }}));
          menu.popup(win);
        });
        win.webContents.on('crashed', () => {
          process.exit(0);
        });
        /* win.webContents.on('devtools-closed', () => {
          process.exit(0);
        }); */
      })
      .catch(err => {
        console.warn(err.stack);
        process.exit(1);
      });
  } else {
    console.warn('usage: run.cmd url=[URL]');
    process.exit(1);
  }
} else if (command === 'install') {
  const name = (() => {
    for (let i = 2; i < process.argv.length; i++) {
      const arg = process.argv[i];
      const match = arg.match(/^name=(.+)$/);
      if (match) {
        return match[1];
      }
    }
    return null;
  })();
  const port = (() => {
    for (let i = 2; i < process.argv.length; i++) {
      const arg = process.argv[i];
      const match = arg.match(/^port=(.+)$/);
      if (match) {
        const port = parseInt(match[1], 10);
        if (!isNaN(port) && isFinite(port) && port > 0) {
          return port;
        }
      }
    }
    return null;
  })();
  if (name && port) {
    serverLib.createLocalServer({
      name,
      port,
    })
      .then(() => {
        process.exit();
      })
      .catch(err => {
        console.warn(err.stack);
        process.exit(1);
      });
  } else {
    console.warn('usage: run.cmd install name=[NAME] port=[PORT]');
    process.exit(1);
  }
} else {
  console.warn('unknown command: ' + command);
  process.exit(1);
}
