const path = require('path');
const fs = require('fs');
const os = require('os');

const rootPath = path.join('/Users/', os.userInfo().username, '/Library/Application Support/OpenVR/.openvr');
const openvrPathsPath = path.join(rootPath, 'openvrpaths.vrpath');
fs.lstat(openvrPathsPath, (err, stats) => {
  if (err) {
    if (err.code === 'ENOENT') {
      const jsonString = JSON.stringify({
        "config" : [ rootPath ],
        "external_drivers" : null,
        "jsonid" : "vrpathreg",
        "log" : [ rootPath ],
        "runtime" : [
           path.join(__dirname, '/node_modules/native-openvr-deps/bin/osx64'),
         ],
        "version" : 1
      }, null, 2);
      fs.writeFile(openvrPathsPath, jsonString, err => {
        if (err) {
          console.warn(err);
        }
      });
    } else {
      console.warn(err);
    }
  }
});