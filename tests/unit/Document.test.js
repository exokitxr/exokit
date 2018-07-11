/* global assert, describe, it */
const fs = require('fs');
const path = require('path');
const {Response} = require('window-fetch');

const GlobalContext = require('../../src/GlobalContext');

const dummyHtml = fs.readFileSync(path.resolve(__dirname, './data/dummy.html'), 'utf-8');

describe('_parseDocument', () => {
  it('parses basic document', () => {
    const window = GlobalContext._makeWindow({dataPath: '', url: 'https://test.com'});
    // Stub fetch for script tag.
    window.fetch = () => Promise.resolve(new Response(''));
    const document = GlobalContext._parseDocument(dummyHtml, window);
    assert.ok(document);
    assert.ok(document.head);
    assert.ok(document.body);
    assert.equal(document.querySelector('a').getAttribute('href'), 'test.html');
  });
});
