/*
 * webgl.h
 *
 *  Created on: Dec 13, 2011
 *      Author: ngk437
 */

#ifndef _WEBGLCONTEXT_WEBGL_H_
#define _WEBGLCONTEXT_WEBGL_H_

#include <nan.h>

#if defined(LUMIN) || defined(__ANDROID__)
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#define GL_ETC1_RGB8_OES 0x8D64
#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642

#elif defined(_WIN32)
#include <GL/glew.h>
#include <GLES2/gl2platform.h>
#include <GLES2/gl2ext.h>

#elif defined(__APPLE__)
#if TARGET_OS_IPHONE
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include <OpenGLES/ES2/glext.h>
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE GL_VERTEX_ATTRIB_ARRAY_DIVISOR
#define GL_ETC1_RGB8_OES 0x8D64
#elif TARGET_OS_MAC
#include <GL/glew.h>
#include <GLES2/gl2platform.h>
#include <GLES2/gl2ext.h>
#else
#error "Unknown Apple platform"
#endif

#elif defined(__linux__)
#include <GL/glew.h>
#include <GLES2/gl2platform.h>
#include <GLES2/gl2ext.h>

#endif

#define UNPACK_FLIP_Y_WEBGL 0x9240
#define UNPACK_PREMULTIPLY_ALPHA_WEBGL 0x9241
#define CONTEXT_LOST_WEBGL 0x9242
#define UNPACK_COLORSPACE_CONVERSION_WEBGL 0x9243
#define BROWSER_DEFAULT_WEBGL 0x9244
#define MAX_CLIENT_WAIT_TIMEOUT_WEBGL ((uint32_t)2e7)

#include <defines.h>

#if !defined(LUMIN) && !defined(__ANDROID__)
#include <glfw/include/glfw.h>
#else
#include <egl/include/egl.h>
#endif

using namespace v8;
using namespace node;

enum GlKey {
  GL_KEY_COMPOSE,
  GL_KEY_PLANE,
};

#define GL_READ_BUFFER                                  0x0C02
#define GL_UNPACK_ROW_LENGTH                            0x0CF2
#define GL_UNPACK_SKIP_ROWS                             0x0CF3
#define GL_UNPACK_SKIP_PIXELS                           0x0CF4
#define GL_PACK_ROW_LENGTH                              0x0D02
#define GL_PACK_SKIP_ROWS                               0x0D03
#define GL_PACK_SKIP_PIXELS                             0x0D04
#define GL_COLOR                                        0x1800
#define GL_DEPTH                                        0x1801
#define GL_STENCIL                                      0x1802
#define GL_RED                                          0x1903
  //#define GL_RGB8                                         0x8051
  //#define GL_RGBA8                                        0x8058
  //#define GL_RGB10_A2                                     0x8059
