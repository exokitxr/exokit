/* global afterEach, assert, beforeEach, describe, it */
const exokit = require('../../src/index');

describe('MutationObserver', () => {
  var window;

  beforeEach(async () => {
    window = exokit({
      require: true,
    });

    return await window.evalAsync(`new Promise((accept, reject) => {
      const assert = require('assert');
      window.assert = assert;
    
      const el = document.createElement('div');
      el.id = 'parent';
      el.setAttribute('foo', 'foo');
      window.el = el;

      childEl = document.createElement('p');
      childEl.id = 'child';
      window.childEl = childEl;
      
      var observerHelper = cb => {
        const observer = new MutationObserver(cb);
        observer.observe(el, {attributes: true, childList: false, subtree: false});
        return observer;
      };
      window.observerHelper = observerHelper;

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
    return await window.destroy();
  });

  describe('attributes', () => {
    it('calls back on attribute added', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
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
          accept();
        });
        el.setAttribute('bar', 'bar');
      })`);
    });

    it('calls back on attribute changed', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
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
          accept();
        });
        el.setAttribute('foo', 'foofoo');
      })`);
    });

    it('calls back on attribute removed', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
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
          accept();
        });
        el.removeAttribute('foo');
      })`);
    });
  });

  describe('childList', () => {
    it('calls back on child added', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
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
          accept();
        });
        el.appendChild(childEl);
      })`);
    });

    it('calls back on child removed', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
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
          accept();
        });
        el.removeChild(childEl);
      })`);
    });

    it('calls back on child added via innerHTML', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        observerHelper(mutations => {
          const mutation = mutations[0];
          assert.equal(mutation.addedNodes.length, 1);
          assert.equal(mutation.addedNodes[0].id, 'childInnerHTML');
          assert.equal(mutation.removedNodes.length, 0);
          accept();
        });
        el.innerHTML = '<p id="childInnerHTML"></p>';
      })`);
    });

    it('does not call back on nested children added via innerHTML', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        observerHelper(mutations => {
          const mutation = mutations[0];
          const ids = mutation.addedNodes.map(node => node.getAttribute('id'));
          assert.ok(ids.includes('childInner1'), 'Child 1');
          assert.ok(ids.includes('childInner2'), 'Child 2');
          assert.ok(!ids.includes('childInner3'), 'Child 3');
          assert.equal(mutation.addedNodes.length, 2);
          assert.equal(mutation.removedNodes.length, 0);
          accept();
        });
        el.innerHTML = '<p id="childInner1"></p><div id="childInner2"><p id="childInner3"></p></div>';
      })`);
    });

    it('calls back on child added via document fragment', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        observerHelper(mutations => {
          const mutation = mutations[0];
          assert.equal(mutation.addedNodes.length, 1);
          assert.equal(mutation.addedNodes[0].id, 'child');
          assert.equal(mutation.removedNodes.length, 0);
          accept();
        });
        const fragment = document.createDocumentFragment();
        fragment.appendChild(childEl);
        el.appendChild(fragment);
      })`);
    });

    it('does not call back on multiple children added via document fragment', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        observerHelper(mutations => {
          const mutation = mutations[0];
          const ids = mutation.addedNodes.map(node => node.id);
          assert.ok(ids.includes('child1'), 'Child 1');
          assert.ok(ids.includes('child2'), 'Child 2');
          assert.ok(!ids.includes('child3'), 'Child 3');
          assert.equal(mutation.addedNodes.length, 2);
          accept();
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
      })`);
    });
  });

  describe('subtree', () => {
    beforeEach(async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        el.appendChild(childEl);
        
        var observerHelper = cb => {
          const observer = new MutationObserver(cb);
          observer.observe(el, {attributes: true, childList: true, subtree: true});
          return observer;
        };
        window.observerHelper = observerHelper;

        if (document.readyState !== 'complete') {
          window.onload = () => {
            accept();
          };
        } else {
          accept();
        }
      })`);
    });

    it('detects nested setAttribute', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
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
          accept();
        });
        childEl.setAttribute('qux', 'qux');
      })`);
    });

    it('detects nested appends', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
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
          accept();
        });
        const grandchildEl = document.createElement('a');
        childEl.appendChild(grandchildEl);
      })`);
    });

    it('detects nested removes', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
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
          accept();
        });

        childEl.removeChild(grandchildEl);
      })`);
    });
  });

  describe('characterData', () => {
    it('detects text node characterData change', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        const textNode = document.createTextNode('');
        const observer = new MutationObserver(mutations => {
          const mutation = mutations[0];
          assert.equal(mutations.length, 1);
          assert.equal(mutation.type , 'characterData');
          setTimeout(() => {
            accept();
          }, 10);
        });
        observer.observe(textNode, {characterData: true});
        textNode.data = 'zol';
      })`);
    });
  });

  describe('document observation', () => {
    it('can observe document', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        document.body.appendChild(el);

        observerHelper(mutations => {
          const mutation = mutations[0];
          assert.equal(mutations.length, 1);
          assert.equal(mutation.addedNodes.length, 1);
          assert.equal(mutation.addedNodes[0].id, 'child');
          assert.equal(mutation.type , 'childList');
          setTimeout(() => {
            accept();
          }, 10);
        });
        el.appendChild(childEl);
      })`);
    });

    it('does not mutate on non-attached elements when listening to document', async () => {
      return await window.evalAsync(`new Promise((accept, reject) => {
        const observer = new MutationObserver(() => {
          assert.equal(1, 0, 'Should not have triggered');
        });
        observer.observe(document, {attributes: false, childList: true, subtree: true});

        // el is not part of document.
        el.appendChild(childEl);
        el.setAttribute('data-foo', 'foo');
        childEl.setAttribute('data-bar', 'bar');
        
        setTimeout(() => {
          accept();
        }, 10);
      })`);
    });
  });
});
