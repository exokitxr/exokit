/*
 * webgl.h
 *
 *  Created on: Dec 13, 2011
 *      Author: ngk437
 */

#ifndef _WEBGLCONTEXT_WEBGL_H_
#define _WEBGLCONTEXT_WEBGL_H_

#include <nan/nan.h>

#if _WIN32
#include <GL/glew.h>
#include <GLES2/gl2platform.h>
#include <GLES2/gl2ext.h>
#endif
#if __ANDROID__
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

using namespace node;
using namespace v8;

namespace webgl {
NAN_METHOD(Uniform1f);
NAN_METHOD(Uniform2f);
NAN_METHOD(Uniform3f);
NAN_METHOD(Uniform4f);
NAN_METHOD(Uniform1i);
NAN_METHOD(Uniform2i);
NAN_METHOD(Uniform3i);
NAN_METHOD(Uniform4i);
NAN_METHOD(Uniform1fv);
NAN_METHOD(Uniform2fv);
NAN_METHOD(Uniform3fv);
NAN_METHOD(Uniform4fv);
NAN_METHOD(Uniform1iv);
NAN_METHOD(Uniform2iv);
NAN_METHOD(Uniform3iv);
NAN_METHOD(Uniform4iv);
NAN_METHOD(PixelStorei);
NAN_METHOD(BindAttribLocation);
NAN_METHOD(GetError);
NAN_METHOD(DrawArrays);
NAN_METHOD(DrawArraysInstancedANGLE);
NAN_METHOD(UniformMatrix2fv);
NAN_METHOD(UniformMatrix3fv);
NAN_METHOD(UniformMatrix4fv);
NAN_METHOD(GenerateMipmap);
NAN_METHOD(GetAttribLocation);
NAN_METHOD(DepthFunc);
NAN_METHOD(Viewport);
NAN_METHOD(CreateShader);
NAN_METHOD(ShaderSource);
NAN_METHOD(CompileShader);
NAN_METHOD(GetShaderParameter);
NAN_METHOD(GetShaderInfoLog);
NAN_METHOD(CreateProgram);
NAN_METHOD(AttachShader);
NAN_METHOD(LinkProgram);
NAN_METHOD(GetProgramParameter);
NAN_METHOD(GetUniformLocation);
NAN_METHOD(ClearColor);
NAN_METHOD(ClearDepth);
NAN_METHOD(Disable);
NAN_METHOD(Enable);
NAN_METHOD(CreateTexture);
NAN_METHOD(BindTexture);
NAN_METHOD(FlipTextureData);
NAN_METHOD(TexImage2D);
NAN_METHOD(TexParameteri);
NAN_METHOD(TexParameterf);
NAN_METHOD(Clear);
NAN_METHOD(UseProgram);
NAN_METHOD(CreateBuffer);
NAN_METHOD(BindBuffer);
NAN_METHOD(CreateFramebuffer);
NAN_METHOD(BindFramebuffer);
NAN_METHOD(FramebufferTexture2D);
NAN_METHOD(BufferData);
NAN_METHOD(BufferSubData);
NAN_METHOD(BlendEquation);
NAN_METHOD(BlendFunc);
NAN_METHOD(EnableVertexAttribArray);
NAN_METHOD(VertexAttribPointer);
NAN_METHOD(ActiveTexture);
NAN_METHOD(DrawElements);
NAN_METHOD(DrawElementsInstancedANGLE);
NAN_METHOD(Flush);
NAN_METHOD(Finish);

NAN_METHOD(VertexAttrib1f);
NAN_METHOD(VertexAttrib2f);
NAN_METHOD(VertexAttrib3f);
NAN_METHOD(VertexAttrib4f);
NAN_METHOD(VertexAttrib1fv);
NAN_METHOD(VertexAttrib2fv);
NAN_METHOD(VertexAttrib3fv);
NAN_METHOD(VertexAttrib4fv);
NAN_METHOD(VertexAttribDivisorANGLE);

NAN_METHOD(BlendColor);
NAN_METHOD(BlendEquationSeparate);
NAN_METHOD(BlendFuncSeparate);
NAN_METHOD(ClearStencil);
NAN_METHOD(ColorMask);
NAN_METHOD(CopyTexImage2D);
NAN_METHOD(CopyTexSubImage2D);
NAN_METHOD(CullFace);
NAN_METHOD(DepthMask);
NAN_METHOD(DepthRange);
NAN_METHOD(DisableVertexAttribArray);
NAN_METHOD(Hint);
NAN_METHOD(IsEnabled);
NAN_METHOD(LineWidth);
NAN_METHOD(PolygonOffset);

NAN_METHOD(Scissor);
NAN_METHOD(StencilFunc);
NAN_METHOD(StencilFuncSeparate);
NAN_METHOD(StencilMask);
NAN_METHOD(StencilMaskSeparate);
NAN_METHOD(StencilOp);
NAN_METHOD(StencilOpSeparate);
NAN_METHOD(BindRenderbuffer);
NAN_METHOD(CreateRenderbuffer);

NAN_METHOD(DeleteBuffer);
NAN_METHOD(DeleteFramebuffer);
NAN_METHOD(DeleteProgram);
NAN_METHOD(DeleteRenderbuffer);
NAN_METHOD(DeleteShader);
NAN_METHOD(DeleteTexture);
NAN_METHOD(DetachShader);
NAN_METHOD(FramebufferRenderbuffer);
NAN_METHOD(GetVertexAttribOffset);

NAN_METHOD(IsBuffer);
NAN_METHOD(IsFramebuffer);
NAN_METHOD(IsProgram);
NAN_METHOD(IsRenderbuffer);
NAN_METHOD(IsShader);
NAN_METHOD(IsTexture);

NAN_METHOD(RenderbufferStorage);
NAN_METHOD(GetShaderSource);
NAN_METHOD(ValidateProgram);

NAN_METHOD(TexSubImage2D);
NAN_METHOD(ReadPixels);
NAN_METHOD(GetTexParameter);
NAN_METHOD(GetActiveAttrib);
NAN_METHOD(GetActiveUniform);
NAN_METHOD(GetAttachedShaders);
NAN_METHOD(GetParameter);
NAN_METHOD(GetBufferParameter);
NAN_METHOD(GetFramebufferAttachmentParameter);
NAN_METHOD(GetProgramInfoLog);
NAN_METHOD(GetRenderbufferParameter);
NAN_METHOD(GetVertexAttrib);
NAN_METHOD(GetSupportedExtensions);
NAN_METHOD(GetExtension);
NAN_METHOD(CheckFramebufferStatus);

NAN_METHOD(FrontFace);
}

#endif
