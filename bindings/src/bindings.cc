/*
 * bindings.cc
 *
 *  Created on: Dec 13, 2011
 *      Author: ngk437
 */

#include "bindings.h"

// Used to be a macro, hence the uppercase name.
#define JS_GL_SET_CONSTANT(name, constant) proto->Set(JS_STR( name ), JS_INT(constant))

//#define JS_GL_SET_CONSTANT(name, value) NODE_DEFINE_CONSTANTNODE_DEFINE_CONSTANT

#define JS_GL_CONSTANT(name) JS_GL_SET_CONSTANT(#name, GL_ ## name)

Handle<Object> WebGLContext::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("WebGLContext"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto, "getWindowHandle", GetWindowHandle);
  Nan::SetMethod(proto, "setWindowHandle", SetWindowHandle);

  Nan::SetMethod(proto, "uniform1f", setContextWrap<webgl::Uniform1f>);
  Nan::SetMethod(proto, "uniform2f", setContextWrap<webgl::Uniform2f>);
  Nan::SetMethod(proto, "uniform3f", setContextWrap<webgl::Uniform3f>);
  Nan::SetMethod(proto, "uniform4f", setContextWrap<webgl::Uniform4f>);
  Nan::SetMethod(proto, "uniform1i", setContextWrap<webgl::Uniform1i>);
  Nan::SetMethod(proto, "uniform2i", setContextWrap<webgl::Uniform2i>);
  Nan::SetMethod(proto, "uniform3i", setContextWrap<webgl::Uniform3i>);
  Nan::SetMethod(proto, "uniform4i", setContextWrap<webgl::Uniform4i>);
  Nan::SetMethod(proto, "uniform1fv", setContextWrap<webgl::Uniform1fv>);
  Nan::SetMethod(proto, "uniform2fv", setContextWrap<webgl::Uniform2fv>);
  Nan::SetMethod(proto, "uniform3fv", setContextWrap<webgl::Uniform3fv>);
  Nan::SetMethod(proto, "uniform4fv", setContextWrap<webgl::Uniform4fv>);
  Nan::SetMethod(proto, "uniform1iv", setContextWrap<webgl::Uniform1iv>);
  Nan::SetMethod(proto, "uniform2iv", setContextWrap<webgl::Uniform2iv>);
  Nan::SetMethod(proto, "uniform3iv", setContextWrap<webgl::Uniform3iv>);
  Nan::SetMethod(proto, "uniform4iv", setContextWrap<webgl::Uniform4iv>);
  Nan::SetMethod(proto, "pixelStorei", setContextWrap<webgl::PixelStorei>);
  Nan::SetMethod(proto, "bindAttribLocation", setContextWrap<webgl::BindAttribLocation>);
  Nan::SetMethod(proto, "getError", setContextWrap<webgl::GetError>);
  Nan::SetMethod(proto, "drawArrays", setContextWrap<webgl::DrawArrays>);
  Nan::SetMethod(proto, "drawArraysInstancedANGLE", setContextWrap<webgl::DrawArraysInstancedANGLE>);
  Nan::SetMethod(proto, "uniformMatrix2fv", setContextWrap<webgl::UniformMatrix2fv>);
  Nan::SetMethod(proto, "uniformMatrix3fv", setContextWrap<webgl::UniformMatrix3fv>);
  Nan::SetMethod(proto, "uniformMatrix4fv", setContextWrap<webgl::UniformMatrix4fv>);

  Nan::SetMethod(proto, "generateMipmap", setContextWrap<webgl::GenerateMipmap>);

  Nan::SetMethod(proto, "getAttribLocation", setContextWrap<webgl::GetAttribLocation>);
  Nan::SetMethod(proto, "depthFunc", setContextWrap<webgl::DepthFunc>);
  Nan::SetMethod(proto, "viewport", setContextWrap<webgl::Viewport>);
  Nan::SetMethod(proto, "createShader", setContextWrap<webgl::CreateShader>);
  Nan::SetMethod(proto, "shaderSource", setContextWrap<webgl::ShaderSource>);
  Nan::SetMethod(proto, "compileShader", setContextWrap<webgl::CompileShader>);
  Nan::SetMethod(proto, "getShaderParameter", setContextWrap<webgl::GetShaderParameter>);
  Nan::SetMethod(proto, "getShaderInfoLog", setContextWrap<webgl::GetShaderInfoLog>);
  Nan::SetMethod(proto, "createProgram", setContextWrap<webgl::CreateProgram>);
  Nan::SetMethod(proto, "attachShader", setContextWrap<webgl::AttachShader>);
  Nan::SetMethod(proto, "linkProgram", setContextWrap<webgl::LinkProgram>);
  Nan::SetMethod(proto, "getProgramParameter", setContextWrap<webgl::GetProgramParameter>);
  Nan::SetMethod(proto, "getUniformLocation", setContextWrap<webgl::GetUniformLocation>);
  Nan::SetMethod(proto, "clearColor", setContextWrap<webgl::ClearColor>);
  Nan::SetMethod(proto, "clearDepth", setContextWrap<webgl::ClearDepth>);

  Nan::SetMethod(proto, "disable", setContextWrap<webgl::Disable>);
  Nan::SetMethod(proto, "createTexture", setContextWrap<webgl::CreateTexture>);
  Nan::SetMethod(proto, "bindTexture", setContextWrap<webgl::BindTexture>);
  // Nan::SetMethod(proto, "flipTextureData", setContextWrap<webgl::FlipTextureData>);
  Nan::SetMethod(proto, "texImage2D", setContextWrap<webgl::TexImage2D>);
  Nan::SetMethod(proto, "texParameteri", setContextWrap<webgl::TexParameteri>);
  Nan::SetMethod(proto, "texParameterf", setContextWrap<webgl::TexParameterf>);
  Nan::SetMethod(proto, "clear", setContextWrap<webgl::Clear>);
  Nan::SetMethod(proto, "useProgram", setContextWrap<webgl::UseProgram>);
  Nan::SetMethod(proto, "createFramebuffer", setContextWrap<webgl::CreateFramebuffer>);
  Nan::SetMethod(proto, "bindFramebuffer", setContextWrap<webgl::BindFramebuffer>);
  Nan::SetMethod(proto, "framebufferTexture2D", setContextWrap<webgl::FramebufferTexture2D>);
  Nan::SetMethod(proto, "createBuffer", setContextWrap<webgl::CreateBuffer>);
  Nan::SetMethod(proto, "bindBuffer", setContextWrap<webgl::BindBuffer>);
  Nan::SetMethod(proto, "bufferData", setContextWrap<webgl::BufferData>);
  Nan::SetMethod(proto, "bufferSubData", setContextWrap<webgl::BufferSubData>);
  Nan::SetMethod(proto, "enable", setContextWrap<webgl::Enable>);
  Nan::SetMethod(proto, "blendEquation", setContextWrap<webgl::BlendEquation>);
  Nan::SetMethod(proto, "blendFunc", setContextWrap<webgl::BlendFunc>);
  Nan::SetMethod(proto, "enableVertexAttribArray", setContextWrap<webgl::EnableVertexAttribArray>);
  Nan::SetMethod(proto, "vertexAttribPointer", setContextWrap<webgl::VertexAttribPointer>);
  Nan::SetMethod(proto, "activeTexture", setContextWrap<webgl::ActiveTexture>);
  Nan::SetMethod(proto, "drawElements", setContextWrap<webgl::DrawElements>);
  Nan::SetMethod(proto, "drawElementsInstancedANGLE", setContextWrap<webgl::DrawElementsInstancedANGLE>);
  Nan::SetMethod(proto, "flush", setContextWrap<webgl::Flush>);
  Nan::SetMethod(proto, "finish", setContextWrap<webgl::Finish>);

  Nan::SetMethod(proto, "vertexAttrib1f", setContextWrap<webgl::VertexAttrib1f>);
  Nan::SetMethod(proto, "vertexAttrib2f", setContextWrap<webgl::VertexAttrib2f>);
  Nan::SetMethod(proto, "vertexAttrib3f", setContextWrap<webgl::VertexAttrib3f>);
  Nan::SetMethod(proto, "vertexAttrib4f", setContextWrap<webgl::VertexAttrib4f>);
  Nan::SetMethod(proto, "vertexAttrib1fv", setContextWrap<webgl::VertexAttrib1fv>);
  Nan::SetMethod(proto, "vertexAttrib2fv", setContextWrap<webgl::VertexAttrib2fv>);
  Nan::SetMethod(proto, "vertexAttrib3fv", setContextWrap<webgl::VertexAttrib3fv>);
  Nan::SetMethod(proto, "vertexAttrib4fv", setContextWrap<webgl::VertexAttrib4fv>);
  Nan::SetMethod(proto, "vertexAttribDivisorANGLE", setContextWrap<webgl::VertexAttribDivisorANGLE>);

  Nan::SetMethod(proto, "blendColor", webgl::BlendColor);
  Nan::SetMethod(proto, "blendEquationSeparate", webgl::BlendEquationSeparate);
  Nan::SetMethod(proto, "blendFuncSeparate", webgl::BlendFuncSeparate);
  Nan::SetMethod(proto, "clearStencil", webgl::ClearStencil);
  Nan::SetMethod(proto, "colorMask", webgl::ColorMask);
  Nan::SetMethod(proto, "copyTexImage2D", webgl::CopyTexImage2D);
  Nan::SetMethod(proto, "copyTexSubImage2D", webgl::CopyTexSubImage2D);
  Nan::SetMethod(proto, "cullFace", webgl::CullFace);
  Nan::SetMethod(proto, "depthMask", webgl::DepthMask);
  Nan::SetMethod(proto, "depthRange", webgl::DepthRange);
  Nan::SetMethod(proto, "disableVertexAttribArray", webgl::DisableVertexAttribArray);
  Nan::SetMethod(proto, "hint", webgl::Hint);
  Nan::SetMethod(proto, "isEnabled", webgl::IsEnabled);
  Nan::SetMethod(proto, "lineWidth", webgl::LineWidth);
  Nan::SetMethod(proto, "polygonOffset", webgl::PolygonOffset);

  Nan::SetMethod(proto, "scissor", webgl::Scissor);
  Nan::SetMethod(proto, "stencilFunc", webgl::StencilFunc);
  Nan::SetMethod(proto, "stencilFuncSeparate", webgl::StencilFuncSeparate);
  Nan::SetMethod(proto, "stencilMask", webgl::StencilMask);
  Nan::SetMethod(proto, "stencilMaskSeparate", webgl::StencilMaskSeparate);
  Nan::SetMethod(proto, "stencilOp", webgl::StencilOp);
  Nan::SetMethod(proto, "stencilOpSeparate", webgl::StencilOpSeparate);
  Nan::SetMethod(proto, "bindRenderbuffer", webgl::BindRenderbuffer);
  Nan::SetMethod(proto, "createRenderbuffer", webgl::CreateRenderbuffer);

  Nan::SetMethod(proto, "deleteBuffer", webgl::DeleteBuffer);
  Nan::SetMethod(proto, "deleteFramebuffer", webgl::DeleteFramebuffer);
  Nan::SetMethod(proto, "deleteProgram", webgl::DeleteProgram);
  Nan::SetMethod(proto, "deleteRenderbuffer", webgl::DeleteRenderbuffer);
  Nan::SetMethod(proto, "deleteShader", webgl::DeleteShader);
  Nan::SetMethod(proto, "deleteTexture", webgl::DeleteTexture);
  Nan::SetMethod(proto, "detachShader", webgl::DetachShader);
  Nan::SetMethod(proto, "framebufferRenderbuffer", webgl::FramebufferRenderbuffer);
  Nan::SetMethod(proto, "getVertexAttribOffset", webgl::GetVertexAttribOffset);

  Nan::SetMethod(proto, "isBuffer", setContextWrap<webgl::IsBuffer>);
  Nan::SetMethod(proto, "isFramebuffer", setContextWrap<webgl::IsFramebuffer>);
  Nan::SetMethod(proto, "isProgram", setContextWrap<webgl::IsProgram>);
  Nan::SetMethod(proto, "isRenderbuffer", setContextWrap<webgl::IsRenderbuffer>);
  Nan::SetMethod(proto, "isShader", setContextWrap<webgl::IsShader>);
  Nan::SetMethod(proto, "isTexture", setContextWrap<webgl::IsTexture>);

  Nan::SetMethod(proto, "renderbufferStorage", setContextWrap<webgl::RenderbufferStorage>);
  Nan::SetMethod(proto, "getShaderSource", setContextWrap<webgl::GetShaderSource>);
  Nan::SetMethod(proto, "validateProgram", setContextWrap<webgl::ValidateProgram>);

  Nan::SetMethod(proto, "texSubImage2D", setContextWrap<webgl::TexSubImage2D>);
  Nan::SetMethod(proto, "readPixels", setContextWrap<webgl::ReadPixels>);
  Nan::SetMethod(proto, "getTexParameter", setContextWrap<webgl::GetTexParameter>);
  Nan::SetMethod(proto, "getActiveAttrib", setContextWrap<webgl::GetActiveAttrib>);
  Nan::SetMethod(proto, "getActiveUniform", setContextWrap<webgl::GetActiveUniform>);
  Nan::SetMethod(proto, "getAttachedShaders", setContextWrap<webgl::GetAttachedShaders>);
  Nan::SetMethod(proto, "getParameter", setContextWrap<webgl::GetParameter>);
  Nan::SetMethod(proto, "getBufferParameter", setContextWrap<webgl::GetBufferParameter>);
  Nan::SetMethod(proto, "getFramebufferAttachmentParameter", setContextWrap<webgl::GetFramebufferAttachmentParameter>);
  Nan::SetMethod(proto, "getProgramInfoLog", setContextWrap<webgl::GetProgramInfoLog>);
  Nan::SetMethod(proto, "getRenderbufferParameter", setContextWrap<webgl::GetRenderbufferParameter>);
  Nan::SetMethod(proto, "getVertexAttrib", setContextWrap<webgl::GetVertexAttrib>);
  Nan::SetMethod(proto, "getSupportedExtensions", setContextWrap<webgl::GetSupportedExtensions>);
  Nan::SetMethod(proto, "getExtension", setContextWrap<webgl::GetExtension>);
  Nan::SetMethod(proto, "checkFramebufferStatus", setContextWrap<webgl::CheckFramebufferStatus>);

  Nan::SetMethod(proto, "frontFace", setContextWrap<webgl::FrontFace>);

  // OpenGL ES 2.1 constants

  /* ClearBufferMask */
  JS_GL_CONSTANT(DEPTH_BUFFER_BIT);
  JS_GL_CONSTANT(STENCIL_BUFFER_BIT);
  JS_GL_CONSTANT(COLOR_BUFFER_BIT);

  /* Boolean */
  JS_GL_CONSTANT(FALSE);
  JS_GL_CONSTANT(TRUE);

  /* BeginMode */
  JS_GL_CONSTANT(POINTS);
  JS_GL_CONSTANT(LINES);
  JS_GL_CONSTANT(LINE_LOOP);
  JS_GL_CONSTANT(LINE_STRIP);
  JS_GL_CONSTANT(TRIANGLES);
  JS_GL_CONSTANT(TRIANGLE_STRIP);
  JS_GL_CONSTANT(TRIANGLE_FAN);

  /* AlphaFunction (not supported in ES20) */
  /*      GL_NEVER */
  /*      GL_LESS */
  /*      GL_EQUAL */
  /*      GL_LEQUAL */
  /*      GL_GREATER */
  /*      GL_NOTEQUAL */
  /*      GL_GEQUAL */
  /*      GL_ALWAYS */

  /* BlendingFactorDest */
  JS_GL_CONSTANT(ZERO);
  JS_GL_CONSTANT(ONE);
  JS_GL_CONSTANT(SRC_COLOR);
  JS_GL_CONSTANT(ONE_MINUS_SRC_COLOR);
  JS_GL_CONSTANT(SRC_ALPHA);
  JS_GL_CONSTANT(ONE_MINUS_SRC_ALPHA);
  JS_GL_CONSTANT(DST_ALPHA);
  JS_GL_CONSTANT(ONE_MINUS_DST_ALPHA);

  /* BlendingFactorSrc */
  /*      GL_ZERO */
  /*      GL_ONE */
  JS_GL_CONSTANT(DST_COLOR);
  JS_GL_CONSTANT(ONE_MINUS_DST_COLOR);
  JS_GL_CONSTANT(SRC_ALPHA_SATURATE);
  /*      GL_SRC_ALPHA */
  /*      GL_ONE_MINUS_SRC_ALPHA */
  /*      GL_DST_ALPHA */
  /*      GL_ONE_MINUS_DST_ALPHA */

  /* BlendEquationSeparate */
  JS_GL_CONSTANT(FUNC_ADD);
  JS_GL_CONSTANT(BLEND_EQUATION);
  JS_GL_CONSTANT(BLEND_EQUATION_RGB);    /* same as BLEND_EQUATION */
  JS_GL_CONSTANT(BLEND_EQUATION_ALPHA);

  /* BlendSubtract */
  JS_GL_CONSTANT(FUNC_SUBTRACT);
  JS_GL_CONSTANT(FUNC_REVERSE_SUBTRACT);

  /* Separate Blend Functions */
  JS_GL_CONSTANT(BLEND_DST_RGB);
  JS_GL_CONSTANT(BLEND_SRC_RGB);
  JS_GL_CONSTANT(BLEND_DST_ALPHA);
  JS_GL_CONSTANT(BLEND_SRC_ALPHA);
  JS_GL_CONSTANT(CONSTANT_COLOR);
  JS_GL_CONSTANT(ONE_MINUS_CONSTANT_COLOR);
  JS_GL_CONSTANT(CONSTANT_ALPHA);
  JS_GL_CONSTANT(ONE_MINUS_CONSTANT_ALPHA);
  JS_GL_CONSTANT(BLEND_COLOR);

  /* Buffer Objects */
  JS_GL_CONSTANT(ARRAY_BUFFER);
  JS_GL_CONSTANT(ELEMENT_ARRAY_BUFFER);
  JS_GL_CONSTANT(ARRAY_BUFFER_BINDING);
  JS_GL_CONSTANT(ELEMENT_ARRAY_BUFFER_BINDING);

  JS_GL_CONSTANT(STREAM_DRAW);
  JS_GL_CONSTANT(STATIC_DRAW);
  JS_GL_CONSTANT(DYNAMIC_DRAW);

  JS_GL_CONSTANT(BUFFER_SIZE);
  JS_GL_CONSTANT(BUFFER_USAGE);

  JS_GL_CONSTANT(CURRENT_VERTEX_ATTRIB);

  /* CullFaceMode */
  JS_GL_CONSTANT(FRONT);
  JS_GL_CONSTANT(BACK);
  JS_GL_CONSTANT(FRONT_AND_BACK);

  /* DepthFunction */
  /*      GL_NEVER */
  /*      GL_LESS */
  /*      GL_EQUAL */
  /*      GL_LEQUAL */
  /*      GL_GREATER */
  /*      GL_NOTEQUAL */
  /*      GL_GEQUAL */
  /*      GL_ALWAYS */

  /* EnableCap */
  JS_GL_CONSTANT(TEXTURE_2D);
  JS_GL_CONSTANT(CULL_FACE);
  JS_GL_CONSTANT(BLEND);
  JS_GL_CONSTANT(DITHER);
  JS_GL_CONSTANT(STENCIL_TEST);
  JS_GL_CONSTANT(DEPTH_TEST);
  JS_GL_CONSTANT(SCISSOR_TEST);
  JS_GL_CONSTANT(POLYGON_OFFSET_FILL);
  JS_GL_CONSTANT(SAMPLE_ALPHA_TO_COVERAGE);
  JS_GL_CONSTANT(SAMPLE_COVERAGE);

  /* ErrorCode */
  JS_GL_CONSTANT(NO_ERROR);
  JS_GL_CONSTANT(INVALID_ENUM);
  JS_GL_CONSTANT(INVALID_VALUE);
  JS_GL_CONSTANT(INVALID_OPERATION);
  JS_GL_CONSTANT(OUT_OF_MEMORY);

  /* FrontFaceDirection */
  JS_GL_CONSTANT(CW);
  JS_GL_CONSTANT(CCW);

  /* GetPName */
  JS_GL_CONSTANT(LINE_WIDTH);
  JS_GL_CONSTANT(ALIASED_POINT_SIZE_RANGE);
  JS_GL_CONSTANT(ALIASED_LINE_WIDTH_RANGE);
  JS_GL_CONSTANT(CULL_FACE_MODE);
  JS_GL_CONSTANT(FRONT_FACE);
  JS_GL_CONSTANT(DEPTH_RANGE);
  JS_GL_CONSTANT(DEPTH_WRITEMASK);
  JS_GL_CONSTANT(DEPTH_CLEAR_VALUE);
  JS_GL_CONSTANT(DEPTH_FUNC);
  JS_GL_CONSTANT(STENCIL_CLEAR_VALUE);
  JS_GL_CONSTANT(STENCIL_FUNC);
  JS_GL_CONSTANT(STENCIL_FAIL);
  JS_GL_CONSTANT(STENCIL_PASS_DEPTH_FAIL);
  JS_GL_CONSTANT(STENCIL_PASS_DEPTH_PASS);
  JS_GL_CONSTANT(STENCIL_REF);
  JS_GL_CONSTANT(STENCIL_VALUE_MASK);
  JS_GL_CONSTANT(STENCIL_WRITEMASK);
  JS_GL_CONSTANT(STENCIL_BACK_FUNC);
  JS_GL_CONSTANT(STENCIL_BACK_FAIL);
  JS_GL_CONSTANT(STENCIL_BACK_PASS_DEPTH_FAIL);
  JS_GL_CONSTANT(STENCIL_BACK_PASS_DEPTH_PASS);
  JS_GL_CONSTANT(STENCIL_BACK_REF);
  JS_GL_CONSTANT(STENCIL_BACK_VALUE_MASK);
  JS_GL_CONSTANT(STENCIL_BACK_WRITEMASK);
  JS_GL_CONSTANT(VIEWPORT);
  JS_GL_CONSTANT(SCISSOR_BOX);
  /*      GL_SCISSOR_TEST */
  JS_GL_CONSTANT(COLOR_CLEAR_VALUE);
  JS_GL_CONSTANT(COLOR_WRITEMASK);
  JS_GL_CONSTANT(UNPACK_ALIGNMENT);
  JS_GL_CONSTANT(PACK_ALIGNMENT);
  JS_GL_CONSTANT(MAX_TEXTURE_SIZE);
  JS_GL_CONSTANT(MAX_VIEWPORT_DIMS);
  JS_GL_CONSTANT(SUBPIXEL_BITS);
  JS_GL_CONSTANT(RED_BITS);
  JS_GL_CONSTANT(GREEN_BITS);
  JS_GL_CONSTANT(BLUE_BITS);
  JS_GL_CONSTANT(ALPHA_BITS);
  JS_GL_CONSTANT(DEPTH_BITS);
  JS_GL_CONSTANT(STENCIL_BITS);
  JS_GL_CONSTANT(POLYGON_OFFSET_UNITS);
  /*      GL_POLYGON_OFFSET_FILL */
  JS_GL_CONSTANT(POLYGON_OFFSET_FACTOR);
  JS_GL_CONSTANT(TEXTURE_BINDING_2D);
  JS_GL_CONSTANT(SAMPLE_BUFFERS);
  JS_GL_CONSTANT(SAMPLES);
  JS_GL_CONSTANT(SAMPLE_COVERAGE_VALUE);
  JS_GL_CONSTANT(SAMPLE_COVERAGE_INVERT);

  /* GetTextureParameter */
  /*      GL_TEXTURE_MAG_FILTER */
  /*      GL_TEXTURE_MIN_FILTER */
  /*      GL_TEXTURE_WRAP_S */
  /*      GL_TEXTURE_WRAP_T */

  JS_GL_CONSTANT(NUM_COMPRESSED_TEXTURE_FORMATS);
  JS_GL_CONSTANT(COMPRESSED_TEXTURE_FORMATS);

  /* HintMode */
  JS_GL_CONSTANT(DONT_CARE);
  JS_GL_CONSTANT(FASTEST);
  JS_GL_CONSTANT(NICEST);

  /* HintTarget */
  JS_GL_CONSTANT(GENERATE_MIPMAP_HINT);

  /* DataType */
  JS_GL_CONSTANT(BYTE);
  JS_GL_CONSTANT(UNSIGNED_BYTE);
  JS_GL_CONSTANT(SHORT);
  JS_GL_CONSTANT(UNSIGNED_SHORT);
  JS_GL_CONSTANT(INT);
  JS_GL_CONSTANT(UNSIGNED_INT);
  JS_GL_CONSTANT(FLOAT);
