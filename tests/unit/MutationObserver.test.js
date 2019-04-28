/* global afterEach, assert, beforeEach, describe, it */
const exokit = require('../../src/index');

describe('MutationObserver', () => {
  var childEl;
  var el;
  var window;
  var document;
  var MutationObserver;

  beforeEach(done => {
    const o = exokit.make();
    window = o.window;
    window.navigator.getVRDisplaysSync = () => [];
    document = o.document;
    MutationObserver = window.MutationObserver;

    el = document.createElement('div');
    el.id = 'parent';
    el.setAttribute('foo', 'foo');
    childEl = document.createElement('p');
    childEl.id = 'child';

    setTimeout(() => { done(); });
  });

  afterEach(() => {
    window.destroy();
  });

  describe('attributes', () => {
    const observerHelper = cb => {
      const observer = new MutationObserver(cb);
      observer.observe(el, {attributes: true, childList: false, subtree: false});
      return observer;
    };

    it('calls back on attribute added', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 0);
        assert.equal(mutation.attributeName, 'bar');
        assert.equal(mutation.oldValue, undefined);
        assert.equal(mutation.target, el);
        assert.equal(mutation.nextSibling, null);
        assert.equal(mutation.removedNodes.length, 0);
        assert.equal(mutation.previousSibling, null);
        assert.equal(mutation.type , 'attributes');
        done();
      });
      el.setAttribute('bar', 'bar');
    });

    it('calls back on attribute changed', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 0);
        assert.equal(mutation.attributeName, 'foo');
        assert.equal(mutation.oldValue, 'foo');
        assert.equal(mutation.target, el);
        assert.equal(mutation.nextSibling, null);
        assert.equal(mutation.removedNodes.length, 0);
        assert.equal(mutation.previousSibling, null);
        assert.equal(mutation.type , 'attributes');
        done();
      });
      el.setAttribute('foo', 'foofoo');
    });

    it('calls back on attribute removed', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 0);
        assert.equal(mutation.attributeName, 'foo');
        assert.equal(mutation.oldValue, 'foo');
        assert.equal(mutation.target, el);
        assert.equal(mutation.nextSibling, null);
        assert.equal(mutation.removedNodes.length, 0);
        assert.equal(mutation.previousSibling, null);
        assert.equal(mutation.type , 'attributes');
        done();
      });
      el.removeAttribute('foo');
    });
  });

  describe('childList', () => {
    const observerHelper = cb => {
      const observer = new MutationObserver(cb);
      observer.observe(el, {attributes: false, childList: true, subtree: false});
      return observer;
    };

    it('calls back on child added', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 1);
        assert.equal(mutation.addedNodes[0], childEl);
        assert.equal(mutation.attributeName, null);
        assert.equal(mutation.oldValue, null);
        assert.equal(mutation.target, el);
        assert.equal(mutation.nextSibling, null);
        assert.equal(mutation.removedNodes.length, 0);
        assert.equal(mutation.previousSibling, null);
        assert.equal(mutation.type , 'childList');
        done();
      });
      el.appendChild(childEl);
    });

    it('calls back on child removed', done => {
      el.appendChild(childEl);
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 0);
        assert.equal(mutation.attributeName, null);
        assert.equal(mutation.oldValue, null);
        assert.equal(mutation.target, el);
        assert.equal(mutation.nextSibling, null);
        assert.equal(mutation.removedNodes.length, 1);
        assert.equal(mutation.removedNodes[0], childEl);
        assert.equal(mutation.previousSibling, null);
        assert.equal(mutation.type , 'childList');
        done();
      });
      el.removeChild(childEl);
    });

    it('calls back on child added via innerHTML', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutation.addedNodes.length, 1);
        assert.equal(mutation.addedNodes[0].id, 'childInnerHTML');
        assert.equal(mutation.removedNodes.length, 0);
        done();
      });
      el.innerHTML = '<p id="childInnerHTML"></p>';
    });

    it('does not call back on nested children added via innerHTML', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        const ids = mutation.addedNodes.map(node => node.getAttribute('id'));
        assert.ok(ids.includes('childInner1'), 'Child 1');
        assert.ok(ids.includes('childInner2'), 'Child 2');
        assert.notOk(ids.includes('childInner3'), 'Child 3');
        assert.equal(mutation.addedNodes.length, 2);
        assert.equal(mutation.removedNodes.length, 0);
        done();
      });
      el.innerHTML = '<p id="childInner1"></p><div id="childInner2"><p id="childInner3"></p></div>';
    });

    it('calls back on child added via document fragment', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutation.addedNodes.length, 1);
        assert.equal(mutation.addedNodes[0].id, 'child');
        assert.equal(mutation.removedNodes.length, 0);
        done();
      });
      const fragment = document.createDocumentFragment();
      fragment.appendChild(childEl);
      el.appendChild(fragment);
    });

    it('does not call back on multiple children added via document fragment', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        const ids = mutation.addedNodes.map(node => node.id);
        assert.ok(ids.includes('child1'), 'Child 1');
        assert.ok(ids.includes('child2'), 'Child 2');
        assert.notOk(ids.includes('child3'), 'Child 3');
        assert.equal(mutation.addedNodes.length, 2);
        done();
      });
      const fragment = document.createDocumentFragment();
      const child1 = document.createElement('p');
      child1.setAttribute('id', 'child1');
      fragment.appendChild(child1);
      const child2 = document.createElement('p');
      child2.setAttribute('id', 'child2');
      fragment.appendChild(child2);
      const child3 = document.createElement('p');
      child3.setAttribute('id', 'child3');
      child2.appendChild(child3);
      el.appendChild(fragment);
    });
  });

  describe('subtree', () => {
    const observerHelper = cb => {
      const observer = new MutationObserver(cb);
      observer.observe(el, {attributes: true, childList: true, subtree: true});
      return observer;
    };

    beforeEach(done => {
      el.appendChild(childEl);
      setTimeout(() => { done(); });
    });

    it('detects nested setAttribute', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 0);
        assert.equal(mutation.attributeName, 'qux');
        assert.equal(mutation.oldValue, undefined);
        assert.equal(mutation.target, childEl);
        assert.equal(mutation.nextSibling, null);
        assert.equal(mutation.removedNodes.length, 0);
        assert.equal(mutation.previousSibling, null);
        assert.equal(mutation.type , 'attributes');
        done();
      });
      childEl.setAttribute('qux', 'qux');
    });

    it('detects nested appends', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 1);
        assert.equal(mutation.attributeName, null);
        assert.equal(mutation.oldValue, null);
        assert.equal(mutation.target.tagName, 'P');
        assert.equal(mutation.nextSibling, null);
        assert.equal(mutation.removedNodes.length, 0);
        assert.equal(mutation.previousSibling, null);
        assert.equal(mutation.type , 'childList');
        done();
      });
      const grandchildEl = document.createElement('a');
      childEl.appendChild(grandchildEl);
    });

    it('detects nested removes', done => {
      const grandchildEl = document.createElement('a');
      childEl.appendChild(grandchildEl);

      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 0);
        assert.equal(mutation.attributeName, null);
        assert.equal(mutation.oldValue, null);
        assert.equal(mutation.target.tagName, 'P');
        assert.equal(mutation.nextSibling, null);
        assert.equal(mutation.removedNodes.length, 1);
        assert.equal(mutation.previousSibling, null);
        assert.equal(mutation.type , 'childList');
        done();
      });

      childEl.removeChild(grandchildEl);
    });
  });

  describe('characterData', () => {
    it('detects text node characterData change', done => {
      const textNode = document.createTextNode('');
      const observer = new MutationObserver(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.type , 'characterData');
        done();
      });
      observer.observe(textNode, {characterData: true});
      textNode.data = 'zol';
    });
  });

  describe('document observation', () => {
    it('can observe document', done => {
      document.body.appendChild(el);

      const observerHelper = cb => {
        const observer = new MutationObserver(cb);
        observer.observe(document, {attributes: false, childList: true, subtree: true});
        return observer;
      };

      observerHelper(mutations => {
        const mutation = mutations[0];
        assert.equal(mutations.length, 1);
        assert.equal(mutation.addedNodes.length, 1);
        assert.equal(mutation.addedNodes[0].id, 'child');
        assert.equal(mutation.type , 'childList');
        done();
      });

      setTimeout(() => { el.appendChild(childEl); });
    });

    it('does not mutate on non-attached elements when listening to document', done => {
      const observer = new MutationObserver(() => {
        assert.equal(1, 0, 'Should not have triggered');
      });
      observer.observe(document, {attributes: false, childList: true, subtree: true});

      // el is not part of document.
      el.appendChild(childEl);
      el.setAttribute('data-foo', 'foo');
      childEl.setAttribute('data-bar', 'bar');

      done();
    });
  });
});
