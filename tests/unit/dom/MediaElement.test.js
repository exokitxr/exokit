/* global assert, beforeEach, describe */
const fs = require('fs');
const path = require('path');

const helpers = require('../helpers');
const exokit = require('../../../index');

const audioData = fs.readFileSync(path.resolve(__dirname, '../data/test.ogg'), 'base64');
const audioUri = `data:audio/ogg;base64,${audioData}`;

const imageData = fs.readFileSync(path.resolve(__dirname, '../data/test.png'), 'base64');
const imageUri = `data:image/img;base64,${imageData}`;

const videoData = fs.readFileSync(path.resolve(__dirname, '../data/test.mp4'), 'base64');
const videoUri = `data:video/mp4;base64,${videoData}`;

describe('MediaElement', () => {
  var document;
  var el;

  beforeEach(() => {
    document = exokit().document;
  });

  describe('<video>', () => {
    it('can set src', () => {
      el = document.createElement('video');
      el.setAttribute('src', videoUri);
    });
  });

  describe('<img>', () => {
    it('can set src', () => {
      el = document.createElement('img');
      el.setAttribute('src', imageUri);
    });
  });

  helpers.describeSkipCI('<audio>', () => {
    it('can set src', () => {
      el = document.createElement('audio');
      el.setAttribute('src', audioUri);
    });
  });
});