#ifndef __APPLE__
  JS_GL_CONSTANT(FIXED);
#endif

  /* PixelFormat */
  JS_GL_CONSTANT(DEPTH_COMPONENT);
  JS_GL_CONSTANT(ALPHA);
  JS_GL_CONSTANT(RGB);
  JS_GL_CONSTANT(RGBA);
  JS_GL_CONSTANT(LUMINANCE);
  JS_GL_CONSTANT(LUMINANCE_ALPHA);

  /* PixelType */
  /*      GL_UNSIGNED_BYTE */
  JS_GL_CONSTANT(UNSIGNED_SHORT_4_4_4_4);
  JS_GL_CONSTANT(UNSIGNED_SHORT_5_5_5_1);
  JS_GL_CONSTANT(UNSIGNED_SHORT_5_6_5);

  /* Shaders */
  JS_GL_CONSTANT(FRAGMENT_SHADER);
  JS_GL_CONSTANT(VERTEX_SHADER);
  JS_GL_CONSTANT(MAX_VERTEX_ATTRIBS);
#ifndef __APPLE__
  JS_GL_CONSTANT(MAX_VERTEX_UNIFORM_VECTORS);
  JS_GL_CONSTANT(MAX_VARYING_VECTORS);
#endif
  JS_GL_CONSTANT(MAX_COMBINED_TEXTURE_IMAGE_UNITS);
  JS_GL_CONSTANT(MAX_VERTEX_TEXTURE_IMAGE_UNITS);
  JS_GL_CONSTANT(MAX_TEXTURE_IMAGE_UNITS);
