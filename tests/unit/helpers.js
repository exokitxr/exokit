/* global describe, suite */
const exokit = require('../../index');

module.exports.createWindow = function () {
  return exokit().window;
};

/**
 * Test that is only run locally and is skipped on CI.
 */
module.exports.describeSkipCI = process.env.TEST_ENV === 'ci'
  ? describe.skip
  : describe;
