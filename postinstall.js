const path = require('path');
const fs = require('fs');
const os = require('os');
const mkdirp = require('mkdirp');

const rootPath = path.join(os.userInfo().homedir, '.config/openvr');
const openvrPathsPath = path.join(rootPath, 'openvrpaths.vrpath');
fs.lstat(openvrPathsPath, (err, stats) => {
  if (err) {
    if (err.code === 'ENOENT') {
      mkdirp(rootPath, err => {
        if (!err) {
          const jsonString = JSON.stringify({
            "config" : [ rootPath ],
            "external_drivers" : null,
            "jsonid" : "vrpathreg",
            "log" : [ rootPath ],
            "runtime" : [
               path.join(__dirname, 'node_modules', 'native-openvr-deps/bin/linux64'),
             ],
            "version" : 1
          }, null, 2);
          fs.writeFile(openvrPathsPath, jsonString, err => {
            if (err) {
              throw err;
            }
          });
        } else {
          throw err;
        }
      });
    } else {
      throw err;
    }
  }
});
