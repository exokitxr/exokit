/* global assert, describe, it */
const exokit = require('./src/index');

setTimeout(async () => {
  const window = exokit.make({
    require: true,
  });
  await window.evalAsync(`
    const path = require('path');
    const fs = require('fs');
    const assert = require('assert');
    
    console.log('TKTK test 1');
    document.body.innerHTML = fs.readFileSync(path.resolve(${JSON.stringify(__dirname)}, './tests/unit/data/dummy.html'), 'utf8');
    console.log('TKTK test 2');
    assert.ok(document);
    console.log('TKTK test 3');
    assert.ok(document.head);
    console.log('TKTK test 4');
    assert.ok(document.body);
    console.log('TKTK test 5');
    assert.equal(document.querySelector('a').getAttribute('href'), 'test.html');
    console.log('TKTK test 6');
  `);
  console.log('TKTK test 7');
  await window.destroy();
  console.log('TKTK test 8');
  await window.onexit();
  console.log('TKTK test 9');
  await window.onexit();
  console.log('TKTK test 10');

  exokit.exit();
  console.log('TKTK test 11');

}, 10);
