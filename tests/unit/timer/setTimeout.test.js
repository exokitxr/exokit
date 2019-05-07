/* global assert, beforeEach, describe, it */
const exokit = require('../../../src/index');

describe('setTimeout', () => {
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

  it('timeout 0', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let timedout = false;
      window.setTimeout(() => {
        timedout = true;

        accept();
      }, 0);

      assert.equal(timedout, false);
    })`);
  });

  it('timeout 10', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let timedout = false;
      window.setTimeout(() => {
        timedout = true;

        accept();
      }, 10);

      assert.equal(timedout, false);
    })`);
  });

  it('clear timeout', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let timedout = false;
      const timeout = window.setTimeout(() => {
        timedout = true;
      }, 0);

      assert.equal(typeof timeout, 'number');
      assert.equal(timedout, false);

      window.clearTimeout(timeout);

      setTimeout(() => {
        assert.equal(timedout, false);

        accept();
      }, 100);
    })`);
  });
});
