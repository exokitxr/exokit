/* global assert, beforeEach, describe, it, TEST_URL */
const fs = require('fs');
const path = require('path');

const helpers = require('../helpers');
const exokit = require('../../../src/index');

const imageData = fs.readFileSync(path.resolve(__dirname, '../data/test.png'), 'base64');
const audioData = fs.readFileSync(path.resolve(__dirname, '../data/test.ogg'), 'base64');
const videoData = fs.readFileSync(path.resolve(__dirname, '../data/test.mp4'), 'base64');

describe('HTMLSrcableElement', () => {
  var window;

  beforeEach(async () => {
    window = exokit({
      require: true,
    });

    return await window.evalAsync(`
      const imageDataUri = \`data:image/img;base64,${imageData}\`;
      window.imageDataUri = imageDataUri;
      const audioDataUri = \`data:audio/ogg;base64,${audioData}\`;
      window.audioDataUri = audioDataUri;
      const videoDataUri = \`data:video/mp4;base64,${videoData}\`;
      window.videoDataUri = videoDataUri;
    `);
  });

  afterEach(async () => {
    return await window.destroy();
  });

  describe('<img>', () => {
    it('can setAttribute', async () => {
      return await window.evalAsync(`
        el = document.createElement('img');
        el.onload = () => { done(); };
        el.onerror = err => { done(err); };
        el.setAttribute('src', \`${TEST_URL}/test.png\`);
      `);
    });

    it('can set src', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('img');
        el.onload = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.src = \`${TEST_URL}/test.png\`;
      })`);
    });

    it('can set empty src', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('img');
        el.setAttribute('src', '');
        document.body.appendChild(el);
        setTimeout(() => { accept(); });
      })`);
    });

    it('works with data url', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('img');
        el.onload = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.src = imageDataUri;
      })`);
    });

    it('supports addEventListener', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('img');
        el.addEventListener('load', () => { accept(); });
        el.addEventListener('error', err => { reject(err); });
        el.src = \`${TEST_URL}/test.png\`;
      })`);
    });

    it('is async', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('img');

        let passed = false;
        el.onload = () => {
          if (passed) {
            accept();
          } else {
            reject(new Error('seems sync'));
          }
        };
        el.onerror = err => { reject(err); };

        setImmediate(() => {
          passed = true;
        });

        el.src = \`${TEST_URL}/test.png\`;
      })`);
    });
  });

  helpers.describeSkipCI('<audio>', () => {
    it('can setAttribute', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('audio');
        el.oncanplay = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.setAttribute('src', \`${TEST_URL}/test.ogg\`);
      })`);
    });

    it('can set src', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('audio');
        el.oncanplay = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.src = \`${TEST_URL}/test.ogg\`;
      })`);
    });

    it('can set empty src', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('audio');
        el.setAttribute('src', '');
        document.body.appendChild(el);
        setTimeout(() => { accept(); });
      })`);
    });

    it('works with data url', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('audio');
        el.oncanplay = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.src = audioDataUri;
      })`);
    });

    it('supports addEventListener', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('audio');
        el.addEventListener('canplay', () => { accept(); });
        el.addEventListener('error', err => { reject(err); });
        el.src = \`${TEST_URL}/test.ogg\`;
      })`);
    });

    it('is async', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('audio');

        let passed = false;
        el.oncanplay = () => {
          if (passed) {
            accept();
          } else {
            reject(new Error('seems sync'));
          }
        };
        el.onerror = err => { reject(err); };

        setImmediate(() => {
          passed = true;
        });

        el.src = \`${TEST_URL}/test.ogg\`;
      })`);
    });
  });

  describe.skip('<video>', () => { // XXX
    it('can setAttribute', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('video');
        el.oncanplay = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.setAttribute('src', \`${TEST_URL}/test.mp4\`);
      })`);
    });

    it('can set src', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('video');
        el.oncanplay = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.src = \`${TEST_URL}/test.mp4\`;
      })`);
    })

    it('can set empty src', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('video');
        el.setAttribute('src', '');
        document.body.appendChild(el);
        setTimeout(() => { accept(); });
      })`);
    });

    it('works with data url', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('video');
        el.oncanplay = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.src = videoDataUri;
      })`);
    });

    it('supports addEventListener', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('video');
        el.addEventListener('canplay', () => { accept(); });
        el.addEventListener('error', err => { reject(err); });
        el.src = \`${TEST_URL}/test.mp4\`;
      })`);
    });

    it('is async', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('video');

        let passed = false;
        el.oncanplay = () => {
          if (passed) {
            accept();
          } else {
            reject(new Error('seems sync'));
          }
        };
        passed = true;
        el.onerror = err => { reject(err); };

        setImmediate(() => {
          passed = true;
        });

        el.src = \`${TEST_URL}/test.mp4\`;
      })`);
    });
  });
});
