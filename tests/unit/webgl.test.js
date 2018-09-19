/* global afterEach, beforeEach, describe, assert, it */
const exokit = require('../../src/index');
const helpers = require('./helpers');

helpers.describeSkipCI('webgl', () => {
  var ext;
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
    it('returns EXT_blend_minmax', () => {
      ext = gl.getExtension('EXT_blend_minmax');
      assert.equal(typeof ext.MIN_EXT, 'number');
      assert.equal(typeof ext.MAX_EXT, 'number');
    });

    it('returns EXT_color_buffer_half_float', () => {
      ext = gl.getExtension('EXT_color_buffer_half_float');
      assert.equal(typeof ext.FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT, 'number');
      assert.equal(typeof ext.RGB16F_EXT, 'number');
      assert.equal(typeof ext.RGBA16F_EXT, 'number');
      assert.equal(typeof ext.UNSIGNED_NORMALIZED_EXT, 'number');
    });

    it('returns EXT_sRGB', () => {
      ext = gl.getExtension('EXT_sRGB');
      assert.equal(typeof ext.FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT, 'number');
      assert.equal(typeof ext.SRGB8_ALPHA8_EXT, 'number');
      assert.equal(typeof ext.SRGB_ALPHA_EXT, 'number');
      assert.equal(typeof ext.SRGB_EXT, 'number');
    });

    it('returns OES_vertex_array_object ', () => {
      ext = gl.getExtension('OES_vertex_array_object');
      const vao = ext.createVertexArrayOES();
      assert.ok(!ext.isVertexArrayOES(vao));
      ext.bindVertexArrayOES(vao);
      assert.ok(ext.isVertexArrayOES(vao));
      ext.deleteVertexArrayOES(vao);
    });
  });
});