#ifndef __APPLE__
  JS_GL_CONSTANT(MAX_FRAGMENT_UNIFORM_VECTORS);
#endif
  JS_GL_CONSTANT(SHADER_TYPE);
  JS_GL_CONSTANT(DELETE_STATUS);
  JS_GL_CONSTANT(LINK_STATUS);
  JS_GL_CONSTANT(VALIDATE_STATUS);
  JS_GL_CONSTANT(ATTACHED_SHADERS);
  JS_GL_CONSTANT(ACTIVE_UNIFORMS);
  JS_GL_CONSTANT(ACTIVE_UNIFORM_MAX_LENGTH);
  JS_GL_CONSTANT(ACTIVE_ATTRIBUTES);
  JS_GL_CONSTANT(ACTIVE_ATTRIBUTE_MAX_LENGTH);
  JS_GL_CONSTANT(SHADING_LANGUAGE_VERSION);
  JS_GL_CONSTANT(CURRENT_PROGRAM);

  /* StencilFunction */
  JS_GL_CONSTANT(NEVER);
  JS_GL_CONSTANT(LESS);
  JS_GL_CONSTANT(EQUAL);
  JS_GL_CONSTANT(LEQUAL);
  JS_GL_CONSTANT(GREATER);
  JS_GL_CONSTANT(NOTEQUAL);
  JS_GL_CONSTANT(GEQUAL);
  JS_GL_CONSTANT(ALWAYS);

  /* StencilOp */
  /*      GL_ZERO */
  JS_GL_CONSTANT(KEEP);
  JS_GL_CONSTANT(REPLACE);
  JS_GL_CONSTANT(INCR);
  JS_GL_CONSTANT(DECR);
  JS_GL_CONSTANT(INVERT);
  JS_GL_CONSTANT(INCR_WRAP);
  JS_GL_CONSTANT(DECR_WRAP);

  /* StringName */
  JS_GL_CONSTANT(VENDOR);
  JS_GL_CONSTANT(RENDERER);
  JS_GL_CONSTANT(VERSION);
  JS_GL_CONSTANT(EXTENSIONS);

  /* TextureMagFilter */
  JS_GL_CONSTANT(NEAREST);
  JS_GL_CONSTANT(LINEAR);

  /* TextureMinFilter */
  /*      GL_NEAREST */
  /*      GL_LINEAR */
  JS_GL_CONSTANT(NEAREST_MIPMAP_NEAREST);
  JS_GL_CONSTANT(LINEAR_MIPMAP_NEAREST);
  JS_GL_CONSTANT(NEAREST_MIPMAP_LINEAR);
  JS_GL_CONSTANT(LINEAR_MIPMAP_LINEAR);

  /* TextureParameterName */
  JS_GL_CONSTANT(TEXTURE_MAG_FILTER);
  JS_GL_CONSTANT(TEXTURE_MIN_FILTER);
  JS_GL_CONSTANT(TEXTURE_WRAP_S);
  JS_GL_CONSTANT(TEXTURE_WRAP_T);

  /* TextureTarget */
  /*      GL_TEXTURE_2D */
  JS_GL_CONSTANT(TEXTURE);

  JS_GL_CONSTANT(TEXTURE_CUBE_MAP);
  JS_GL_CONSTANT(TEXTURE_BINDING_CUBE_MAP);
  JS_GL_CONSTANT(TEXTURE_CUBE_MAP_POSITIVE_X);
  JS_GL_CONSTANT(TEXTURE_CUBE_MAP_NEGATIVE_X);
  JS_GL_CONSTANT(TEXTURE_CUBE_MAP_POSITIVE_Y);
  JS_GL_CONSTANT(TEXTURE_CUBE_MAP_NEGATIVE_Y);
  JS_GL_CONSTANT(TEXTURE_CUBE_MAP_POSITIVE_Z);
  JS_GL_CONSTANT(TEXTURE_CUBE_MAP_NEGATIVE_Z);
  JS_GL_CONSTANT(MAX_CUBE_MAP_TEXTURE_SIZE);

  /* TextureUnit */
  JS_GL_CONSTANT(TEXTURE0);
  JS_GL_CONSTANT(TEXTURE1);
  JS_GL_CONSTANT(TEXTURE2);
  JS_GL_CONSTANT(TEXTURE3);
  JS_GL_CONSTANT(TEXTURE4);
  JS_GL_CONSTANT(TEXTURE5);
  JS_GL_CONSTANT(TEXTURE6);
  JS_GL_CONSTANT(TEXTURE7);
  JS_GL_CONSTANT(TEXTURE8);
  JS_GL_CONSTANT(TEXTURE9);
  JS_GL_CONSTANT(TEXTURE10);
  JS_GL_CONSTANT(TEXTURE11);
  JS_GL_CONSTANT(TEXTURE12);
  JS_GL_CONSTANT(TEXTURE13);
  JS_GL_CONSTANT(TEXTURE14);
  JS_GL_CONSTANT(TEXTURE15);
  JS_GL_CONSTANT(TEXTURE16);
  JS_GL_CONSTANT(TEXTURE17);
  JS_GL_CONSTANT(TEXTURE18);
  JS_GL_CONSTANT(TEXTURE19);
  JS_GL_CONSTANT(TEXTURE20);
  JS_GL_CONSTANT(TEXTURE21);
  JS_GL_CONSTANT(TEXTURE22);
  JS_GL_CONSTANT(TEXTURE23);
  JS_GL_CONSTANT(TEXTURE24);
  JS_GL_CONSTANT(TEXTURE25);
  JS_GL_CONSTANT(TEXTURE26);
  JS_GL_CONSTANT(TEXTURE27);
  JS_GL_CONSTANT(TEXTURE28);
  JS_GL_CONSTANT(TEXTURE29);
  JS_GL_CONSTANT(TEXTURE30);
  JS_GL_CONSTANT(TEXTURE31);
  JS_GL_CONSTANT(ACTIVE_TEXTURE);

  /* TextureWrapMode */
  JS_GL_CONSTANT(REPEAT);
  JS_GL_CONSTANT(CLAMP_TO_EDGE);
  JS_GL_CONSTANT(MIRRORED_REPEAT);

  /* Uniform Types */
  JS_GL_CONSTANT(FLOAT_VEC2);
  JS_GL_CONSTANT(FLOAT_VEC3);
  JS_GL_CONSTANT(FLOAT_VEC4);
  JS_GL_CONSTANT(INT_VEC2);
  JS_GL_CONSTANT(INT_VEC3);
  JS_GL_CONSTANT(INT_VEC4);
  JS_GL_CONSTANT(BOOL);
  JS_GL_CONSTANT(BOOL_VEC2);
  JS_GL_CONSTANT(BOOL_VEC3);
  JS_GL_CONSTANT(BOOL_VEC4);
  JS_GL_CONSTANT(FLOAT_MAT2);
  JS_GL_CONSTANT(FLOAT_MAT3);
  JS_GL_CONSTANT(FLOAT_MAT4);
  JS_GL_CONSTANT(SAMPLER_2D);
  JS_GL_CONSTANT(SAMPLER_CUBE);

  /* Vertex Arrays */
  JS_GL_CONSTANT(VERTEX_ATTRIB_ARRAY_ENABLED);
  JS_GL_CONSTANT(VERTEX_ATTRIB_ARRAY_SIZE);
  JS_GL_CONSTANT(VERTEX_ATTRIB_ARRAY_STRIDE);
  JS_GL_CONSTANT(VERTEX_ATTRIB_ARRAY_TYPE);
  JS_GL_CONSTANT(VERTEX_ATTRIB_ARRAY_NORMALIZED);
  JS_GL_CONSTANT(VERTEX_ATTRIB_ARRAY_POINTER);
  JS_GL_CONSTANT(VERTEX_ATTRIB_ARRAY_BUFFER_BINDING);

  /* Read Format */
