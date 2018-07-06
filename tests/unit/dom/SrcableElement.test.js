/* global assert, beforeEach, describe, it, TEST_URL */
const fs = require('fs');
const path = require('path');

const helpers = require('../helpers');
const exokit = require('../../../index');

const imageData = fs.readFileSync(path.resolve(__dirname, '../data/test.png'), 'base64');
const imageDataUri = `data:image/img;base64,${imageData}`;

const audioData = fs.readFileSync(path.resolve(__dirname, '../data/test.ogg'), 'base64');
const audioDataUri = `data:audio/ogg;base64,${audioData}`;

const videoData = fs.readFileSync(path.resolve(__dirname, '../data/test.mp4'), 'base64');
const videoDataUri = `data:video/mp4;base64,${videoData}`;

describe('HTMLSrcableElement', () => {
  var document;
  var el;

  beforeEach(() => {
    document = exokit().document;
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

    it('works with data url', done => {
      el = document.createElement('img');
      el.onload = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = imageDataUri;
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

    it('works with data url', done => {
      el = document.createElement('audio');
      el.oncanplay = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = audioDataUri;
    });
  });

  describe('<video> setAttribute', () => {
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

    it('works with data url', done => {
      el = document.createElement('video');
      el.oncanplay = () => { done(); };
      el.onerror = err => { done(err); };
      el.src = videoDataUri;
    });;
  });
});
