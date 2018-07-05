/* global assert, describe, it */
const fs = require('fs');
const path = require('path');
const helpers = require('./helpers');

let testBuffer = fs.readFileSync(path.resolve(__dirname, './data/test.ogg'));
testBuffer = new Uint8Array(testBuffer).buffer;

describe('audio', () => {
  var window;

  it('creates audio context', done => {
    window = helpers.createWindow();
    let context = new window.AudioContext();
    context.decodeAudioData(testBuffer, audioBuffer => {
      assert.ok(audioBuffer);
      done();
    });
  });
});
