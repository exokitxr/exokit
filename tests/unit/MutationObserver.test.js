/* global assert, beforeEach, describe, it */
const helpers = require('./helpers');

describe('MutationObserver', () => {
  var childEl;
  var el;

  const window = helpers.createWindow();
  const document = window.document;
  const MutationObserver = window.MutationObserver;

  beforeEach(() => {
    el = document.createElement('div');
    el.id = 'parent';
    el.setAttribute('foo', 'foo');
    childEl = document.createElement('p');
    childEl.id = 'child';
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

    it('calls back on multiple children added via innerHTML', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        const ids = mutation.addedNodes.map(node => node.getAttribute('id'));
        assert.ok(ids.includes('childInner1'), 'Child 1');
        assert.ok(ids.includes('childInner2'), 'Child 2');
        assert.ok(ids.includes('childInner3'), 'Child 3');
        assert.equal(mutation.addedNodes.length, 3);
        assert.equal(mutation.removedNodes.length, 0);
        done();
      });
      el.innerHTML = '<p id="childInner1"></p><div id="childInner2"><p id="childInner3"></p> </div>';
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

    it('calls back on multiple children added via document fragment', done => {
      observerHelper(mutations => {
        const mutation = mutations[0];
        const ids = mutation.addedNodes.map(node => node.id);
        assert.ok(ids.includes('child1'), 'Child 1');
        assert.ok(ids.includes('child2'), 'Child 2');
        assert.ok(ids.includes('child3'), 'Child 3');
        assert.equal(mutation.addedNodes.length, 3);
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

    el.appendChild(childEl);
  });
});