#define GL_TEXTURE_BINDING_3D                           0x806A
#define GL_UNPACK_SKIP_IMAGES                           0x806D
#define GL_UNPACK_IMAGE_HEIGHT                          0x806E
#define GL_TEXTURE_3D                                   0x806F
#define GL_TEXTURE_WRAP_R                               0x8072
#define GL_MAX_3D_TEXTURE_SIZE                          0x8073
#define GL_UNSIGNED_INT_2_10_10_10_REV                  0x8368
#define GL_MAX_ELEMENTS_VERTICES                        0x80E8
#define GL_MAX_ELEMENTS_INDICES                         0x80E9
#define GL_TEXTURE_MIN_LOD                              0x813A
#define GL_TEXTURE_MAX_LOD                              0x813B
#define GL_TEXTURE_BASE_LEVEL                           0x813C
#define GL_TEXTURE_MAX_LEVEL                            0x813D
#define GL_MIN                                          0x8007
#define GL_MAX                                          0x8008
#define GL_DEPTH_COMPONENT24                            0x81A6
#define GL_MAX_TEXTURE_LOD_BIAS                         0x84FD
#define GL_TEXTURE_COMPARE_MODE                         0x884C
#define GL_TEXTURE_COMPARE_FUNC                         0x884D
#define GL_CURRENT_QUERY                                0x8865
#define GL_QUERY_RESULT                                 0x8866
#define GL_QUERY_RESULT_AVAILABLE                       0x8867
#define GL_STREAM_READ                                  0x88E1
#define GL_STREAM_COPY                                  0x88E2
#define GL_STATIC_READ                                  0x88E5
#define GL_STATIC_COPY                                  0x88E6
#define GL_DYNAMIC_READ                                 0x88E9
#define GL_DYNAMIC_COPY                                 0x88EA
  //#define GL_MAX_DRAW_BUFFERS                             0x8824
  //#define GL_DRAW_BUFFER0                                 0x8825
  //#define GL_DRAW_BUFFER1                                 0x8826
  //#define GL_DRAW_BUFFER2                                 0x8827
  //#define GL_DRAW_BUFFER3                                 0x8828
  //#define GL_DRAW_BUFFER4                                 0x8829
  //#define GL_DRAW_BUFFER5                                 0x882A
  //#define GL_DRAW_BUFFER6                                 0x882B
  //#define GL_DRAW_BUFFER7                                 0x882C
  //#define GL_DRAW_BUFFER8                                 0x882D
  //#define GL_DRAW_BUFFER9                                 0x882E
  //#define GL_DRAW_BUFFER10                                0x882F
  //#define GL_DRAW_BUFFER11                                0x8830
  //#define GL_DRAW_BUFFER12                                0x8831
  //#define GL_DRAW_BUFFER13                                0x8832
  //#define GL_DRAW_BUFFER14                                0x8833
  //#define GL_DRAW_BUFFER15                                0x8834
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS              0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                0x8B4A
#define GL_SAMPLER_3D                                   0x8B5F
#define GL_SAMPLER_2D_SHADOW                            0x8B62
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT              0x8B8B
  //#define GL_PIXEL_PACK_BUFFER                            0x88EB
  //#define GL_PIXEL_UNPACK_BUFFER                          0x88EC
  //#define GL_PIXEL_PACK_BUFFER_BINDING                    0x88ED
  //#define GL_PIXEL_UNPACK_BUFFER_BINDING                  0x88EF
#define GL_FLOAT_MAT2x3                                 0x8B65
#define GL_FLOAT_MAT2x4                                 0x8B66
#define GL_FLOAT_MAT3x2                                 0x8B67
#define GL_FLOAT_MAT3x4                                 0x8B68
#define GL_FLOAT_MAT4x2                                 0x8B69
#define GL_FLOAT_MAT4x3                                 0x8B6A
#define GL_SRGB                                         0x8C40
  //#define GL_SRGB8                                        0x8C41
#define GL_SRGB8_ALPHA8                                 0x8C43
#define GL_COMPARE_REF_TO_TEXTURE                       0x884E
  //#define GL_RGBA32F                                      0x8814
  //#define GL_RGB32F                                       0x8815
  //#define GL_RGBA16F                                      0x881A
  //#define GL_RGB16F                                       0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER                  0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS                     0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET                     0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET                     0x8905
#define GL_MAX_VARYING_COMPONENTS                       0x8B4B
#define GL_TEXTURE_2D_ARRAY                             0x8C1A
#define GL_TEXTURE_BINDING_2D_ARRAY                     0x8C1D
  //#define GL_R11F_G11F_B10F                               0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV                 0x8C3B
  //#define GL_RGB9_E5                                      0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV                     0x8C3E
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE               0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS   0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS                  0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START              0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE               0x8C85
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN        0x8C88
#define GL_RASTERIZER_DISCARD                           0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS      0x8C8B
#define GL_INTERLEAVED_ATTRIBS                          0x8C8C
#define GL_SEPARATE_ATTRIBS                             0x8C8D
  //#define GL_TRANSFORM_FEEDBACK_BUFFER                    0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING            0x8C8F
