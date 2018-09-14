/* global afterEach, assert, beforeEach, describe, doesnotexist, it */
const exokit = require('../../../src/index');

describe('requestAnimationFrame', () => {
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

  it('catches errors', function (cb) {
    const spy = this.sinon.spy();
    function step () {
      spy();
      console.log(doesnotexist);
    }

    window.requestAnimationFrame(step);
    window.requestAnimationFrame(step);

    setTimeout(() => {
      assert.equal(spy.callCount, 2);
      cb();
    }, 100);
  });
});