#ifndef __APPLE__
  JS_GL_CONSTANT(IMPLEMENTATION_COLOR_READ_TYPE);
  JS_GL_CONSTANT(IMPLEMENTATION_COLOR_READ_FORMAT);
#endif

  /* Shader Source */
  JS_GL_CONSTANT(COMPILE_STATUS);
  JS_GL_CONSTANT(INFO_LOG_LENGTH);
  JS_GL_CONSTANT(SHADER_SOURCE_LENGTH);
#ifndef __APPLE__
  JS_GL_CONSTANT(SHADER_COMPILER);
#endif

  /* Shader Binary */
#ifndef __APPLE__
  JS_GL_CONSTANT(SHADER_BINARY_FORMATS);
  JS_GL_CONSTANT(NUM_SHADER_BINARY_FORMATS);
#endif

  /* Shader Precision-Specified Types */
#ifndef __APPLE__
  JS_GL_CONSTANT(LOW_FLOAT);
  JS_GL_CONSTANT(MEDIUM_FLOAT);
  JS_GL_CONSTANT(HIGH_FLOAT);
  JS_GL_CONSTANT(LOW_INT);
  JS_GL_CONSTANT(MEDIUM_INT);
  JS_GL_CONSTANT(HIGH_INT);
#endif

  /* Framebuffer Object. */
  JS_GL_CONSTANT(FRAMEBUFFER);
  JS_GL_CONSTANT(RENDERBUFFER);

  JS_GL_CONSTANT(RGBA4);
  JS_GL_CONSTANT(RGB5_A1);
