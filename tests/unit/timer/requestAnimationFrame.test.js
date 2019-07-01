/* global afterEach, assert, beforeEach, describe, doesnotexist, it */
const exokit = require('../../../src/index');

describe('requestAnimationFrame', () => {
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
  });

  it('raf', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let rafed = false;
      window.requestAnimationFrame(() => {
        rafed = true;
        
        accept();
      });

      assert.equal(rafed, false);
    })`);
  });

  it('cancel raf', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let rafed = false;
      const raf = window.requestAnimationFrame(() => {
        rafed = true;
      });

      assert.equal(rafed, false);

      window.cancelAnimationFrame(raf);

      setTimeout(() => {
        assert.equal(rafed, false);

        accept();
      }, 100);
    })`);
  });

  it('catches errors', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let callCount = 0;
      function step () {
        callCount++;
        console.log(doesnotexist);
      }

      window.requestAnimationFrame(step);
      window.requestAnimationFrame(step);

      setTimeout(() => {
        assert.equal(callCount, 2);

        accept();
      }, 100);
    })`);
  });
});
