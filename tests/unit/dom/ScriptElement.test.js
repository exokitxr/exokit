/* global assert, beforeEach, describe, it */
const exokit = require('../../../src/index');

describe('ScriptElement', () => {
  var window;

  beforeEach(async () => {
    window = await exokit.load(`${TEST_URL}/scripts.html`);
    
    return await window.evalAsync(`new Promise((accept, reject) => {
      const assert = require('assert');
      window.assert = assert;
      
      if (document.readyState !== 'complete') {
        window.onload = () => {
          accept();
        };
      } else {
        accept();
      }
    })`);
  });

  afterEach(async () => {
    return window.destroy();
  });

  describe('<script>', () => {
    it('HTML tags', async () => {
      return await window.evalAsync(`
        assert.equal(window.lol1 instanceof window.Lol1, true);
        assert.equal(window.lol2 instanceof window.Lol2, true);
        assert.equal(window.lol3 instanceof window.Lol3, true);
        assert.equal(window.lol4 instanceof window.Lol4, true);
      `);
    });

    it('Insert <script async=false>', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        let count = 0;
        let err = null;
        window.check = i => {
          if (i !== count) {
            err = err || new Error('failed check ' + i + ' ' + count);
          }

          count++;

          if (count === 2) {
            if (!err) {
              accept();
            } else {
              reject(err);
            }
          }
        };

        {
          const script = document.createElement('script');
          script.async = false;
          script.src = 'lol-sync0.js';
          document.body.appendChild(script);
        }

        {
          const script = document.createElement('script');
          script.async = false;
          script.src = 'lol-sync1.js';
          document.body.appendChild(script);
        }

        {
          const script = document.createElement('script');
          script.async = false;
          script.src = 'lol-sync2.js';
          document.body.appendChild(script);
        }

        1;
      })`);
    });
  });
});
