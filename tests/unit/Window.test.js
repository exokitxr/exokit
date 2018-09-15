/* global assert, describe, it */
const exokit = require('../../src/index');

describe('Promise', () => {
  it('cannot be overwritten', () => {
    const window = exokit().window;
    const originalPromise = window.Promise;
    window.Promise = () => {};
    assert.equal(window.Promise, originalPromise);
  });
});
