/* global afterEach, beforeEach, assert, it */
const exokit = require('../../src/index');
const helpers = require('./helpers');

helpers.describeSkipCI('audio', () => {
  var window;

  beforeEach(async () => {
    window = exokit.make({
      require: true,
    });

    return await window.evalAsync(`
      window.assert = require('assert');
    
      const path = require('path'); 
      const fs = require('fs');
      let testBuffer = fs.readFileSync(path.resolve(${JSON.stringify(__dirname)}, './data/test.ogg'));
      testBuffer = new Uint8Array(testBuffer).buffer;
      window.testBuffer = testBuffer;
    `);
  });

  afterEach(async () => {
    return await window.destroy();
  });

  it('creates audio context', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let context = new window.AudioContext();
      context.decodeAudioData(testBuffer, audioBuffer => {
        assert.ok(audioBuffer);
        accept();
      });
    })`);
  });

  it('handles invalid audio context data', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let context = new window.AudioContext();
      context.decodeAudioData('foo').then(() => {
        reject(new Error('invalid audio data succeeded'));
      }).catch(() => {
        accept();
      });
    })`);
  });


  it('catches user callback error', () => {
    return window.evalAsync(`new Promise((accept, reject) => {
      let context = new window.AudioContext();
      context.decodeAudioData(testBuffer, () => {
        accept();
        throw new Error();
      });
    })`);
  });
});
