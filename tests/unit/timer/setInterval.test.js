/* global assert, beforeEach, describe, it */
const exokit = require('../../../index');

describe('setInterval', () => {
  var window;

  beforeEach(cb => {
    exokit.load('data:text/html,<html></html>')
      .then(o => {
        window = o.window;
        window.navigator.getVRDisplaysSync = () => [];
        cb();
      });
  });

  afterEach(() => {
    window.destroy();
  });

  it('interval 0', cb => {
    let intervals = 0;
    const interval = window.setInterval(() => {
      if (++intervals === 3) {
        window.clearInterval(interval);

        cb();
      }
    }, 0);

    assert.equal(typeof interval, 'number');
    assert.equal(intervals, 0);
  });

  it('interval 10', cb => {
    let intervals = 0;
    const interval = window.setInterval(() => {
      if (++intervals === 3) {
        window.clearInterval(interval);

        cb();
      }
    }, 10);

    assert.equal(typeof interval, 'number');
    assert.equal(intervals, 0);
  });

  it('clear interval', cb => {
    let intervals = 0;
    const interval = window.setInterval(() => {
      intervals++;
    }, 0);

    assert.equal(typeof interval, 'number');
    assert.equal(intervals, 0);

    window.clearInterval(interval);

    setTimeout(() => {
      assert.equal(intervals, 0);

      cb();
    }, 100);
  });
});