#define GL_RGBA32UI                                     0x8D70
#define GL_RGB32UI                                      0x8D71
#define GL_RGBA16UI                                     0x8D76
#define GL_RGB16UI                                      0x8D77
  //#define GL_RGBA8UI                                      0x8D7C
  //#define GL_RGB8UI                                       0x8D7D
#define GL_RGBA32I                                      0x8D82
#define GL_RGB32I                                       0x8D83
#define GL_RGBA16I                                      0x8D88
#define GL_RGB16I                                       0x8D89
#define GL_RGBA8I                                       0x8D8E
#define GL_RGB8I                                        0x8D8F
#define GL_RED_INTEGER                                  0x8D94
#define GL_RGB_INTEGER                                  0x8D98
#define GL_RGBA_INTEGER                                 0x8D99
#define GL_SAMPLER_2D_ARRAY                             0x8DC1
#define GL_SAMPLER_2D_ARRAY_SHADOW                      0x8DC4
#define GL_SAMPLER_CUBE_SHADOW                          0x8DC5
#define GL_UNSIGNED_INT_VEC2                            0x8DC6
#define GL_UNSIGNED_INT_VEC3                            0x8DC7
#define GL_UNSIGNED_INT_VEC4                            0x8DC8
#define GL_INT_SAMPLER_2D                               0x8DCA
#define GL_INT_SAMPLER_3D                               0x8DCB
#define GL_INT_SAMPLER_CUBE                             0x8DCC
#define GL_INT_SAMPLER_2D_ARRAY                         0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_2D                      0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D                      0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE                    0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                0x8DD7
#define GL_DEPTH_COMPONENT32F                           0x8CAC
#define GL_DEPTH32F_STENCIL8                            0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV               0x8DAD
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING        0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE        0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE              0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE            0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE             0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE            0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE            0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE          0x8217
#define GL_FRAMEBUFFER_DEFAULT                          0x8218
  //#define GL_DEPTH_STENCIL_ATTACHMENT                     0x821A
  //#define GL_DEPTH_STENCIL                                0x84F9
#define GL_UNSIGNED_INT_24_8                            0x84FA
  //#define GL_DEPTH24_STENCIL8                             0x88F0
#define GL_UNSIGNED_NORMALIZED                          0x8C17
  //#define GL_DRAW_FRAMEBUFFER_BINDING                     0x8CA6
  //#define GL_READ_FRAMEBUFFER                             0x8CA8
  //#define GL_DRAW_FRAMEBUFFER                             0x8CA9
  //#define GL_READ_FRAMEBUFFER_BINDING                     0x8CAA
#define GL_RENDERBUFFER_SAMPLES                         0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER         0x8CD4
  //#define GL_MAX_COLOR_ATTACHMENTS                        0x8CDF
  //#define GL_COLOR_ATTACHMENT1                            0x8CE1
  //#define GL_COLOR_ATTACHMENT2                            0x8CE2
  //#define GL_COLOR_ATTACHMENT3                            0x8CE3
  //#define GL_COLOR_ATTACHMENT4                            0x8CE4
  //#define GL_COLOR_ATTACHMENT5                            0x8CE5
  //#define GL_COLOR_ATTACHMENT6                            0x8CE6
  //#define GL_COLOR_ATTACHMENT7                            0x8CE7
  //#define GL_COLOR_ATTACHMENT8                            0x8CE8
  //#define GL_COLOR_ATTACHMENT9                            0x8CE9
  //#define GL_COLOR_ATTACHMENT10                           0x8CEA
  //#define GL_COLOR_ATTACHMENT11                           0x8CEB
  //#define GL_COLOR_ATTACHMENT12                           0x8CEC
  //#define GL_COLOR_ATTACHMENT13                           0x8CED
  //#define GL_COLOR_ATTACHMENT14                           0x8CEE
  //#define GL_COLOR_ATTACHMENT15                           0x8CEF
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE           0x8D56
#define GL_MAX_SAMPLES                                  0x8D57
  //#define GL_HALF_FLOAT                                   0x140B
