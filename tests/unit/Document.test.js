/* global assert, describe, it */
const exokit = require('../../src/index');

describe('_parseDocument', () => {
  it('parses basic document', async () => {
    const window = exokit({
      require: true,
    });
    await window.evalAsync(`
      const path = require('path');
      const fs = require('fs');
      const assert = require('assert');
      
      document.body.innerHTML = fs.readFileSync(path.resolve(${JSON.stringify(__dirname)}, './data/dummy.html'), 'utf8');
      assert.ok(document);
      assert.ok(document.head);
      assert.ok(document.body);
      assert.equal(document.querySelector('a').getAttribute('href'), 'test.html');
    `);
    return await window.destroy();
  });
});
