/* global afterEach, assert, beforeEach, describe, it */
const exokit = require('../../../src/index');
const _normalizePrototype = require('../../../src/utils')._normalizePrototype;

describe('utils._normalizeProtoype', () => {
  var window;

  beforeEach(() => {
    const o = exokit.make();
    window = o.window;
  });

  afterEach(() => {
    window.destroy();
  });

  it('handles odd inputs', () => {
    assert.equal(_normalizePrototype(null, global), null);
    assert.equal(_normalizePrototype(null, window), null);

    const obj = {};
    assert.equal(_normalizePrototype(obj, global), obj);
    assert.equal(_normalizePrototype(obj, window), obj);
  });

  it('normalizes', () => {
    const promise = new Promise(() => {});
    assert.ok(_normalizePrototype(promise, window) instanceof window.Promise);

    assert.ok(_normalizePrototype(new window.Promise(() => {}), global) instanceof
              Promise);
  });

  it('handles original prototype replaced', () => {
    window.Promise = {};
    const promise = new Promise(() => {});
    assert.equal(_normalizePrototype(promise, window).constructor.name, 'Promise');
  });
});