#ifndef __APPLE__
  //JS_GL_CONSTANT(RGB565);
#endif
  JS_GL_CONSTANT(DEPTH_COMPONENT16);
  // JS_GL_CONSTANT(STENCIL_INDEX);
  JS_GL_CONSTANT(STENCIL_INDEX8);
  JS_GL_CONSTANT(DEPTH_STENCIL);
  JS_GL_CONSTANT(DEPTH24_STENCIL8);

  JS_GL_CONSTANT(RENDERBUFFER_WIDTH);
  JS_GL_CONSTANT(RENDERBUFFER_HEIGHT);
  JS_GL_CONSTANT(RENDERBUFFER_INTERNAL_FORMAT);
  JS_GL_CONSTANT(RENDERBUFFER_RED_SIZE);
  JS_GL_CONSTANT(RENDERBUFFER_GREEN_SIZE);
  JS_GL_CONSTANT(RENDERBUFFER_BLUE_SIZE);
  JS_GL_CONSTANT(RENDERBUFFER_ALPHA_SIZE);
  JS_GL_CONSTANT(RENDERBUFFER_DEPTH_SIZE);
  JS_GL_CONSTANT(RENDERBUFFER_STENCIL_SIZE);

  JS_GL_CONSTANT(FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE);
  JS_GL_CONSTANT(FRAMEBUFFER_ATTACHMENT_OBJECT_NAME);
  JS_GL_CONSTANT(FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL);
  JS_GL_CONSTANT(FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE);

  JS_GL_CONSTANT(COLOR_ATTACHMENT0);
  JS_GL_CONSTANT(DEPTH_ATTACHMENT);
  JS_GL_CONSTANT(STENCIL_ATTACHMENT);
  JS_GL_CONSTANT(DEPTH_STENCIL_ATTACHMENT);

  JS_GL_CONSTANT(NONE);

  JS_GL_CONSTANT(FRAMEBUFFER_COMPLETE);
  JS_GL_CONSTANT(FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
  JS_GL_CONSTANT(FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
#ifndef __APPLE__
  //JS_GL_CONSTANT(FRAMEBUFFER_INCOMPLETE_DIMENSIONS);
#endif
  JS_GL_CONSTANT(FRAMEBUFFER_UNSUPPORTED);

  JS_GL_CONSTANT(FRAMEBUFFER_BINDING);
  JS_GL_CONSTANT(RENDERBUFFER_BINDING);
  JS_GL_CONSTANT(MAX_RENDERBUFFER_SIZE);

  JS_GL_CONSTANT(INVALID_FRAMEBUFFER_OPERATION);

  /* WebGL-specific enums */
  JS_GL_SET_CONSTANT( "UNPACK_FLIP_Y_WEBGL" , 0x9240);
  JS_GL_SET_CONSTANT("UNPACK_PREMULTIPLY_ALPHA_WEBGL" , 0x9241);
  JS_GL_SET_CONSTANT("CONTEXT_LOST_WEBGL" , 0x9242);
  JS_GL_SET_CONSTANT("UNPACK_COLORSPACE_CONVERSION_WEBGL", 0x9243);
  JS_GL_SET_CONSTANT("BROWSER_DEFAULT_WEBGL" , 0x9244);

  //////////////////////////////
  // NOT in WebGL spec
  //////////////////////////////

  // PBO
  JS_GL_SET_CONSTANT("PIXEL_PACK_BUFFER" , 0x88EB);
  JS_GL_SET_CONSTANT("PIXEL_UNPACK_BUFFER" , 0x88EC);
  JS_GL_SET_CONSTANT("PIXEL_PACK_BUFFER_BINDING" , 0x88ED);
  JS_GL_SET_CONSTANT("PIXEL_UNPACK_BUFFER_BINDING", 0x88EF);

  // external
  JS_GL_SET_CONSTANT("TEXTURE_EXTERNAL_OES", 0x8D65);

  Local<Function> ctorFn = ctor->GetFunction();
  return scope.Escape(ctorFn);
}

WebGLContext::WebGLContext() {}

WebGLContext::~WebGLContext() {}

NAN_METHOD(WebGLContext::New) {
  Nan::HandleScope scope;

  WebGLContext *gl = new WebGLContext();
  Local<Object> glObj = info.This();
  gl->Wrap(glObj);

  info.GetReturnValue().Set(glObj);
}

NAN_METHOD(WebGLContext::GetWindowHandle) {
  Nan::HandleScope scope;

  WebGLContext *gl = ObjectWrap::Unwrap<WebGLContext>(info.This());
  if (gl->windowHandle) {
    info.GetReturnValue().Set(glfw::pointerToArray(gl->windowHandle));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(WebGLContext::SetWindowHandle) {
  Nan::HandleScope scope;

  WebGLContext *gl = ObjectWrap::Unwrap<WebGLContext>(info.This());
  if (info[0]->IsArray()) {
    gl->windowHandle = (GLFWwindow *)glfw::arrayToPointer(Local<Array>::Cast(info[0]));
  } else {
    gl->windowHandle = nullptr;
  }
}

Local<Object> makeGl() {
  return WebGLContext::Initialize(Isolate::GetCurrent());
}

Local<Object> makeImage() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  return scope.Escape(Image::Initialize(isolate));
}

Local<Object> makeImageData() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  return scope.Escape(ImageData::Initialize(isolate));
}

Local<Object> makeImageBitmap(Local<Value> imageCons) {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  return scope.Escape(ImageBitmap::Initialize(isolate, imageCons));
}

Local<Object> makeCanvasRenderingContext2D(Local<Value> imageDataCons) {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  return scope.Escape(CanvasRenderingContext2D::Initialize(isolate, imageDataCons));
}

Local<Object> makePath2D() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  return scope.Escape(Path2D::Initialize(isolate));
}