#define GL_RG                                           0x8227
#define GL_RG_INTEGER                                   0x8228
  //#define GL_R8                                           0x8229
  //#define GL_RG8                                          0x822B
  //#define GL_R16F                                         0x822D
  //#define GL_R32F                                         0x822E
  //#define GL_RG16F                                        0x822F
  //#define GL_RG32F                                        0x8230
#define GL_R8I                                          0x8231
  //#define GL_R8UI                                         0x8232
#define GL_R16I                                         0x8233
#define GL_R16UI                                        0x8234
#define GL_R32I                                         0x8235
#define GL_R32UI                                        0x8236
#define GL_RG8I                                         0x8237
  //#define GL_RG8UI                                        0x8238
#define GL_RG16I                                        0x8239
  //#define GL_RG16UI                                       0x823A
#define GL_RG32I                                        0x823B
  //#define GL_RG32UI                                       0x823C
  //#define GL_VERTEX_ARRAY_BINDING                         0x85B5
#define GL_R8_SNORM                                     0x8F94
#define GL_RG8_SNORM                                    0x8F95
#define GL_RGB8_SNORM                                   0x8F96
#define GL_RGBA8_SNORM                                  0x8F97
#define GL_SIGNED_NORMALIZED                            0x8F9C
  //#define GL_COPY_READ_BUFFER                             0x8F36
  //#define GL_COPY_WRITE_BUFFER                            0x8F37
#define GL_COPY_READ_BUFFER_BINDING                     0x8F36
#define GL_COPY_WRITE_BUFFER_BINDING                    0x8F37
  //#define GL_UNIFORM_BUFFER                               0x8A11
#define GL_UNIFORM_BUFFER_BINDING                       0x8A28
#define GL_UNIFORM_BUFFER_START                         0x8A29
#define GL_UNIFORM_BUFFER_SIZE                          0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                    0x8A2B
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                  0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS                  0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS                  0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE                       0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS       0x8A31
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS     0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT              0x8A34
#define GL_ACTIVE_UNIFORM_BLOCKS                        0x8A36
#define GL_UNIFORM_TYPE                                 0x8A37
#define GL_UNIFORM_SIZE                                 0x8A38
#define GL_UNIFORM_BLOCK_INDEX                          0x8A3A
#define GL_UNIFORM_OFFSET                               0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE                         0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE                        0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR                         0x8A3E
#define GL_UNIFORM_BLOCK_BINDING                        0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE                      0x8A40
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS                0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES         0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER    0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER  0x8A46
#define GL_INVALID_INDEX                                0xFFFFFFFF
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS                 0x9122
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                0x9125
#define GL_MAX_SERVER_WAIT_TIMEOUT                      0x9111
  //#define GL_OBJECT_TYPE                                  0x9112
  //#define GL_SYNC_CONDITION                               0x9113
  //#define GL_SYNC_STATUS                                  0x9114
  //#define GL_SYNC_FLAGS                                   0x9115
  //#define GL_SYNC_FENCE                                   0x9116
  //#define GL_SYNC_GPU_COMMANDS_COMPLETE                   0x9117
  //#define GL_UNSIGNALED                                   0x9118
  //#define GL_SIGNALED                                     0x9119
  //#define GL_ALREADY_SIGNALED                             0x911A
  //#define GL_TIMEOUT_EXPIRED                              0x911B
  //#define GL_CONDITION_SATISFIED                          0x911C
  //#define GL_WAIT_FAILED                                  0x911D
  //#define GL_SYNC_FLUSH_COMMANDS_BIT                      0x00000001
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR                  0x88FE
#define GL_ANY_SAMPLES_PASSED                           0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE              0x8D6A
#define GL_SAMPLER_BINDING                              0x8919
#define GL_RGB10_A2UI                                   0x906F
#define GL_INT_2_10_10_10_REV                           0x8D9F
#define GL_TRANSFORM_FEEDBACK                           0x8E22
#define GL_TRANSFORM_FEEDBACK_PAUSED                    0x8E23
#define GL_TRANSFORM_FEEDBACK_ACTIVE                    0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING                   0x8E25
#define GL_TEXTURE_IMMUTABLE_FORMAT                     0x912F
#define GL_MAX_ELEMENT_INDEX                            0x8D6B
#define GL_TEXTURE_IMMUTABLE_LEVELS                     0x82DF

