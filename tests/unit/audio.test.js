/* global afterEach, beforeEach, assert, it */
const fs = require('fs');
const path = require('path');
const exokit = require('../../src/index');
const helpers = require('./helpers');

let testBuffer = fs.readFileSync(path.resolve(__dirname, './data/test.ogg'));
testBuffer = new Uint8Array(testBuffer).buffer;

helpers.describeSkipCI('audio', () => {
  var window;

  beforeEach(() => {
    const o = exokit.make();
    window = o.window;

    window.navigator.getVRDisplaysSync = () => [];
  });

  afterEach(() => {
    window.destroy();
  });

  it('creates audio context', done => {
    let context = new window.AudioContext();
    context.decodeAudioData(testBuffer, audioBuffer => {
      assert.ok(audioBuffer);
      done();
    });
  });

  it('handles invalid audio context data', done => {
    let context = new window.AudioContext();
    context.decodeAudioData('foo').catch(() => {
      done();
    });
  });


  it('catches user callback error', done => {
    let context = new window.AudioContext();
    context.decodeAudioData(testBuffer, () => {
      done();
      throw new Error();
    });
  });
});
