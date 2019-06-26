const os = require('os');

const {
  getGamepads,
  getHMDType,
} = require('./VR.js');
const {
  nativeAudio: {
    MicrophoneMediaStream,
  },
  nativeVideo: {
    VideoDevice,
  },
  nativeWindow,
} = require('./native-bindings');
const GlobalContext = require('./GlobalContext');

function getUserMedia(constraints) {
  if (constraints.audio) {
    return Promise.resolve(new MicrophoneMediaStream());
  } else if (constraints.video) {
    const dev = new VideoDevice();
    dev.constraints = constraints.video;
    return Promise.resolve(dev);
  } else {
    return Promise.reject(new Error('constraints not met'));
  }
}

class MediaDevices {
  constructor() {
    this.getUserMedia = getUserMedia;
  }
  enumerateDevices() {
    let deviceIds = 0;
    let groupIds = 0;
    return Promise.resolve([
      {
        deviceId: (++deviceIds) + '',
        groupId: (++groupIds) + '',
        kind: 'audioinput',
        label: 'Microphone',
      },
    ]);
  }
}
module.exports.MediaDevices = MediaDevices;

class Clipboard {
  read() {
    return Promise.resolve(); // TODO
  }
  readText() {
    return new Promise(resolve => {
      resolve(nativeWindow.getClipboard().slice(0, 256));// why do we slice this?
    });
  }
  write() {
    return Promise.resolve(); // TODO
  }
  writeText(clipboardContents) {
    return new Promise(resolve => {
      nativeWindow.setClipboard(clipboardContents);
      resolve();
    });
  }
}
module.exports.Clipboard = Clipboard;

class Navigator {
  constructor() {
    this.userAgent = `Mozilla/5.0 (OS) AppleWebKit/999.0 (KHTML, like Gecko) Chrome/999.0.0.0 Safari/999.0 Exokit/${GlobalContext.version}`;
    this.vendor = 'Exokit';
    this.platform = os.platform();
    this.hardwareConcurrency = os.cpus().length;
    this.appCodeName = 'Mozilla';
    this.appName = 'Netscape';
    this.appVersion = '5.0';
    this.language = 'en-US';
    this.mediaDevices = new MediaDevices();
    this.clipboard = new Clipboard();
    this.webkitGetUserMedia = getUserMedia; // for feature detection
  }
  getVRDisplaysSync() {
    return getHMDType() ? [window[symbols.mrDisplaysSymbol].vrDisplay] : [];
  }
  getGamepads() {
    return getGamepads();
  }
}
module.exports.Navigator = Navigator;