//  const GLint64 TIMEOUT_IGNORED                              = -1;

  /* WebGL-specific enums */
//  const GLenum MAX_CLIENT_WAIT_TIMEOUT_WEBGL                 = 0x9247;



void flipImageData(char *dstData, char *srcData, size_t width, size_t height, size_t pixelSize);

class ViewportState {
public:
  ViewportState(GLint x = 0, GLint y = 0, GLsizei w = 0, GLsizei h = 0, bool valid = false);
  ViewportState &operator=(const ViewportState &viewportState);

  GLint x;
  GLint y;
  GLsizei w;
  GLsizei h;
  bool valid;
};

class ColorMaskState {
public:
  ColorMaskState(GLboolean r = true, GLboolean g = true, GLboolean b = true, GLboolean a = true, bool valid = false);
  ColorMaskState &operator=(const ColorMaskState &colorMaskState);

  GLboolean r;
  GLboolean g;
  GLboolean b;
  GLboolean a;
  bool valid;
};

class WebGLRenderingContext : public ObjectWrap {
public:
  static std::pair<Local<Object>, Local<FunctionTemplate>> Initialize(Isolate *isolate);

  WebGLRenderingContext();
  ~WebGLRenderingContext();

  static NAN_METHOD(New);
  static NAN_METHOD(Destroy);
  static NAN_METHOD(GetWindowHandle);
  static NAN_METHOD(SetWindowHandle);
  static NAN_METHOD(SetDefaultVao);
  static NAN_METHOD(IsDirty);
  static NAN_METHOD(ClearDirty);

  static NAN_METHOD(Uniform1f);
  static NAN_METHOD(Uniform2f);
  static NAN_METHOD(Uniform3f);
  static NAN_METHOD(Uniform4f);
  static NAN_METHOD(Uniform1i);
  static NAN_METHOD(Uniform2i);
  static NAN_METHOD(Uniform3i);
  static NAN_METHOD(Uniform4i);
  static NAN_METHOD(Uniform1ui);
  static NAN_METHOD(Uniform2ui);
  static NAN_METHOD(Uniform3ui);
  static NAN_METHOD(Uniform4ui);
  static NAN_METHOD(Uniform1fv);
  static NAN_METHOD(Uniform2fv);
  static NAN_METHOD(Uniform3fv);
  static NAN_METHOD(Uniform4fv);
  static NAN_METHOD(Uniform1iv);
  static NAN_METHOD(Uniform2iv);
  static NAN_METHOD(Uniform3iv);
  static NAN_METHOD(Uniform4iv);

  static NAN_METHOD(Uniform1uiv);
  static NAN_METHOD(Uniform2uiv);
  static NAN_METHOD(Uniform3uiv);
  static NAN_METHOD(Uniform4uiv);
  static NAN_METHOD(UniformMatrix2fv);
  static NAN_METHOD(UniformMatrix3fv);
  static NAN_METHOD(UniformMatrix4fv);
  static NAN_METHOD(UniformMatrix3x2fv);
  static NAN_METHOD(UniformMatrix4x2fv);
  static NAN_METHOD(UniformMatrix2x3fv);
  static NAN_METHOD(UniformMatrix4x3fv);
  static NAN_METHOD(UniformMatrix2x4fv);
  static NAN_METHOD(UniformMatrix3x4fv);

