    
'use strict'

const electron = require('electron')
const app = electron.app
const BrowserWindow = electron.BrowserWindow
const path = require('path')
const url = require('url')

let mainWindow

function createWindow () {
  mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    show: false,
    webPreferences: {
      offscreen: true,
    },
  })
  mainWindow.loadURL('http://facebook.com');

  mainWindow.webContents.on('paint', (event, dirty, image) => {
    {
      const b = Uint32Array.from([dirty.x, dirty.y, dirty.width, dirty.height]);
      const b2 = Buffer.from(b.buffer, b.byteOffset, b.byteLength);
      process.stdout.write(b2);
    }
    const i = image.crop(dirty);
    const i2 = i.getBitmap();
    process.stdout.write(i2);
    console.warn('got dirty', dirty, i2.byteLength);
    // updateBitmap(dirty, image.getBitmap())
  });

  mainWindow.on('closed', function () {
    mainWindow = null
  });
}

app.on('ready', createWindow);

app.on('window-all-closed', function () {
  if (process.platform !== 'darwin') {
    app.quit()
  }
});

app.on('activate', function () {
  if (mainWindow === null) {
    createWindow()
  }
});

console.warn('error');