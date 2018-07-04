var chai = require('chai');
var path = require('path');
var Sinon = require('sinon');

var core = require('../../core');
core.setNativeBindingsModule(path.resolve('native-bindings.js'));

global.assert = chai.assert;

beforeEach(function () {
  this.sinon = Sinon.createSandbox();
});

afterEach(function () {
  this.sinon.restore();
});
