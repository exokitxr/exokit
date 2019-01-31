'use strict';

const path = require('path');
const bindings  = require(path.join(__dirname, '..', 'native-bindings'));
module.exports = bindings.nativeRtc;
