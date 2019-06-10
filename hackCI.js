const { spawn } = require('child_process');
const fetch = require('node-fetch');
const _ =  require('lodash');

let getPrevHash = new Promise(function(resolve, reject) {
  const prevHash = spawn('git', ['rev-parse', 'HEAD~1']);
  prevHash.stdout.on('data', (data) => {
    console.log(`Prev Hash: ${data}`);
    resolve(data);
  });
  prevHash.stderr.on('data', (data) => {
    console.log(`stderr: ${data}`);
    reject()
  });
  prevHash.on('close', (code) => {
    console.log(`child process exited with code ${code}`);
  });
})

let getCurrentHash = new Promise(function(resolve, reject) {
  const prevHash = spawn('git', ['rev-parse', 'HEAD']);
  prevHash.stdout.on('data', (data) => {
    console.log(`Current Hash: ${data}`);
    resolve(data);
  });
  prevHash.stderr.on('data', (data) => {
    console.log(`stderr: ${data}`);
    reject()
  });
  prevHash.on('close', (code) => {
    console.log(`child process exited with code ${code}`);
  });
})

async function main(){
  let prevHash = await getPrevHash;
  let currentHash = await getCurrentHash;
  prevHash = prevHash.toString();
  currentHash = prevHash.toString();
  // GET PREVIOUS COMMIT PACKAGE.JSON
  fetch('https://raw.githubusercontent.com/exokitxr/exokit/' + prevHash.slice(0, -3) + '/package.json',)
  .then(function(response) {
    return response.json();
  })
  .then(function(json) {
    const prevPackageJSON = json;
    // GET CURRENT COMMIT PACKAGE.JSON
      fetch('https://raw.githubusercontent.com/exokitxr/exokit/' + currentHash.slice(0, -3) + '/package.json',)
    .then(function(response) {
      return response.json();
    })
    .then(function(json) {
      const currentPackageJSON = json;
      // COMPARE PACKAGE.JSON's, could be more specific but for now just compare full file.
      if(_.isEqual(prevPackageJSON, currentPackageJSON)){
        console.log('Skipping install');
      }
      else{
        // NPM INSTALL / INSTALL STUFF HERE
        const npmInstall = spawn('npm', ['install', '-g', 'appdmg']);
        npmInstall.stdout.on('data', (data) => {
          console.log(data);
        });
        npmInstall.stderr.on('data', (data) => {
          console.log(`stderr: ${data}`);
        });
        npmInstall.on('close', (code) => {
          console.log(`child process exited with code ${code}`);
        });
      }
    })
  })
}

main();

