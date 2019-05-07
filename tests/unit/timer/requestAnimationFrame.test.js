/* global afterEach, assert, beforeEach, describe, doesnotexist, it */
const exokit = require('../../../src/index');

async function sleep(ms) {
  return new Promise((accept, reject) => {
    setTimeout(() => {
      accept();
    }, ms);
  });
}

describe('requestAnimationFrame', () => {
  var window;

  beforeEach(async () => {
    window = await exokit.load('data:text/html,<html></html>');
    
    return await window.evalAsync(`
      const assert = require('assert');
      window.assert = assert;
      const sinon = require('sinon');
      window.sinon = sinon;
      window.callCount = 0;
      1;
    `);
  });

  afterEach(async () => {
    console.log('sleep');
    await sleep(400);
    console.log('window.destroy');
    try {
      await window.destroy();
    } catch (err) {
      console.log(err.stack);
    }
    console.log('exokit.exit');
    await exokit.exit();
    console.log('requestAnimationFrame test DONE');
  });

  it('raf', async () => {
    return await window.evalAsync(`new Promise((accept, reject) => {
      let rafed = false;
      window.requestAnimationFrame(() => {
        rafed = true;
        
        accept();
      });
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
      //const spy = sinon.spy();
      function step () {
        console.log('step TKTK');
        window.callCount = window.callCount + 1;
        //spy();
        //let doesnotexist = null;
        //console.log(doesnotexist());
      }

      setTimeout(() => {
        assert.equal(window.callCount, 2);

        accept();
      }, 100);

      setTimeout(() => {
        window.requestAnimationFrame(() => {
          console.log('step 1a');
          step();
          console.log('step 1b');
          setTimeout(() => {
            window.requestAnimationFrame(() => {
              console.log('step 2a');
              step();
              console.log('step 2b');
            });
          }, 10);
        });
      }, 10);
    })`);
  });
});
