if (typeof require !== 'undefined') {
  const { spawn } = require('child_process');
  const os = require('os');
  const platform = os.platform();
  const process = require('process');
  const path = require('path');

  const fs = require('fs');
  const http = require('http');

  function launch(flags, url){
      console.log('launching...')
      console.log('flags', flags)
      let flagsArray = ['--experimental-worker', '.', url];
      let flagsString = '';
      if(flags.length > 1){
          flagsString += '-';
          flags.forEach(flag => {
              flagsString += flag;
          });
          flagsArray.push(flagsString);
      }
      else if(flags.length == 1){
          flagsString += '-';
          flagsString += flags[0];
          flagsArray.push(flagsString);
      }
      console.log(flagsArray);
      const ls = spawn('node', flagsArray, {detached: true});

      ls.stdout.on('data', (data) => {
          console.log(`stdout: ${data}`);
      });
      
      ls.stderr.on('data', (data) => {
          console.log(`stderr: ${data}`);
      });
      
      ls.on('close', (code) => {
          console.log(`child process exited with code ${code}`);
      });
  }

  function update(){
      console.log('updating...')

      const downloadPath = '/Users/chris/Downloads/'
      let extension = '';
      let url = '';

      switch (platform) {
          case 'win32':
              url = 'http://get.webmr.io/windows';
              extension = '.exe';
              break;
          case 'darwin':
              url = 'http://get.webmr.io/macos';
              extension = '.dmg';
              break;
          case 'linux':
              url = 'http://get.webmr.io/linux';
              extension = '.bin';
              break;
      }

      let writeStream = fs.createWriteStream(downloadPath + 'exokit-installer'  + extension);
      
      http.get(url, (res) => {

        console.log('Downloading Exokit...');

        const downloadSize = res.headers['content-length' ];
        let chunkSize = 0;
        let prevChunkSize = 0;
        let prevProgress = 0;
        let currentProgress = 0;

        res.on('data', (d) => {
          prevChunkSize = chunkSize;
          prevProgress = ((prevChunkSize / downloadSize) * 100).toFixed(1);
          chunkSize += d.length;
          currentProgress = ((chunkSize / downloadSize) * 100).toFixed(1);
          writeStream.write(d);
          if(prevProgress !== currentProgress){
            iframe.contentWindow.postMessage({
                progress: currentProgress
            })
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
          console.log('finished')
      });
  }
}