  static NAN_METHOD(PixelStorei);
  static NAN_METHOD(BindAttribLocation);
  static NAN_METHOD(GetError);
  static NAN_METHOD(DrawArrays);
  static NAN_METHOD(DrawArraysInstanced);
  static NAN_METHOD(DrawArraysInstancedANGLE);
  static NAN_METHOD(GenerateMipmap);
  static NAN_METHOD(GetAttribLocation);
  static NAN_METHOD(DepthFunc);
  static NAN_METHOD(Viewport);
  static NAN_METHOD(CreateShader);
  static NAN_METHOD(ShaderSource);
  static NAN_METHOD(CompileShader);
  static NAN_METHOD(GetShaderParameter);
  static NAN_METHOD(GetShaderInfoLog);
  static NAN_METHOD(CreateProgram);
  static NAN_METHOD(AttachShader);
  static NAN_METHOD(LinkProgram);
  static NAN_METHOD(GetProgramParameter);
  static NAN_METHOD(GetUniformLocation);
  static NAN_METHOD(GetUniformBlockIndex);
  static NAN_METHOD(UniformBlockBinding);
  static NAN_METHOD(ClearColor);
  static NAN_METHOD(ClearDepth);
  static NAN_METHOD(Disable);
  static NAN_METHOD(Enable);
  static NAN_METHOD(CreateTexture);
  static NAN_METHOD(BindTexture);
  static NAN_METHOD(FlipTextureData);
  static NAN_METHOD(TexImage2D);
  static NAN_METHOD(CompressedTexImage2D);
  static NAN_METHOD(TexParameteri);
  static NAN_METHOD(TexParameterf);
  static NAN_METHOD(Clear);
  static NAN_METHOD(UseProgram);
  static NAN_METHOD(CreateBuffer);
  static NAN_METHOD(BindBuffer);
  static NAN_METHOD(BindBufferBase);
  static NAN_METHOD(CreateFramebuffer);
  static NAN_METHOD(BindFramebuffer);
  static NAN_METHOD(BindFramebufferRaw);
  static NAN_METHOD(FramebufferTexture2D);
  static NAN_METHOD(BlitFramebuffer);
  static NAN_METHOD(BufferData);
  static NAN_METHOD(BufferSubData);
  static NAN_METHOD(BlendEquation);
  static NAN_METHOD(BlendFunc);
  static NAN_METHOD(EnableVertexAttribArray);
  static NAN_METHOD(VertexAttribPointer);
  static NAN_METHOD(VertexAttribIPointer);
  static NAN_METHOD(ActiveTexture);
  static NAN_METHOD(DrawElements);
  static NAN_METHOD(DrawElementsInstanced);
  static NAN_METHOD(DrawElementsInstancedANGLE);
  static NAN_METHOD(DrawRangeElements);
  static NAN_METHOD(Flush);
  static NAN_METHOD(Finish);

  static NAN_METHOD(VertexAttrib1f);
  static NAN_METHOD(VertexAttrib2f);
  static NAN_METHOD(VertexAttrib3f);
  static NAN_METHOD(VertexAttrib4f);
  static NAN_METHOD(VertexAttrib1fv);
  static NAN_METHOD(VertexAttrib2fv);
  static NAN_METHOD(VertexAttrib3fv);
  static NAN_METHOD(VertexAttrib4fv);

  static NAN_METHOD(VertexAttribI4i);
  static NAN_METHOD(VertexAttribI4iv);
  static NAN_METHOD(VertexAttribI4ui);
  static NAN_METHOD(VertexAttribI4uiv);

  static NAN_METHOD(VertexAttribDivisor);
  static NAN_METHOD(VertexAttribDivisorANGLE);
  static NAN_METHOD(DrawBuffers);
  static NAN_METHOD(DrawBuffersWEBGL);

  static NAN_METHOD(BlendColor);
  static NAN_METHOD(BlendEquationSeparate);
  static NAN_METHOD(BlendFuncSeparate);
  static NAN_METHOD(ClearStencil);
  static NAN_METHOD(ColorMask);
  static NAN_METHOD(CopyTexImage2D);
  static NAN_METHOD(CopyTexSubImage2D);
  static NAN_METHOD(CullFace);
  static NAN_METHOD(DepthMask);
  static NAN_METHOD(DepthRange);
  static NAN_METHOD(DisableVertexAttribArray);
  static NAN_METHOD(Hint);
  static NAN_METHOD(IsEnabled);
  static NAN_METHOD(LineWidth);
  static NAN_METHOD(PolygonOffset);
  static NAN_METHOD(SampleCoverage);

