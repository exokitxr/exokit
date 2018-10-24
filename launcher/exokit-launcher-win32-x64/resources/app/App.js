const {BrowserWindow} = require('electron');
const {app} = require('electron');
const {ipcMain} = require('electron');
const { spawn } = require('child_process');
const path = require('path');
const url = require('url');
const https = require('https');
const fs = require('fs');

const exokitPath = 'C:\\Users\\ceddy\\Documents\\GitHub\\exokit\\scripts\\exokit.cmd'; // have to make this relative

let window = null;

// Wait until the app is ready
app.once('ready', () => {

  // Create a new window
  window = new BrowserWindow({
    // Set the initial width to 800px
    width: 1200,
    // Set the initial height to 600px
    height: 800,
    // Set the default background color of the window to match the CSS
    // background color of the page, this prevents any white flickering
    backgroundColor: '#D6D8DC',
    // Don't show the window until it's ready, this prevents any white flickering
    show: false,

    icon: path.join(__dirname, 'ExokitLogo.png')
  });

  // Load a URL in the window to the local index.html path
  window.loadURL(url.format({
    pathname: path.join(__dirname, 'index.html'),
    protocol: 'file:',
    slashes: true
  }));

  // Show window when page is ready
  window.once('ready-to-show', () => {
    window.show();
  });
});

// Accept communication from frontend
ipcMain.on('asynchronous-message', (event, arg) => {
  switch (arg) {
    case 'terminal':
      spawn(exokitPath, [], {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
      event.returnValue = 'Launching Terminal...';
      break;
    case 'exohome':
      spawn(exokitPath, ['-h'], {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
      event.returnValue = 'Launching ExoHome... ';
      break;
    case 'version':
      const version = spawn(exokitPath, ['-v']);
      let stdout = '';
      version.stdout.on('data', (data) => {
        stdout += String(data);
      });
      version.once('exit', function(){
        event.sender.send('asynchronous-reply', stdout);

        let writeStream = fs.createWriteStream('C:\\Users\\ceddy\\Downloads\\exokit-installer.exe');

        let url = '';

        console.log('Detected OS:', process.platform);

        switch (process.platform) {
          case 'win32':
            url = 'https://get.webmr.io/windows';
            break;
          case 'darwin':
            url = 'https://get.webmr.io/macos';
            break;
          case 'linux':
            url = 'https://get.webmr.io/linux';
            break;
        }

        https.get(url, (res) => {

          console.log('Downloading Exokit...');

          const downloadSize = res.headers['content-length' ];
          let chunkSize = 0;

          res.on('data', (d) => {
            chunkSize += d.length;
            writeStream.write(d);
            event.sender.send('asynchronous-reply', chunkSize / downloadSize);
          });

          res.on('end', () => {
            console.log('Download complete!');
            writeStream.close();
          });

        }).on('error', (e) => {
          console.error(e);
        });

        writeStream.on('finish', () => {
          launchInstaller();
        });
      });
      break;
    default:
      event.returnValue = 'message does not make sense to electron backend';
      break;
  }
});


function launchInstaller(){
  spawn('C:\\Users\\ceddy\\Downloads\\exokit-installer.exe', [], {detached: true, stdio: ['ignore', 'ignore', 'ignore']});
}
