/* global assert, beforeEach, describe, it */
const exokit = require('../../../index');

describe('ScriptElement', () => {
  var window;

  beforeEach(cb => {
    exokit.load(`${TEST_URL}/scripts.html`)
      .then(o => {
        window = o.window;
        cb();
      });
  });

  afterEach(() => {
    window.destroy();
  });

  describe('<script>', () => {
    it('runs in order', () => {
      assert.equal(window.lol1 instanceof window.Lol1, true);
      assert.equal(window.lol2 instanceof window.Lol2, true);
      assert.equal(window.lol3 instanceof window.Lol3, true);
      assert.equal(window.lol4 instanceof window.Lol4, true);
    });
  });
});