  static NAN_METHOD(Scissor);
  static NAN_METHOD(StencilFunc);
  static NAN_METHOD(StencilFuncSeparate);
  static NAN_METHOD(StencilMask);
  static NAN_METHOD(StencilMaskSeparate);
  static NAN_METHOD(StencilOp);
  static NAN_METHOD(StencilOpSeparate);
  static NAN_METHOD(BindRenderbuffer);
  static NAN_METHOD(CreateRenderbuffer);

  static NAN_METHOD(DeleteBuffer);
  static NAN_METHOD(DeleteFramebuffer);
  static NAN_METHOD(DeleteProgram);
  static NAN_METHOD(DeleteRenderbuffer);
  static NAN_METHOD(DeleteShader);
  static NAN_METHOD(DeleteTexture);
  static NAN_METHOD(DetachShader);
  static NAN_METHOD(FramebufferRenderbuffer);
  static NAN_METHOD(GetVertexAttribOffset);
  static NAN_METHOD(GetShaderPrecisionFormat);

  static NAN_METHOD(IsBuffer);
  static NAN_METHOD(IsFramebuffer);
  static NAN_METHOD(IsProgram);
  static NAN_METHOD(IsRenderbuffer);
  static NAN_METHOD(IsShader);
  static NAN_METHOD(IsTexture);
  static NAN_METHOD(IsVertexArray);
  static NAN_METHOD(IsSync);

  static NAN_METHOD(RenderbufferStorage);
  static NAN_METHOD(GetShaderSource);
  static NAN_METHOD(ValidateProgram);

  static NAN_METHOD(TexSubImage2D);
  static NAN_METHOD(TexStorage2D);

  static NAN_METHOD(ReadPixels);
  static NAN_METHOD(GetTexParameter);
  static NAN_METHOD(GetActiveAttrib);
  static NAN_METHOD(GetActiveUniform);
  static NAN_METHOD(GetAttachedShaders);
  static NAN_METHOD(GetParameter);
  static NAN_METHOD(GetBufferParameter);
  static NAN_METHOD(GetFramebufferAttachmentParameter);
  static NAN_METHOD(GetProgramInfoLog);
  static NAN_METHOD(GetRenderbufferParameter);
  static NAN_METHOD(GetUniform);
  static NAN_METHOD(GetVertexAttrib);
  static NAN_METHOD(GetSupportedExtensions);
  static NAN_METHOD(GetExtension);
  static NAN_METHOD(CheckFramebufferStatus);

  static NAN_METHOD(CreateVertexArray);
  static NAN_METHOD(DeleteVertexArray);
  static NAN_METHOD(BindVertexArray);
  static NAN_METHOD(BindVertexArrayOES);

  static NAN_METHOD(FenceSync);
  static NAN_METHOD(DeleteSync);
  static NAN_METHOD(ClientWaitSync);
  static NAN_METHOD(WaitSync);
  static NAN_METHOD(GetSyncParameter);

  static NAN_METHOD(FrontFace);

  static NAN_METHOD(IsContextLost);

  static NAN_GETTER(DrawingBufferWidthGetter);
  static NAN_GETTER(DrawingBufferHeightGetter);

  static NAN_METHOD(GetFramebuffer);
  static NAN_METHOD(SetDefaultFramebuffer);

  void SetVertexArrayBinding(GLuint vao) {
    vertexArrayBindings[GL_VERTEX_SHADER] = vao;
  }
  GLuint GetVertexArrayBinding() {
    return vertexArrayBindings[GL_VERTEX_SHADER];
  }
  bool HasVertexArrayBinding() {
    return vertexArrayBindings.find(GL_VERTEX_SHADER) != vertexArrayBindings.end();
  }