Local<Object> makeAudio() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  Local<Object> exports = Nan::New<Object>();

  exports->Set(JS_STR("Audio"), webaudio::Audio::Initialize(isolate));
  Local<Value> audioParamCons = webaudio::AudioParam::Initialize(isolate);
  exports->Set(JS_STR("AudioParam"), audioParamCons);
  Local<Value> fakeAudioParamCons = webaudio::FakeAudioParam::Initialize(isolate);
  // exports->Set(JS_STR("FakeAudioParam"), fakeAudioParamCons);
  Local<Value> audioListenerCons = webaudio::AudioListener::Initialize(isolate, fakeAudioParamCons);
  exports->Set(JS_STR("AudioListener"), audioListenerCons);
  Local<Value> audioSourceNodeCons = webaudio::AudioSourceNode::Initialize(isolate);
  // exports->Set(JS_STR("AudioSourceNode"), audioSourceNodeCons);
  Local<Value> audioDestinationNodeCons = webaudio::AudioDestinationNode::Initialize(isolate);
  exports->Set(JS_STR("AudioDestinationNode"), audioDestinationNodeCons);
  Local<Value> gainNodeCons = webaudio::GainNode::Initialize(isolate, audioParamCons);
  exports->Set(JS_STR("GainNode"), gainNodeCons);
  Local<Value> analyserNodeCons = webaudio::AnalyserNode::Initialize(isolate);
  exports->Set(JS_STR("AnalyserNode"), analyserNodeCons);
  Local<Value> pannerNodeCons = webaudio::PannerNode::Initialize(isolate, fakeAudioParamCons);
  exports->Set(JS_STR("PannerNode"), pannerNodeCons);
  Local<Value> stereoPannerNodeCons = webaudio::StereoPannerNode::Initialize(isolate, audioParamCons);
  exports->Set(JS_STR("StereoPannerNode"), stereoPannerNodeCons);
  Local<Value> microphoneMediaStreamCons = webaudio::MicrophoneMediaStream::Initialize(isolate);
  exports->Set(JS_STR("MicrophoneMediaStream"), microphoneMediaStreamCons);
  exports->Set(JS_STR("AudioContext"), webaudio::AudioContext::Initialize(isolate, audioListenerCons, audioSourceNodeCons, audioDestinationNodeCons, gainNodeCons, analyserNodeCons, pannerNodeCons, stereoPannerNodeCons));

  return scope.Escape(exports);
}

Local<Object> makeVideo() {
  Isolate *isolate = Isolate::GetCurrent();

  Nan::EscapableHandleScope scope;

  Local<Object> exports = Nan::New<Object>();

  exports->Set(JS_STR("Video"), ffmpeg::Video::Initialize(isolate));

  return scope.Escape(exports);
}
