/* global afterEach, assert, beforeEach, describe, it */
const exokit = require('../../../src/index');

describe('HTMLAnchorElement', () => {
  var window;
  var document;
  var el;

  beforeEach(function () {
    window = exokit.make().window;
    document = window.document;
    el = document.createElement('a');
  });

  afterEach(() => {
    window.destroy();
  });

  describe('a', () => {
    it('can set href', () => {
      el.href = 'https://bar.com';
      assert.equal(el.href, 'https://bar.com');
      el.href = '/bar/';
      assert.equal(el.href, '/bar/');
    });

    it('can get location propert:ies', function () {
      this.sinon.stub(window.location, 'toString').callsFake(() => 'https://foo.com');
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
    });

    it('supports relative path on URL with path', function () {
      this.sinon.stub(window.location, 'toString').callsFake(() => 'https://foo.com/baz/');
      el.href = 'bar/';
      assert.equal(el.href, 'bar/');
      assert.equal(el.host, 'foo.com');
      assert.equal(el.hostname, 'foo.com');
      assert.equal(el.pathname, '/baz/bar/');
      assert.equal(el.protocol, 'https:');
      assert.equal(el.origin, 'https://foo.com');
    });
  });
});
