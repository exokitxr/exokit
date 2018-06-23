#!/usr/bin/env node

const path = require('path');
const fs = require('fs');
const os = require('os');

const packageJson = require('./package.json');
const bugsnag = require('bugsnag');

const bugsnagApiKey = (() => {
  try {
    return fs.readFileSync(path.join(__dirname, 'bugsnag.txt'), 'utf8').match(/^(\S*)/)[1];
  } catch (err) {
    if (err.code !== 'ENOENT') {
      console.warn(err.stack);
    }
    return null;
  }
})();
if (bugsnagApiKey) {
  bugsnag.register(bugsnagApiKey, {
    metaData: {
      packageJson,
      arch: os.arch(),
      cpus: os.cpus(),
      endianness: os.endianness(),
      homedir: os.homedir(),
      hostname: os.hostname(),
      loadavg: os.loadavg(),
      networkInterfaces: os.networkInterfaces(),
      freemem: os.freemem(),
      platform: os.platform(),
      release: os.release(),
      tmpdir: os.tmpdir(),
      totalmem: os.totalmem(),
      type: os.type(),
      uptime: os.uptime(),
      userInfo: os.userInfo(),
    },
  });
}

if (require.main === module) {
  const bs = [];
  process.stdin.on('data', d => {
    bs.push(d);
  });
  process.stdin.on('end', d => {
    const b = Buffer.concat(bs);
    const s = b.toString('utf8');
    const err = new Error(s);
    bugsnag.notify(err, {
      stack: s,
    });
  });
}