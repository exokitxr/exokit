/*
 * bindings.cc
 *
 *  Created on: Dec 13, 2011
 *      Author: ngk437
 */

#include "bindings.h"

// Used to be a macro, hence the uppercase name.
#define JS_GL_SET_CONSTANT(name, constant) target->Set(JS_STR( name ), JS_INT(constant))

//#define JS_GL_SET_CONSTANT(name, value) NODE_DEFINE_CONSTANTNODE_DEFINE_CONSTANT

#define JS_GL_CONSTANT(name) JS_GL_SET_CONSTANT(#name, GL_ ## name)

NAN_METHOD(newGl) {
  Nan::HandleScope scope;

  Local<Object> target(info.This());

  Nan::SetMethod(target, "uniform1f", webgl::Uniform1f);
  Nan::SetMethod(target, "uniform2f", webgl::Uniform2f);
  Nan::SetMethod(target, "uniform3f", webgl::Uniform3f);
  Nan::SetMethod(target, "uniform4f", webgl::Uniform4f);
  Nan::SetMethod(target, "uniform1i", webgl::Uniform1i);
  Nan::SetMethod(target, "uniform2i", webgl::Uniform2i);
  Nan::SetMethod(target, "uniform3i", webgl::Uniform3i);
  Nan::SetMethod(target, "uniform4i", webgl::Uniform4i);
  Nan::SetMethod(target, "uniform1fv", webgl::Uniform1fv);
  Nan::SetMethod(target, "uniform2fv", webgl::Uniform2fv);
  Nan::SetMethod(target, "uniform3fv", webgl::Uniform3fv);
  Nan::SetMethod(target, "uniform4fv", webgl::Uniform4fv);
  Nan::SetMethod(target, "uniform1iv", webgl::Uniform1iv);
  Nan::SetMethod(target, "uniform2iv", webgl::Uniform2iv);
  Nan::SetMethod(target, "uniform3iv", webgl::Uniform3iv);
  Nan::SetMethod(target, "uniform4iv", webgl::Uniform4iv);
  Nan::SetMethod(target, "pixelStorei", webgl::PixelStorei);
  Nan::SetMethod(target, "bindAttribLocation", webgl::BindAttribLocation);
  Nan::SetMethod(target, "getError", webgl::GetError);
  Nan::SetMethod(target, "drawArrays", webgl::DrawArrays);
  Nan::SetMethod(target, "drawArraysInstancedANGLE", webgl::DrawArraysInstancedANGLE);
  Nan::SetMethod(target, "uniformMatrix2fv", webgl::UniformMatrix2fv);
  Nan::SetMethod(target, "uniformMatrix3fv", webgl::UniformMatrix3fv);
  Nan::SetMethod(target, "uniformMatrix4fv", webgl::UniformMatrix4fv);

  Nan::SetMethod(target, "generateMipmap", webgl::GenerateMipmap);

  Nan::SetMethod(target, "getAttribLocation", webgl::GetAttribLocation);
  Nan::SetMethod(target, "depthFunc", webgl::DepthFunc);
  Nan::SetMethod(target, "viewport", webgl::Viewport);
  Nan::SetMethod(target, "createShader", webgl::CreateShader);
  Nan::SetMethod(target, "shaderSource", webgl::ShaderSource);
  Nan::SetMethod(target, "compileShader", webgl::CompileShader);
  Nan::SetMethod(target, "getShaderParameter", webgl::GetShaderParameter);
  Nan::SetMethod(target, "getShaderInfoLog", webgl::GetShaderInfoLog);
  Nan::SetMethod(target, "createProgram", webgl::CreateProgram);
  Nan::SetMethod(target, "attachShader", webgl::AttachShader);
  Nan::SetMethod(target, "linkProgram", webgl::LinkProgram);
  Nan::SetMethod(target, "getProgramParameter", webgl::GetProgramParameter);
  Nan::SetMethod(target, "getUniformLocation", webgl::GetUniformLocation);
  Nan::SetMethod(target, "clearColor", webgl::ClearColor);
  Nan::SetMethod(target, "clearDepth", webgl::ClearDepth);

  Nan::SetMethod(target, "disable", webgl::Disable);
  Nan::SetMethod(target, "createTexture", webgl::CreateTexture);
  Nan::SetMethod(target, "bindTexture", webgl::BindTexture);
  // Nan::SetMethod(target, "flipTextureData", webgl::FlipTextureData);
  Nan::SetMethod(target, "texImage2D", webgl::TexImage2D);
  Nan::SetMethod(target, "texParameteri", webgl::TexParameteri);
  Nan::SetMethod(target, "texParameterf", webgl::TexParameterf);
  Nan::SetMethod(target, "clear", webgl::Clear);
  Nan::SetMethod(target, "useProgram", webgl::UseProgram);
  Nan::SetMethod(target, "createFramebuffer", webgl::CreateFramebuffer);
  Nan::SetMethod(target, "bindFramebuffer", webgl::BindFramebuffer);
  Nan::SetMethod(target, "framebufferTexture2D", webgl::FramebufferTexture2D);
  Nan::SetMethod(target, "createBuffer", webgl::CreateBuffer);
  Nan::SetMethod(target, "bindBuffer", webgl::BindBuffer);
  Nan::SetMethod(target, "bufferData", webgl::BufferData);
  Nan::SetMethod(target, "bufferSubData", webgl::BufferSubData);
  Nan::SetMethod(target, "enable", webgl::Enable);
  Nan::SetMethod(target, "blendEquation", webgl::BlendEquation);
  Nan::SetMethod(target, "blendFunc", webgl::BlendFunc);
  Nan::SetMethod(target, "enableVertexAttribArray", webgl::EnableVertexAttribArray);
  Nan::SetMethod(target, "vertexAttribPointer", webgl::VertexAttribPointer);
  Nan::SetMethod(target, "activeTexture", webgl::ActiveTexture);
  Nan::SetMethod(target, "drawElements", webgl::DrawElements);
  Nan::SetMethod(target, "drawElementsInstancedANGLE", webgl::DrawElementsInstancedANGLE);
  Nan::SetMethod(target, "flush", webgl::Flush);
  Nan::SetMethod(target, "finish", webgl::Finish);

  Nan::SetMethod(target, "vertexAttrib1f", webgl::VertexAttrib1f);
  Nan::SetMethod(target, "vertexAttrib2f", webgl::VertexAttrib2f);
  Nan::SetMethod(target, "vertexAttrib3f", webgl::VertexAttrib3f);
  Nan::SetMethod(target, "vertexAttrib4f", webgl::VertexAttrib4f);
  Nan::SetMethod(target, "vertexAttrib1fv", webgl::VertexAttrib1fv);
  Nan::SetMethod(target, "vertexAttrib2fv", webgl::VertexAttrib2fv);
  Nan::SetMethod(target, "vertexAttrib3fv", webgl::VertexAttrib3fv);
  Nan::SetMethod(target, "vertexAttrib4fv", webgl::VertexAttrib4fv);
  Nan::SetMethod(target, "vertexAttribDivisorANGLE", webgl::VertexAttribDivisorANGLE);

  Nan::SetMethod(target, "blendColor", webgl::BlendColor);
  Nan::SetMethod(target, "blendEquationSeparate", webgl::BlendEquationSeparate);
  Nan::SetMethod(target, "blendFuncSeparate", webgl::BlendFuncSeparate);
  Nan::SetMethod(target, "clearStencil", webgl::ClearStencil);
  Nan::SetMethod(target, "colorMask", webgl::ColorMask);
  Nan::SetMethod(target, "copyTexImage2D", webgl::CopyTexImage2D);
  Nan::SetMethod(target, "copyTexSubImage2D", webgl::CopyTexSubImage2D);
  Nan::SetMethod(target, "cullFace", webgl::CullFace);
  Nan::SetMethod(target, "depthMask", webgl::DepthMask);
  Nan::SetMethod(target, "depthRange", webgl::DepthRange);
  Nan::SetMethod(target, "disableVertexAttribArray", webgl::DisableVertexAttribArray);
  Nan::SetMethod(target, "hint", webgl::Hint);
  Nan::SetMethod(target, "isEnabled", webgl::IsEnabled);
  Nan::SetMethod(target, "lineWidth", webgl::LineWidth);
  Nan::SetMethod(target, "polygonOffset", webgl::PolygonOffset);

  Nan::SetMethod(target, "scissor", webgl::Scissor);
  Nan::SetMethod(target, "stencilFunc", webgl::StencilFunc);
  Nan::SetMethod(target, "stencilFuncSeparate", webgl::StencilFuncSeparate);
  Nan::SetMethod(target, "stencilMask", webgl::StencilMask);
  Nan::SetMethod(target, "stencilMaskSeparate", webgl::StencilMaskSeparate);
  Nan::SetMethod(target, "stencilOp", webgl::StencilOp);
  Nan::SetMethod(target, "stencilOpSeparate", webgl::StencilOpSeparate);
  Nan::SetMethod(target, "bindRenderbuffer", webgl::BindRenderbuffer);
  Nan::SetMethod(target, "createRenderbuffer", webgl::CreateRenderbuffer);

  Nan::SetMethod(target, "deleteBuffer", webgl::DeleteBuffer);
  Nan::SetMethod(target, "deleteFramebuffer", webgl::DeleteFramebuffer);
  Nan::SetMethod(target, "deleteProgram", webgl::DeleteProgram);
  Nan::SetMethod(target, "deleteRenderbuffer", webgl::DeleteRenderbuffer);
  Nan::SetMethod(target, "deleteShader", webgl::DeleteShader);
  Nan::SetMethod(target, "deleteTexture", webgl::DeleteTexture);
  Nan::SetMethod(target, "detachShader", webgl::DetachShader);
  Nan::SetMethod(target, "framebufferRenderbuffer", webgl::FramebufferRenderbuffer);
  Nan::SetMethod(target, "getVertexAttribOffset", webgl::GetVertexAttribOffset);

  Nan::SetMethod(target, "isBuffer", webgl::IsBuffer);
  Nan::SetMethod(target, "isFramebuffer", webgl::IsFramebuffer);
  Nan::SetMethod(target, "isProgram", webgl::IsProgram);
  Nan::SetMethod(target, "isRenderbuffer", webgl::IsRenderbuffer);
  Nan::SetMethod(target, "isShader", webgl::IsShader);
  Nan::SetMethod(target, "isTexture", webgl::IsTexture);

  Nan::SetMethod(target, "renderbufferStorage", webgl::RenderbufferStorage);
  Nan::SetMethod(target, "getShaderSource", webgl::GetShaderSource);
  Nan::SetMethod(target, "validateProgram", webgl::ValidateProgram);

  Nan::SetMethod(target, "texSubImage2D", webgl::TexSubImage2D);
  Nan::SetMethod(target, "readPixels", webgl::ReadPixels);
  Nan::SetMethod(target, "getTexParameter", webgl::GetTexParameter);
  Nan::SetMethod(target, "getActiveAttrib", webgl::GetActiveAttrib);
  Nan::SetMethod(target, "getActiveUniform", webgl::GetActiveUniform);
  Nan::SetMethod(target, "getAttachedShaders", webgl::GetAttachedShaders);
  Nan::SetMethod(target, "getParameter", webgl::GetParameter);
  Nan::SetMethod(target, "getBufferParameter", webgl::GetBufferParameter);
  Nan::SetMethod(target, "getFramebufferAttachmentParameter", webgl::GetFramebufferAttachmentParameter);
  Nan::SetMethod(target, "getProgramInfoLog", webgl::GetProgramInfoLog);
  Nan::SetMethod(target, "getRenderbufferParameter", webgl::GetRenderbufferParameter);
  Nan::SetMethod(target, "getVertexAttrib", webgl::GetVertexAttrib);
  Nan::SetMethod(target, "getSupportedExtensions", webgl::GetSupportedExtensions);
  Nan::SetMethod(target, "getExtension", webgl::GetExtension);
  Nan::SetMethod(target, "checkFramebufferStatus", webgl::CheckFramebufferStatus);

  Nan::SetMethod(target, "frontFace", webgl::FrontFace);

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

  info.GetReturnValue().Set(info.This());
}

