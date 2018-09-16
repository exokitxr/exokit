/* global afterEach, beforeEach */
var chai = require('chai');
var express = require('express');
var path = require('path');
var Sinon = require('sinon');

const PORT = 10000;

var core = require('../../src/core');
core.setNativeBindingsModule(path.resolve(__dirname, '../../src/native-bindings.js'));

global.assert = chai.assert;
global.TEST_URL = `http://127.0.0.1:${PORT}`;

const server = express();
let serverListener;

beforeEach(function (done) {
  this.sinon = Sinon.createSandbox();
  server.use('/', express.static(__dirname + '/data'));
  serverListener = server.listen(PORT, done);
});

afterEach(function (done) {
  this.sinon.restore();
  serverListener.close(done);
});
