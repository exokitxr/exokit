/* global assert, beforeEach, describe, it, TEST_URL */
const fs = require('fs');
const path = require('path');

const helpers = require('../helpers');
const exokit = require('../../../src/index');

const imageData = fs.readFileSync(path.resolve(__dirname, '../data/test.png'), 'base64');
const imageDataUri = `data:image/img;base64,${imageData}`;

const audioData = fs.readFileSync(path.resolve(__dirname, '../data/test.ogg'), 'base64');
const audioDataUri = `data:audio/ogg;base64,${audioData}`;

const videoData = fs.readFileSync(path.resolve(__dirname, '../data/test.mp4'), 'base64');
const videoDataUri = `data:video/mp4;base64,${videoData}`;

describe('HTMLSrcableElement', () => {
  var window;
  var document;
  var el;

  beforeEach(() => {
    const o = exokit.make();
    window = o.window;
    window.navigator.getVRDisplaysSync = () => [];
    document = o.document;
  });

  afterEach(() => {
    window.destroy();
  });

  describe('<img>', () => {
    it('can setAttribute', done => {
      el = document.createElement('img');
      el.onload = () => { done(); };
      el.onerror = err => { done(err); };
      el.setAttribute('src', `${TEST_URL}/test.png`);
    });

    it('can set src', done => {
      el = document.createElement('img');
      el.onload = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = `${TEST_URL}/test.png`;
    });

    it('can set empty src', done => {
      el = document.createElement('img');
      el.setAttribute('src', '');
      document.body.appendChild(el);
      setTimeout(() => { done(); });
    });

    it('works with data url', done => {
      el = document.createElement('img');
      el.onload = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = imageDataUri;
    });

    it('supports addEventListener', done => {
      el = document.createElement('img');
      el.addEventListener('load', () => { done(); });
      el.addEventListener('error', err => { done(err); });
      el.src = `${TEST_URL}/test.png`;
    });

    it('is async', done => {
      el = document.createElement('img');

      let passed = false;
      el.onload = () => {
        if (passed) {
          done();
        } else {
          done(new Error('seems sync'));
        }
      };
      el.onerror = err => { done(err); };

      process.nextTick(() => {
        passed = true;
      });

      el.src = `${TEST_URL}/test.png`;
    });
  });

  helpers.describeSkipCI('<audio>', () => {
    it('can setAttribute', done => {
      el = document.createElement('audio');
      el.oncanplay = () => { done(); };
      el.onerror = err => { done(err); };
      el.setAttribute('src', `${TEST_URL}/test.ogg`);
    });

    it('can set src', done => {
      el = document.createElement('audio');
      el.oncanplay = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = `${TEST_URL}/test.ogg`;
    });

    it('can set empty src', done => {
      el = document.createElement('audio');
      el.setAttribute('src', '');
      document.body.appendChild(el);
      setTimeout(() => { done(); });
    });

    it('works with data url', done => {
      el = document.createElement('audio');
      el.oncanplay = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = audioDataUri;
    });

    it('supports addEventListener', done => {
      el = document.createElement('audio');
      el.addEventListener('canplay', () => { done(); });
      el.addEventListener('error', err => { done(err); });
      el.src = `${TEST_URL}/test.ogg`;
    });

    it('is async', done => {
      el = document.createElement('audio');

      let passed = false;
      el.oncanplay = () => {
        if (passed) {
          done();
        } else {
          done(new Error('seems sync'));
        }
      };
      el.onerror = err => { done(err); };

      process.nextTick(() => {
        passed = true;
      });

      el.src = `${TEST_URL}/test.ogg`;
    });
  });

  describe('<video>', () => {
    it('can setAttribute', done => {
      el = document.createElement('video');
      el.oncanplay = () => { done(); };
      el.onerror = err => { done(err); };
      el.setAttribute('src', `${TEST_URL}/test.mp4`);
    });

    it('can set src', done => {
      el = document.createElement('video');
      el.oncanplay = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = `${TEST_URL}/test.mp4`;
    })

    it('can set empty src', done => {
      el = document.createElement('video');
      el.setAttribute('src', '');
      document.body.appendChild(el);
      setTimeout(() => { done(); });
    });

    it('works with data url', done => {
      el = document.createElement('video');
      el.oncanplay = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = videoDataUri;
    });

    it('supports addEventListener', done => {
      el = document.createElement('video');
      el.addEventListener('canplay', () => { done(); });
      el.addEventListener('error', err => { done(err); });
      el.src = `${TEST_URL}/test.mp4`;
    });

    it('is async', done => {
      el = document.createElement('video');

      let passed = false;
      el.oncanplay = () => {
        if (passed) {
          done();
        } else {
          done(new Error('seems sync'));
        }
      };
      passed = true;
      el.onerror = err => { done(err); };

      process.nextTick(() => {
        passed = true;
      });

      el.src = `${TEST_URL}/test.mp4`;
    });
  });
});
