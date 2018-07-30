/* global assert, beforeEach, describe, it */
const exokit = require('../../../index');

describe('ScriptElement', () => {
  var window;
  var document;

  beforeEach(cb => {
    exokit.load(`${TEST_URL}/scripts.html`)
      .then(o => {
        window = o.window;
        document = o.document;

        window.onload = () => {
          cb();
        };
      });
  });

  afterEach(() => {
    window.destroy();
  });

  describe('<script>', () => {
    it('HTML tags', () => {
      assert.equal(window.lol1 instanceof window.Lol1, true);
      assert.equal(window.lol2 instanceof window.Lol2, true);
      assert.equal(window.lol3 instanceof window.Lol3, true);
      assert.equal(window.lol4 instanceof window.Lol4, true);
    });
    
    it('Insert <script async=false>', cb => {
      let count = 0;
      let err = null;
      window.check = i => {
        if (i !== count) {
          err = err || new Error('failed check ' + i + ' ' + count);
        }
        
        count++;
        
        if (count === 2) {
          cb(err);
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
    });
  });
});
