'use strict';

const path = require('path');
const bindings  = require(path.join(__dirname, '..', 'native-bidings'));
module.exports = bindings.nativeRtc;
