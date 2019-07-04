/* global afterEach, beforeEach */
var chai = require('chai');
var express = require('express');
var path = require('path');

const PORT = 10000;

global.assert = chai.assert;
global.TEST_URL = `http://127.0.0.1:${PORT}`;

const server = express();
let serverListener;

beforeEach(function (done) {
  server.use('/', express.static(__dirname + '/data'));
  serverListener = server.listen(PORT, done);
});

afterEach(function (done) {
  serverListener.close(done);
});