  void SetFramebufferBinding(GLenum target, GLuint framebuffer) {
    framebufferBindings[target] = framebuffer;
  }
  GLuint GetFramebufferBinding(GLenum target) {
    return framebufferBindings[target];
  }
  bool HasFramebufferBinding(GLenum target) {
    return framebufferBindings.find(target) != framebufferBindings.end();
  }

  void SetRenderbufferBinding(GLenum target, GLuint renderbuffer) {
    renderbufferBindings[target] = renderbuffer;
  }
  GLuint GetRenderbufferBinding(GLenum target) {
    return renderbufferBindings[target];
  }
  bool HasRenderbufferBinding(GLenum target) {
    return renderbufferBindings.find(target) != renderbufferBindings.end();
  }

  void SetBufferBinding(GLenum target, GLuint buffer) {
    bufferBindings[target] = buffer;
  }
  GLuint GetBufferBinding(GLenum target) {
    return bufferBindings[target];
  }
  bool HasBufferBinding(GLenum target) {
    return bufferBindings.find(target) != bufferBindings.end();
  }

  void SetTextureBinding(GLenum framebuffer, GLenum target, GLuint texture) {
    textureBindings[std::make_pair(framebuffer, target)] = texture;
  }
  GLuint GetTextureBinding(GLenum framebuffer, GLenum target) {
    return textureBindings[std::make_pair(framebuffer, target)];
  }
  bool HasTextureBinding(GLenum framebuffer, GLenum target) {
    return textureBindings.find(std::make_pair(framebuffer, target)) != textureBindings.end();
  }

  void SetProgramBinding(GLuint program) {
    programBindings[GL_VERTEX_SHADER] = program;
  }
  GLuint GetProgramBinding() {
    return programBindings[GL_VERTEX_SHADER];
  }
  bool HasProgramBinding() {
    return programBindings.find(GL_VERTEX_SHADER) != programBindings.end();
  }

  bool live;
  NATIVEwindow *windowHandle;
  GLuint defaultVao;
  bool dirty;
  GLuint defaultFramebuffer;
  bool flipY;
  bool premultiplyAlpha;
  GLint packAlignment;
  GLint unpackAlignment;
  GLuint activeTexture;
  std::map<GLenum, GLuint> vertexArrayBindings;
  std::map<GLenum, GLuint> framebufferBindings;
  std::map<GLenum, GLuint> renderbufferBindings;
  std::map<GLenum, GLuint> bufferBindings;
  std::map<std::pair<GLenum, GLenum>, GLuint> textureBindings;
  std::map<GLenum, GLuint> programBindings;
  ViewportState viewportState;
  ColorMaskState colorMaskState;
  std::map<GlKey, void *> keys;
};

class WebGL2RenderingContext : public WebGLRenderingContext {
public:
  WebGL2RenderingContext();
  ~WebGL2RenderingContext();

  static std::pair<Local<Object>, Local<FunctionTemplate>> Initialize(Isolate *isolate, Local<FunctionTemplate> baseCtor);

  static NAN_METHOD(New);
  
  static NAN_METHOD(CreateQuery);
  static NAN_METHOD(BeginQuery);
  static NAN_METHOD(EndQuery);
  static NAN_METHOD(GetQuery);
  static NAN_METHOD(GetQueryParameter);
  static NAN_METHOD(IsQuery);
  static NAN_METHOD(DeleteQuery);
  
  static NAN_METHOD(CreateTransformFeedback);
  static NAN_METHOD(DeleteTransformFeedback);
  static NAN_METHOD(IsTransformFeedback);
  static NAN_METHOD(BindTransformFeedback);
  static NAN_METHOD(BeginTransformFeedback);
  static NAN_METHOD(EndTransformFeedback);
  static NAN_METHOD(TransformFeedbackVaryings);
  static NAN_METHOD(GetTransformFeedbackVarying);
  static NAN_METHOD(PauseTransformFeedback);
  static NAN_METHOD(ResumeTransformFeedback);
};

#endif
