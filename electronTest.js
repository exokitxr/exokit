const electron = require('./electron');

(async () => {
  const e = await electron();
  const browserWindow = await e.createBrowserWindow({
    width: 1080,
    height: 640,
    show: false,
    frame: false,
    webPreferences: {
      offscreen: true,
      transparent: true,
    },
  });
  browserWindow.on('dom-ready', () => {
    console.log('dom ready');
  });
  browserWindow.on('paint', buffer => {
    console.log('paint', buffer.length);
  });
  await browserWindow.loadURL('https://google.com');
  await browserWindow.setFrameRate(15);
  await browserWindow.insertCSS(`
    ::-webkit-scrollbar {
      height: 30px;
      width: 30px;
      background: #e0e0e0;
    }
    
    ::-webkit-scrollbar-thumb {
      background: #4db6ac;
    }
    
    ::-webkit-scrollbar-corner {
      background: #cfcfcf;
    }
  `);
})();