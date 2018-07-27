/* global assert, beforeEach, describe, it */
const exokit = require('../../../index');

describe('requestAnimationFrame', () => {
  var window;

  beforeEach(cb => {
    exokit.load('data:text/html,<html></html>')
      .then(o => {
        window = o.window;
        cb();
      });
  });

  afterEach(() => {
    window.destroy();
  });

  it('raf', cb => {
    let rafed = false;
    window.requestAnimationFrame(() => {
      rafed = true;

      cb();
    });

    assert.equal(rafed, false);
  });

  it('cancel raf', cb => {
    let rafed = false;
    const raf = window.requestAnimationFrame(() => {
      rafed = true;
    });

    assert.equal(rafed, false);

    window.cancelAnimationFrame(raf);

    setTimeout(() => {
      assert.equal(rafed, false);

      cb();
    }, 100);
  });
});
