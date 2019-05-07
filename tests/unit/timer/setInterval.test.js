/* global assert, beforeEach, describe, it */
const exokit = require('../../../src/index');

describe('setInterval', () => {
  var window;

  beforeEach(async () => {
    window = await exokit.load('data:text/html,<html></html>');
    
    return await window.evalAsync(`
      const assert = require('assert');
      window.assert = assert;
      1;
    `);
  });

  afterEach(async () => {
    await window.destroy();
    await exokit.exit();
  });

  it('interval 0', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let intervals = 0;
      const interval = window.setInterval(() => {
        if (++intervals === 3) {
          window.clearInterval(interval);

          accept();
        }
      }, 0);

      assert.equal(typeof interval, 'number');
      assert.equal(intervals, 0);
    })`);
  });

  it('interval 10', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let intervals = 0;
      const interval = window.setInterval(() => {
        if (++intervals === 3) {
          window.clearInterval(interval);

          accept();
        }
      }, 10);

      assert.equal(typeof interval, 'number');
      assert.equal(intervals, 0);
    })`);
  });

  it('clear interval', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let intervals = 0;
      const interval = window.setInterval(() => {
        intervals++;
      }, 0);

      assert.equal(typeof interval, 'number');
      assert.equal(intervals, 0);

      window.clearInterval(interval);

      setTimeout(() => {
        assert.equal(intervals, 0);

        accept();
      }, 100);
    })`);
  });
});
