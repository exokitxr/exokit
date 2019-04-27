const {spawn} = require('child_process');
const process = require('os').platform();

const fs = require('fs');
const https = require('https');

let exokitPath;
if(process.platform === 'win32'){
    exokitPath = 'scripts\\exokit.cmd';
}
else{
    exokitPath = 'scripts/exokit.sh';
}

function launch(flags){
    console.log('launching...')
    let flagsString = '-';
    if(flags.length > 1){
        flags.forEach(flag => {
            flagsString += flag;
        });
    }
    else{
        flagsString += flag[0];
    }
    spawn(exokitPath, [flagsString]);
}

function update(){
    console.log('updating...')

    const downloadPath = '/Users/chris/Downloads/'

    let writeStream = fs.createWriteStream(downloadPath + 'exokit-installer.exe');
    let url = 'https://get.webmr.io/macos';

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
      let prevChunkSize = 0;
      let prevProgress = 0;
      let currentProgress = 0;

      res.on('data', (d) => {
        prevChunkSize = chunkSize;
        prevProgress = ((prevChunkSize / downloadSize) * 100).toFixed(0);
        chunkSize += d.length;
        currentProgress = ((chunkSize / downloadSize) * 100).toFixed(0);
        writeStream.write(d);
        if(prevProgress !== currentProgress){
          console.log(currentProgress);
        }
      });

      res.on('end', () => {
        console.log('Download complete!');
        writeStream.close();
      });

    }).on('error', (e) => {
      console.error(e);
    });

    writeStream.on('finish', () => {
        console.log('finsihed')
    });
}