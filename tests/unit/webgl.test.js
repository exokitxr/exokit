/* global afterEach, beforeEach, describe, assert, it */
const exokit = require('../../src/index');

describe('webgl', () => {
  var gl;
  var window;

  beforeEach(() => {
    window = exokit().window;
    gl = window.WebGLRenderingContext(window.document.createElement('canvas'));
  });

  afterEach(() => {
    window.destroy();
  });

  describe('getExtension', () => {
    it('enables EXT_blend_minmax', () => {
      const minmax = gl.getExtension('EXT_blend_minmax');
      assert.equal(typeof minmax.MIN_EXT, 'number');
      assert.equal(typeof minmax.MAX_EXT, 'number');
    });
  });
});
