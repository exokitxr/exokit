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
  await browserWindow.loadURL('https://google.com');
  await browserWindow.setFrameRate(15);
  console.log('got');
  browserWindow.on('paint', buffer => {
    console.log('paint', buffer.length);
  });
})();