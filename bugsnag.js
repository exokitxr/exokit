#!/usr/bin/env node

const path = require('path');
const fs = require('fs');
const os = require('os');

const packageJson = require('./package.json');
const bugsnag = require('bugsnag');

const GlobalContext = require('./src/GlobalContext');

const IgnoredStrings = [
  'Failed to set local answer sdp: Called in wrong state: STATE_INPROGRESS',
  'Failed to set local offer sdp: Called in wrong state: STATE_INPROGRESS',
  'Failed to set remote answer sdp: Called in wrong state: STATE_INPROGRESS',
  'Failed to set local offer sdp: Failed to push down transport description: Local fingerprint does not match identity.',
];

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
      argv: process.argv,
      command: GlobalContext.commands,
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

  const Configuration = require('bugsnag/lib/configuration');

  function shouldIgnoreError(err) {
    for (const pattern of IgnoredStrings) {
      if (err.message.indexOf(pattern) >= 0) {
        return true;
      }
    }
    return false;
  }

  Configuration.beforeNotifyCallbacks.push(function (report) {
    for (const evt of report.events) {
      for (const err of evt.exceptions) {
        if (shouldIgnoreError(err)) {
          console.warn('Bugsnag ignored error: ');
          console.warn(err);
          report.ignore();
          return false;
        }
      }
    }
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
