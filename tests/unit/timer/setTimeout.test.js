/* global assert, beforeEach, describe, it */
const exokit = require('../../../index');

describe('setTimeout', () => {
  var window;

  beforeEach(() => {
    window = exokit().window;
  });

  it('timeout 0', cb => {
    let timedout = false;
    window.setTimeout(() => {
      timedout = true;

      cb();
    }, 0);

    assert.equal(timedout, false);
  });

  it('timeout 10', cb => {
    let timedout = false;
    window.setTimeout(() => {
      timedout = true;

      cb();
    }, 10);

    assert.equal(timedout, false);
  });

  it('clear timeout', cb => {
    let timedout = false;
    const timeout = window.setTimeout(() => {
      timedout = true;
    }, 0);

    assert.equal(typeof timedout, 'number');
    assert.equal(timedout, false);

    window.clearTimeout(timeout);

    setTimeout(() => {
      assert.equal(timedout, false);

      cb();
    }, 100);
  });
});
