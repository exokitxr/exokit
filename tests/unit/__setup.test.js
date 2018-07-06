var chai = require('chai');
var express = require('express');
var path = require('path');
var Sinon = require('sinon');

var core = require('../../core');
core.setNativeBindingsModule(path.resolve('native-bindings.js'));

global.assert = chai.assert;
global.TEST_URL = 'http://localhost:3000';

const server = express();
let serverListener;

beforeEach(function (done) {
  this.sinon = Sinon.createSandbox();
  server.use('/', express.static(__dirname + '/data'));
  serverListener = server.listen(3000, done);
});

afterEach(function (done) {
  this.sinon.restore();
  serverListener.close(done);
});
