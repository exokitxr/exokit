/* global afterEach, beforeEach, describe, assert, it */
const exokit = require('../../src/index');
const helpers = require('./helpers');

helpers.describeSkipCI('webgl', () => {
  var window;
  
  beforeEach(async () => {
    window = exokit.make({
      require: true,
    });
    
    return await window.evalAsync(`
      const assert = require('assert');
      window.assert = assert;
    
      const gl = window.WebGLRenderingContext(window.document.createElement('canvas'));
      window.gl = gl;
      1;
    `);
  });

  afterEach(async () => {
    return await window.destroy();
  });

  describe('getExtension', () => {
    it('returns EXT_blend_minmax', async () => {
      return await window.evalAsync(`
        ext = gl.getExtension('EXT_blend_minmax');
        assert.equal(typeof ext.MIN_EXT, 'number');
        assert.equal(typeof ext.MAX_EXT, 'number');
      `);
    });

    it('returns EXT_color_buffer_half_float', async () => {
      return await window.evalAsync(`
        ext = gl.getExtension('EXT_color_buffer_half_float');
        assert.equal(typeof ext.FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT, 'number');
        assert.equal(typeof ext.RGB16F_EXT, 'number');
        assert.equal(typeof ext.RGBA16F_EXT, 'number');
        assert.equal(typeof ext.UNSIGNED_NORMALIZED_EXT, 'number');
      `);
    });

    it('returns EXT_sRGB', async () => {
      return await window.evalAsync(`
        ext = gl.getExtension('EXT_sRGB');
        assert.equal(typeof ext.FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT, 'number');
        assert.equal(typeof ext.SRGB8_ALPHA8_EXT, 'number');
        assert.equal(typeof ext.SRGB_ALPHA_EXT, 'number');
        assert.equal(typeof ext.SRGB_EXT, 'number');
      `);
    });

    it('returns OES_vertex_array_object ', async () => {
      return await window.evalAsync(`
        ext = gl.getExtension('OES_vertex_array_object');
        const vao = ext.createVertexArrayOES();
        assert.ok(!ext.isVertexArrayOES(vao));
        ext.bindVertexArrayOES(vao);
        assert.ok(ext.isVertexArrayOES(vao));
        ext.deleteVertexArrayOES(vao);
      `);
    });
  });
});