Local<Object> makeGl() {
  Isolate *isolate = Isolate::GetCurrent();

  v8::EscapableHandleScope scope(isolate);

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(newGl);
  ctor->SetClassName(JS_STR("WebGLContext"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  return scope.Escape(ctor->GetFunction());
}

Local<Object> makeImage() {
  Isolate *isolate = Isolate::GetCurrent();

  v8::EscapableHandleScope scope(isolate);

  return scope.Escape(Image::Initialize(isolate));
}

Local<Object> makeImageData() {
  Isolate *isolate = Isolate::GetCurrent();

  v8::EscapableHandleScope scope(isolate);

  return scope.Escape(ImageData::Initialize(isolate));
}

Local<Object> makeImageBitmap() {
  Isolate *isolate = Isolate::GetCurrent();

  v8::EscapableHandleScope scope(isolate);

  return scope.Escape(ImageBitmap::Initialize(isolate));
}

Local<Object> makeCanvasRenderingContext2D(Local<Value> imageDataCons) {
  Isolate *isolate = Isolate::GetCurrent();

  v8::EscapableHandleScope scope(isolate);

  return scope.Escape(CanvasRenderingContext2D::Initialize(isolate, imageDataCons));
}

Local<Object> makePath2D() {
  Isolate *isolate = Isolate::GetCurrent();

  v8::EscapableHandleScope scope(isolate);

  return scope.Escape(Path2D::Initialize(isolate));
}
