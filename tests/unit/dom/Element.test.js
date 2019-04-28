/* global afterEach, assert, beforeEach, describe, it */
const exokit = require('../../../src/index');

describe('Element', () => {
  var window;
  var document;
  var el;

  beforeEach(() => {
    const o = exokit.make();
    window = o.window;
    window.navigator.getVRDisplaysSync = () => [];
    document = o.document;
    el = document.createElement('a');
  });

  afterEach(() => {
    window.destroy();
  });

  describe('cloneNode', () => {
    it('clones node', () => {
      el.setAttribute('id', 'foo');
      const clone = el.cloneNode();
      assert.notEqual(clone, el);
      assert.equal(clone.getAttribute('id'), 'foo');
      assert.equal(clone.tagName, 'A');
      assert.ok(!clone.parentNode);
    });

    it('clones recursively', () => {
      // Create child.
      const child = document.createElement('p');
      child.setAttribute('id', 'bar');

      // Create grandchild.
      const child2 = document.createElement('span');
      child2.setAttribute('id', 'qux');

      // Append
      el.appendChild(child);
      child.appendChild(child2);
      el.setAttribute('id', 'foo');

      // Assert parent.
      let clone = el.cloneNode(true);
      assert.equal(clone.getAttribute('id'), 'foo');
      assert.equal(clone.childNodes.length, 1);

      // Assert child.
      assert.equal(clone.childNodes[0].getAttribute('id'), 'bar');
      assert.equal(clone.childNodes[0].childNodes.length, 1);
      assert.equal(clone.childNodes[0].tagName, 'P');
      assert.notEqual(clone.childNodes[0], child);

      // Assert grandchild.
      assert.equal(clone.childNodes[0].childNodes[0].getAttribute('id'), 'qux');
      assert.equal(clone.childNodes[0].childNodes[0].tagName, 'SPAN');
      assert.notEqual(clone.childNodes[0].childNodes[0], child2);
    });
  });

  describe('innerHTML', () => {
    it('serializes innerHTML', () => {
      el.setAttribute('id', 'foo');

      const child = document.createElement('p');
      child.dataset.bar = 'bar';
      el.appendChild(child);

      assert.equal(el.innerHTML, '<P data-bar="bar"></P>');
      assert.equal(child.innerHTML, '');
    });
  });

  describe('outerHTML', () => {
    it('serializes outerHTML', () => {
      el.setAttribute('id', 'foo');

      const child = document.createElement('p');
      child.dataset.bar = 'bar';
      el.appendChild(child);

      assert.ok(el.outerHTML.startsWith('<a id="foo">'));
      assert.ok(el.outerHTML.toLowerCase().indexOf(el.innerHTML.toLowerCase()) !== -1);
      assert.ok(el.outerHTML.endsWith('</a>'));
    });
  });
});
