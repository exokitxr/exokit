/* global assert, beforeEach, describe, it, TEST_URL */
const fs = require('fs');
const path = require('path');

const helpers = require('../helpers');
const exokit = require('../../../index');

const imageData = fs.readFileSync(path.resolve(__dirname, '../data/test.png'), 'base64');
const imageUri = `data:image/img;base64,${imageData}`;

const audioData = fs.readFileSync(path.resolve(__dirname, '../data/test.ogg'), 'base64');
const audioUri = `data:audio/ogg;base64,${audioData}`;

const videoData = fs.readFileSync(path.resolve(__dirname, '../data/test.mp4'), 'base64');
const videoUri = `data:video/mp4;base64,${videoData}`;

describe('HTMLSrcableElement', () => {
  var document;
  var el;

  beforeEach(() => {
    document = exokit().document;
  });

  describe('<img>', () => {
    it('can set src', done => {
      el = document.createElement('img');
      el.onload = () => { done(); };
      el.setAttribute('src', `${TEST_URL}/test.png`);
    });
  });

  helpers.describeSkipCI('<audio>', () => {
    it('can set src', done => {
      el = document.createElement('audio');
      el.oncanplay = () => { done(); };
      el.setAttribute('src', `${TEST_URL}/test.ogg`);
    });
  });

  describe('<video>', () => {
    it('can set src', done => {
      el = document.createElement('video');
      el.oncanplay = () => { done(); };
      el.setAttribute('src', `${TEST_URL}/test.mp4`);
    });
  });
});
