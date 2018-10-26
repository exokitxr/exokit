/* global describe */
const exokit = require('../../src/index');

/**
 * Test that is only run locally and is skipped on CI.
 */
module.exports.describeSkipCI = process.env.TEST_ENV === 'ci'
  ? describe.skip
  : describe;
