/* global afterEach, assert, beforeEach, describe, it */
const exokit = require('../../../src/index');

describe('HTMLAnchorElement', () => {
  var window;

  beforeEach(async () => {
    window = exokit.make({
      require: true,
    });

    return await window.evalAsync(`
      const assert = require('assert');
      window.assert = assert;

      const el = document.createElement('a');
      window.el = el;
      1;
    `);
  });

  afterEach(async () => {
    return await window.destroy();
  });

  describe('a', () => {
    it('can set href', async () => {
      return await window.evalAsync(`
        el.href = 'https://bar.com';
        assert.equal(el.href, 'https://bar.com');
        el.href = '/bar/';
        assert.equal(el.href, '/bar/');
      `);
    });

    it('can get location properties', async () => {
      return await window.evalAsync(`
        window.location.toString = () => 'https://foo.com';
        el.href = 'https://bar.com:8080/corge?qux=1#qaz';
        assert.equal(el.hash, '#qaz');
        assert.equal(el.host, 'bar.com:8080');
        assert.equal(el.hostname, 'bar.com');
        assert.equal(el.password, '');
        assert.equal(el.pathname, '/corge');
        assert.equal(el.port, 8080);
        assert.equal(el.protocol, 'https:');
        assert.equal(el.origin, 'https://bar.com:8080');
        assert.equal(el.search, '?qux=1');
        assert.equal(el.username, '');
      `);
    });

    it('supports relative path on URL with path', async () => {
      return await window.evalAsync(`
        window.location.toString = () => 'https://foo.com/baz/';
        el.href = 'bar/';
        assert.equal(el.href, 'bar/');
        assert.equal(el.pathname, '/baz/bar/');
      `);
    });
  });
});
