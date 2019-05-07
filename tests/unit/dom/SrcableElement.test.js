/* global assert, beforeEach, describe, it, TEST_URL */
const fs = require('fs');
const path = require('path');

const helpers = require('../helpers');
const exokit = require('../../../src/index');

const imageData = fs.readFileSync(path.resolve(__dirname, '../data/test.png'), 'base64');
const audioData = fs.readFileSync(path.resolve(__dirname, '../data/test.ogg'), 'base64');
const videoData = fs.readFileSync(path.resolve(__dirname, '../data/test.mp4'), 'base64');

async function sleep(ms) {
  return new Promise((accept, reject) => {
    setTimeout(() => {
      accept();
    }, ms);
  });
}

describe('HTMLSrcableElement', () => {
  var window;

  beforeEach(async () => {
    window = exokit.make({
      require: true,
    });
  });

  afterEach(async () => {
    console.log('sleep');
    await sleep(10);
    console.log('window.destroy');
    try {
      await window.destroy();
    } catch (err) {
      console.log(err.stack);
    }
    console.log('exokit.exit');
    await exokit.exit();
    console.log('HTMLSrcableElement test DONE');
  });

  describe('<img>', () => {
    it('can setAttribute', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el = document.createElement('img');
        el.onload = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.setAttribute('src', \`${TEST_URL}/test.png\`);
      })`);
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
        const imageDataUri = \`data:image/img;base64,${imageData}\`;
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

  describe.skip('<audio>', () => {
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
        try {
        el = document.createElement('audio');
        el.oncanplay = () => { accept(); };
        el.onerror = err => { reject(err); };
        el.src = \`${TEST_URL}/test.ogg\`;
        } catch (err) {
          console.log(err.stack);
        }
      })`);
    });

    /*
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
        const audioDataUri = \`data:audio/ogg;base64,${audioData}\`;
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
    */
  });

  /*
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
        const videoDataUri = \`data:video/mp4;base64,${videoData}\`;
        window.videoDataUri = videoDataUri;
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
  */
});
