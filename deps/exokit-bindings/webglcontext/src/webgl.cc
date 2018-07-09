#include <cstring>
#include <vector>

#include <webglcontext/include/webgl.h>
#include <canvascontext/include/imageData-context.h>
// #include <node.h>

/* #include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "glesjs", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "glesjs", __VA_ARGS__)) */

// using namespace node;
using namespace v8;
using namespace std;

// forward declarations
/* enum GLObjectType {
  GLOBJECT_TYPE_BUFFER,
  GLOBJECT_TYPE_FRAMEBUFFER,
  GLOBJECT_TYPE_PROGRAM,
  GLOBJECT_TYPE_RENDERBUFFER,
  GLOBJECT_TYPE_SHADER,
  GLOBJECT_TYPE_TEXTURE,
};

void registerGLObj(GLObjectType type, GLuint obj);
void unregisterGLObj(GLuint obj); */

// WebGLRenderingContext

// Used to be a macro, hence the uppercase name.
#define JS_GL_SET_CONSTANT(name, constant) proto->Set(JS_STR( name ), JS_INT(constant))

#define JS_GL_CONSTANT(name) JS_GL_SET_CONSTANT(#name, GL_ ## name)

template<NAN_METHOD(F)>
NAN_METHOD(glCallWrap) {
  Nan::HandleScope scope;

  Local<Object> glObj = info.This();
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);
  if (gl->live) {
    if (gl->windowHandle) {
      glfw::SetCurrentWindowContext(gl->windowHandle);
    }

    F(info);
  }
}
template<NAN_METHOD(F)>
NAN_METHOD(glSwitchCallWrap) {
  Nan::HandleScope scope;

  Local<Object> glObj = info.This();
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);
  if (gl->live) {
    if (gl->windowHandle) {
      glfw::SetCurrentWindowContext(gl->windowHandle);
    }

    F(info);
  }
}

template <typename T>
void setGlConstants(T &proto) {
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
  JS_GL_CONSTANT(COPY_READ_BUFFER);
  JS_GL_CONSTANT(COPY_WRITE_BUFFER);
  JS_GL_CONSTANT(TRANSFORM_FEEDBACK_BUFFER);
  JS_GL_CONSTANT(UNIFORM_BUFFER);
  JS_GL_CONSTANT(PIXEL_PACK_BUFFER);
  JS_GL_CONSTANT(PIXEL_UNPACK_BUFFER);

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
// #ifndef __APPLE__
  JS_GL_CONSTANT(FIXED);
// #endif

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
// #ifndef __APPLE__
  JS_GL_CONSTANT(MAX_VERTEX_UNIFORM_VECTORS);
  JS_GL_CONSTANT(MAX_VARYING_VECTORS);
// #endif
  JS_GL_CONSTANT(MAX_COMBINED_TEXTURE_IMAGE_UNITS);
  JS_GL_CONSTANT(MAX_VERTEX_TEXTURE_IMAGE_UNITS);
  JS_GL_CONSTANT(MAX_TEXTURE_IMAGE_UNITS);
// #ifndef __APPLE__
  JS_GL_CONSTANT(MAX_FRAGMENT_UNIFORM_VECTORS);
// #endif
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
// #ifndef __APPLE__
  JS_GL_CONSTANT(IMPLEMENTATION_COLOR_READ_TYPE);
  JS_GL_CONSTANT(IMPLEMENTATION_COLOR_READ_FORMAT);
// #endif

  /* Shader Source */
  JS_GL_CONSTANT(COMPILE_STATUS);
  JS_GL_CONSTANT(INFO_LOG_LENGTH);
  JS_GL_CONSTANT(SHADER_SOURCE_LENGTH);
// #ifndef __APPLE__
  JS_GL_CONSTANT(SHADER_COMPILER);
// #endif

  /* Shader Binary */
// #ifndef __APPLE__
  JS_GL_CONSTANT(SHADER_BINARY_FORMATS);
  JS_GL_CONSTANT(NUM_SHADER_BINARY_FORMATS);
// #endif

  /* Shader Precision-Specified Types */
// #ifndef __APPLE__
  JS_GL_CONSTANT(LOW_FLOAT);
  JS_GL_CONSTANT(MEDIUM_FLOAT);
  JS_GL_CONSTANT(HIGH_FLOAT);
  JS_GL_CONSTANT(LOW_INT);
  JS_GL_CONSTANT(MEDIUM_INT);
  JS_GL_CONSTANT(HIGH_INT);
// #endif

  /* Framebuffer Object. */
  JS_GL_CONSTANT(FRAMEBUFFER);
  JS_GL_CONSTANT(READ_FRAMEBUFFER);
  JS_GL_CONSTANT(DRAW_FRAMEBUFFER);
  JS_GL_CONSTANT(RENDERBUFFER);

  JS_GL_CONSTANT(RGBA4);
  JS_GL_CONSTANT(RGB5_A1);
// #ifndef __APPLE__
  //JS_GL_CONSTANT(RGB565);
// #endif
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
  JS_GL_CONSTANT(COLOR_ATTACHMENT1);
  JS_GL_CONSTANT(COLOR_ATTACHMENT2);
  JS_GL_CONSTANT(COLOR_ATTACHMENT3);
  JS_GL_CONSTANT(COLOR_ATTACHMENT4);
  JS_GL_CONSTANT(COLOR_ATTACHMENT5);
  JS_GL_CONSTANT(COLOR_ATTACHMENT6);
  JS_GL_CONSTANT(COLOR_ATTACHMENT7);
  JS_GL_CONSTANT(COLOR_ATTACHMENT8);
  JS_GL_CONSTANT(COLOR_ATTACHMENT9);
  JS_GL_CONSTANT(COLOR_ATTACHMENT10);
  JS_GL_CONSTANT(COLOR_ATTACHMENT11);
  JS_GL_CONSTANT(COLOR_ATTACHMENT12);
  JS_GL_CONSTANT(COLOR_ATTACHMENT13);
  JS_GL_CONSTANT(COLOR_ATTACHMENT14);
  JS_GL_CONSTANT(COLOR_ATTACHMENT15);
  JS_GL_CONSTANT(DEPTH_ATTACHMENT);
  JS_GL_CONSTANT(STENCIL_ATTACHMENT);
  JS_GL_CONSTANT(DEPTH_STENCIL_ATTACHMENT);
  JS_GL_CONSTANT(DRAW_BUFFER0);
  JS_GL_CONSTANT(DRAW_BUFFER1);
  JS_GL_CONSTANT(DRAW_BUFFER2);
  JS_GL_CONSTANT(DRAW_BUFFER3);
  JS_GL_CONSTANT(DRAW_BUFFER4);
  JS_GL_CONSTANT(DRAW_BUFFER5);
  JS_GL_CONSTANT(DRAW_BUFFER6);
  JS_GL_CONSTANT(DRAW_BUFFER7);
  JS_GL_CONSTANT(DRAW_BUFFER8);
  JS_GL_CONSTANT(DRAW_BUFFER9);
  JS_GL_CONSTANT(DRAW_BUFFER10);
  JS_GL_CONSTANT(DRAW_BUFFER11);
  JS_GL_CONSTANT(DRAW_BUFFER12);
  JS_GL_CONSTANT(DRAW_BUFFER13);
  JS_GL_CONSTANT(DRAW_BUFFER14);
  JS_GL_CONSTANT(DRAW_BUFFER15);
  JS_GL_CONSTANT(MAX_COLOR_ATTACHMENTS);
  JS_GL_CONSTANT(MAX_DRAW_BUFFERS);

  JS_GL_CONSTANT(NONE);

  JS_GL_CONSTANT(FRAMEBUFFER_COMPLETE);
  JS_GL_CONSTANT(FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
  JS_GL_CONSTANT(FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
// #ifndef __APPLE__
  //JS_GL_CONSTANT(FRAMEBUFFER_INCOMPLETE_DIMENSIONS);
// #endif
  JS_GL_CONSTANT(FRAMEBUFFER_UNSUPPORTED);

  JS_GL_CONSTANT(FRAMEBUFFER_BINDING);
  JS_GL_CONSTANT(RENDERBUFFER_BINDING);
  JS_GL_CONSTANT(MAX_RENDERBUFFER_SIZE);

  JS_GL_CONSTANT(INVALID_FRAMEBUFFER_OPERATION);

  /* Sync objects */
  JS_GL_CONSTANT(OBJECT_TYPE);
  JS_GL_CONSTANT(SYNC_FENCE);
  JS_GL_CONSTANT(SYNC_STATUS);
  JS_GL_CONSTANT(SIGNALED);
  JS_GL_CONSTANT(UNSIGNALED);
  JS_GL_CONSTANT(SYNC_CONDITION);
  JS_GL_CONSTANT(SYNC_GPU_COMMANDS_COMPLETE);
  JS_GL_CONSTANT(SYNC_FLAGS);
  JS_GL_CONSTANT(SYNC_FLUSH_COMMANDS_BIT);
  JS_GL_CONSTANT(ALREADY_SIGNALED);
  JS_GL_CONSTANT(TIMEOUT_EXPIRED);
  JS_GL_CONSTANT(CONDITION_SATISFIED);
  JS_GL_CONSTANT(WAIT_FAILED);
  GLint64 GL_TIMEOUT_IGNORED_TEMP_64 = GL_TIMEOUT_IGNORED;
  double GL_TIMEOUT_IGNORED_TEMP_DOUBLE = *(double *)(&GL_TIMEOUT_IGNORED_TEMP_64);
  proto->Set(JS_STR("TIMEOUT_IGNORED"), Nan::New<v8::Number>(GL_TIMEOUT_IGNORED_TEMP_DOUBLE));

  /* WebGL-specific enums */
  JS_GL_SET_CONSTANT("UNPACK_FLIP_Y_WEBGL", UNPACK_FLIP_Y_WEBGL);
  JS_GL_SET_CONSTANT("UNPACK_PREMULTIPLY_ALPHA_WEBGL", UNPACK_PREMULTIPLY_ALPHA_WEBGL);
  JS_GL_SET_CONSTANT("CONTEXT_LOST_WEBGL", CONTEXT_LOST_WEBGL);
  JS_GL_SET_CONSTANT("UNPACK_COLORSPACE_CONVERSION_WEBGL", UNPACK_COLORSPACE_CONVERSION_WEBGL);
  JS_GL_SET_CONSTANT("BROWSER_DEFAULT_WEBGL", BROWSER_DEFAULT_WEBGL);
  JS_GL_SET_CONSTANT("MAX_CLIENT_WAIT_TIMEOUT_WEBGL", MAX_CLIENT_WAIT_TIMEOUT_WEBGL);

  //////////////////////////////
  // NOT in WebGL spec
  //////////////////////////////

  // PBO
  JS_GL_SET_CONSTANT("PIXEL_PACK_BUFFER_BINDING" , 0x88ED);
  JS_GL_SET_CONSTANT("PIXEL_UNPACK_BUFFER_BINDING", 0x88EF);

  // external
  JS_GL_SET_CONSTANT("TEXTURE_EXTERNAL_OES", 0x8D65);
}

Handle<Object> WebGLRenderingContext::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(WebGLRenderingContext::New);

  s_ct.Reset(ctor);

  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("WebGLRenderingContext"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto, "destroy", Destroy);
  Nan::SetMethod(proto, "getWindowHandle", GetWindowHandle);
  Nan::SetMethod(proto, "setWindowHandle", SetWindowHandle);
  Nan::SetMethod(proto, "isDirty", IsDirty);
  Nan::SetMethod(proto, "clearDirty", ClearDirty);

  Nan::SetMethod(proto, "uniform1f", glCallWrap<Uniform1f>);
  Nan::SetMethod(proto, "uniform2f", glCallWrap<Uniform2f>);
  Nan::SetMethod(proto, "uniform3f", glCallWrap<Uniform3f>);
  Nan::SetMethod(proto, "uniform4f", glCallWrap<Uniform4f>);
  Nan::SetMethod(proto, "uniform1i", glCallWrap<Uniform1i>);
  Nan::SetMethod(proto, "uniform2i", glCallWrap<Uniform2i>);
  Nan::SetMethod(proto, "uniform3i", glCallWrap<Uniform3i>);
  Nan::SetMethod(proto, "uniform4i", glCallWrap<Uniform4i>);
  Nan::SetMethod(proto, "uniform1ui", glCallWrap<Uniform1ui>);
  Nan::SetMethod(proto, "uniform2ui", glCallWrap<Uniform2ui>);
  Nan::SetMethod(proto, "uniform3ui", glCallWrap<Uniform3ui>);
  Nan::SetMethod(proto, "uniform4ui", glCallWrap<Uniform4ui>);
  Nan::SetMethod(proto, "uniform1fv", glCallWrap<Uniform1fv>);
  Nan::SetMethod(proto, "uniform2fv", glCallWrap<Uniform2fv>);
  Nan::SetMethod(proto, "uniform3fv", glCallWrap<Uniform3fv>);
  Nan::SetMethod(proto, "uniform4fv", glCallWrap<Uniform4fv>);
  Nan::SetMethod(proto, "uniform1iv", glCallWrap<Uniform1iv>);
  Nan::SetMethod(proto, "uniform2iv", glCallWrap<Uniform2iv>);
  Nan::SetMethod(proto, "uniform3iv", glCallWrap<Uniform3iv>);
  Nan::SetMethod(proto, "uniform4iv", glCallWrap<Uniform4iv>);

  Nan::SetMethod(proto, "uniform1uiv", glCallWrap<Uniform1uiv>);
  Nan::SetMethod(proto, "uniform2uiv", glCallWrap<Uniform2uiv>);
  Nan::SetMethod(proto, "uniform3uiv", glCallWrap<Uniform3uiv>);
  Nan::SetMethod(proto, "uniform4uiv", glCallWrap<Uniform4uiv>);
  Nan::SetMethod(proto, "uniformMatrix2fv", glCallWrap<UniformMatrix2fv>);
  Nan::SetMethod(proto, "uniformMatrix3fv", glCallWrap<UniformMatrix3fv>);
  Nan::SetMethod(proto, "uniformMatrix4fv", glCallWrap<UniformMatrix4fv>);
  Nan::SetMethod(proto, "uniformMatrix3x2fv", glCallWrap<UniformMatrix3x2fv>);
  Nan::SetMethod(proto, "uniformMatrix4x2fv", glCallWrap<UniformMatrix4x2fv>);
  Nan::SetMethod(proto, "uniformMatrix2x3fv", glCallWrap<UniformMatrix2x3fv>);
  Nan::SetMethod(proto, "uniformMatrix4x3fv", glCallWrap<UniformMatrix4x3fv>);
  Nan::SetMethod(proto, "uniformMatrix2x4fv", glCallWrap<UniformMatrix2x4fv>);
  Nan::SetMethod(proto, "uniformMatrix3x4fv", glCallWrap<UniformMatrix3x4fv>);

  Nan::SetMethod(proto, "pixelStorei", glCallWrap<PixelStorei>);
  Nan::SetMethod(proto, "bindAttribLocation", glCallWrap<BindAttribLocation>);
  Nan::SetMethod(proto, "getError", glCallWrap<GetError>);
  Nan::SetMethod(proto, "drawArrays", glCallWrap<DrawArrays>);
  Nan::SetMethod(proto, "drawArraysInstanced", glCallWrap<DrawArraysInstanced>);

  Nan::SetMethod(proto, "generateMipmap", glCallWrap<GenerateMipmap>);

  Nan::SetMethod(proto, "getAttribLocation", glCallWrap<GetAttribLocation>);
  Nan::SetMethod(proto, "depthFunc", glCallWrap<DepthFunc>);
  Nan::SetMethod(proto, "viewport", glCallWrap<Viewport>);
  Nan::SetMethod(proto, "createShader", glCallWrap<CreateShader>);
  Nan::SetMethod(proto, "shaderSource", glCallWrap<ShaderSource>);
  Nan::SetMethod(proto, "compileShader", glCallWrap<CompileShader>);
  Nan::SetMethod(proto, "getShaderParameter", glCallWrap<GetShaderParameter>);
  Nan::SetMethod(proto, "getShaderInfoLog", glCallWrap<GetShaderInfoLog>);
  Nan::SetMethod(proto, "createProgram", glCallWrap<CreateProgram>);
  Nan::SetMethod(proto, "attachShader", glCallWrap<AttachShader>);
  Nan::SetMethod(proto, "linkProgram", glCallWrap<LinkProgram>);
  Nan::SetMethod(proto, "getProgramParameter", glCallWrap<GetProgramParameter>);
  Nan::SetMethod(proto, "getUniformLocation", glCallWrap<GetUniformLocation>);
  Nan::SetMethod(proto, "getUniform", glCallWrap<GetUniform>);
  Nan::SetMethod(proto, "clearColor", glCallWrap<ClearColor>);
  Nan::SetMethod(proto, "clearDepth", glCallWrap<ClearDepth>);

  Nan::SetMethod(proto, "disable", glCallWrap<Disable>);
  Nan::SetMethod(proto, "createTexture", glCallWrap<CreateTexture>);
  Nan::SetMethod(proto, "bindTexture", glCallWrap<BindTexture>);
  // Nan::SetMethod(proto, "flipTextureData", glCallWrap<FlipTextureData>);
  Nan::SetMethod(proto, "texImage2D", glCallWrap<TexImage2D>);
  Nan::SetMethod(proto, "compressedTexImage2D", glCallWrap<CompressedTexImage2D>);
  Nan::SetMethod(proto, "texParameteri", glCallWrap<TexParameteri>);
  Nan::SetMethod(proto, "texParameterf", glCallWrap<TexParameterf>);
  Nan::SetMethod(proto, "clear", glCallWrap<Clear>);
  Nan::SetMethod(proto, "useProgram", glCallWrap<UseProgram>);
  Nan::SetMethod(proto, "createFramebuffer", glCallWrap<CreateFramebuffer>);
  Nan::SetMethod(proto, "bindFramebuffer", glCallWrap<BindFramebuffer>);
  Nan::SetMethod(proto, "framebufferTexture2D", glCallWrap<FramebufferTexture2D>);
  Nan::SetMethod(proto, "blitFramebuffer", glCallWrap<BlitFramebuffer>);
  Nan::SetMethod(proto, "createBuffer", glCallWrap<CreateBuffer>);
  Nan::SetMethod(proto, "bindBuffer", glCallWrap<BindBuffer>);
  Nan::SetMethod(proto, "bufferData", glCallWrap<BufferData>);
  Nan::SetMethod(proto, "bufferSubData", glCallWrap<BufferSubData>);
  Nan::SetMethod(proto, "enable", glCallWrap<Enable>);
  Nan::SetMethod(proto, "blendEquation", glCallWrap<BlendEquation>);
  Nan::SetMethod(proto, "blendFunc", glCallWrap<BlendFunc>);
  Nan::SetMethod(proto, "enableVertexAttribArray", glCallWrap<EnableVertexAttribArray>);
  Nan::SetMethod(proto, "vertexAttribPointer", glCallWrap<VertexAttribPointer>);
  Nan::SetMethod(proto, "vertexAttribIPointer", glCallWrap<VertexAttribIPointer>);
  Nan::SetMethod(proto, "activeTexture", glCallWrap<ActiveTexture>);
  Nan::SetMethod(proto, "drawElements", glCallWrap<DrawElements>);
  Nan::SetMethod(proto, "drawElementsInstanced", glCallWrap<DrawElementsInstanced>);
  Nan::SetMethod(proto, "drawRangeElements", glCallWrap<DrawRangeElements>);
  Nan::SetMethod(proto, "flush", glCallWrap<Flush>);
  Nan::SetMethod(proto, "finish", glCallWrap<Finish>);

  Nan::SetMethod(proto, "vertexAttrib1f", glCallWrap<VertexAttrib1f>);
  Nan::SetMethod(proto, "vertexAttrib2f", glCallWrap<VertexAttrib2f>);
  Nan::SetMethod(proto, "vertexAttrib3f", glCallWrap<VertexAttrib3f>);
  Nan::SetMethod(proto, "vertexAttrib4f", glCallWrap<VertexAttrib4f>);
  Nan::SetMethod(proto, "vertexAttrib1fv", glCallWrap<VertexAttrib1fv>);
  Nan::SetMethod(proto, "vertexAttrib2fv", glCallWrap<VertexAttrib2fv>);
  Nan::SetMethod(proto, "vertexAttrib3fv", glCallWrap<VertexAttrib3fv>);
  Nan::SetMethod(proto, "vertexAttrib4fv", glCallWrap<VertexAttrib4fv>);

  Nan::SetMethod(proto, "vertexAttribI4i", glCallWrap<VertexAttribI4i>);
  Nan::SetMethod(proto, "vertexAttribI4iv", glCallWrap<VertexAttribI4iv>);
  Nan::SetMethod(proto, "vertexAttribI4ui", glCallWrap<VertexAttribI4ui>);
  Nan::SetMethod(proto, "vertexAttribI4uiv", glCallWrap<VertexAttribI4uiv>);
  
  Nan::SetMethod(proto, "vertexAttribDivisor", glCallWrap<VertexAttribDivisor>);
  Nan::SetMethod(proto, "drawBuffers", glCallWrap<DrawBuffers>);

  Nan::SetMethod(proto, "blendColor", BlendColor);
  Nan::SetMethod(proto, "blendEquationSeparate", BlendEquationSeparate);
  Nan::SetMethod(proto, "blendFuncSeparate", BlendFuncSeparate);
  Nan::SetMethod(proto, "clearStencil", ClearStencil);
  Nan::SetMethod(proto, "colorMask", ColorMask);
  Nan::SetMethod(proto, "copyTexImage2D", CopyTexImage2D);
  Nan::SetMethod(proto, "copyTexSubImage2D", CopyTexSubImage2D);
  Nan::SetMethod(proto, "cullFace", CullFace);
  Nan::SetMethod(proto, "depthMask", DepthMask);
  Nan::SetMethod(proto, "depthRange", DepthRange);
  Nan::SetMethod(proto, "disableVertexAttribArray", DisableVertexAttribArray);
  Nan::SetMethod(proto, "hint", Hint);
  Nan::SetMethod(proto, "isEnabled", IsEnabled);
  Nan::SetMethod(proto, "lineWidth", LineWidth);
  Nan::SetMethod(proto, "polygonOffset", PolygonOffset);

  Nan::SetMethod(proto, "scissor", Scissor);
  Nan::SetMethod(proto, "stencilFunc", StencilFunc);
  Nan::SetMethod(proto, "stencilFuncSeparate", StencilFuncSeparate);
  Nan::SetMethod(proto, "stencilMask", StencilMask);
  Nan::SetMethod(proto, "stencilMaskSeparate", StencilMaskSeparate);
  Nan::SetMethod(proto, "stencilOp", StencilOp);
  Nan::SetMethod(proto, "stencilOpSeparate", StencilOpSeparate);
  Nan::SetMethod(proto, "bindRenderbuffer", BindRenderbuffer);
  Nan::SetMethod(proto, "createRenderbuffer", CreateRenderbuffer);

  Nan::SetMethod(proto, "deleteBuffer", DeleteBuffer);
  Nan::SetMethod(proto, "deleteFramebuffer", DeleteFramebuffer);
  Nan::SetMethod(proto, "deleteProgram", DeleteProgram);
  Nan::SetMethod(proto, "deleteRenderbuffer", DeleteRenderbuffer);
  Nan::SetMethod(proto, "deleteShader", DeleteShader);
  Nan::SetMethod(proto, "deleteTexture", DeleteTexture);
  Nan::SetMethod(proto, "detachShader", DetachShader);
  Nan::SetMethod(proto, "framebufferRenderbuffer", FramebufferRenderbuffer);
  Nan::SetMethod(proto, "getVertexAttribOffset", GetVertexAttribOffset);
  Nan::SetMethod(proto, "getShaderPrecisionFormat", GetShaderPrecisionFormat);

  Nan::SetMethod(proto, "isBuffer", glCallWrap<IsBuffer>);
  Nan::SetMethod(proto, "isFramebuffer", glCallWrap<IsFramebuffer>);
  Nan::SetMethod(proto, "isProgram", glCallWrap<IsProgram>);
  Nan::SetMethod(proto, "isRenderbuffer", glCallWrap<IsRenderbuffer>);
  Nan::SetMethod(proto, "isShader", glCallWrap<IsShader>);
  Nan::SetMethod(proto, "isTexture", glCallWrap<IsTexture>);
  Nan::SetMethod(proto, "isVertexArray", glCallWrap<IsVertexArray>);
  Nan::SetMethod(proto, "isSync", glCallWrap<IsSync>);

  Nan::SetMethod(proto, "renderbufferStorage", glCallWrap<RenderbufferStorage>);
  Nan::SetMethod(proto, "getShaderSource", glCallWrap<GetShaderSource>);
  Nan::SetMethod(proto, "validateProgram", glCallWrap<ValidateProgram>);

  Nan::SetMethod(proto, "texSubImage2D", glCallWrap<TexSubImage2D>);
  Nan::SetMethod(proto, "texStorage2D", glCallWrap<TexStorage2D>);

  Nan::SetMethod(proto, "readPixels", glCallWrap<ReadPixels>);
  Nan::SetMethod(proto, "getTexParameter", glCallWrap<GetTexParameter>);
  Nan::SetMethod(proto, "getActiveAttrib", glCallWrap<GetActiveAttrib>);
  Nan::SetMethod(proto, "getActiveUniform", glCallWrap<GetActiveUniform>);
  Nan::SetMethod(proto, "getAttachedShaders", glCallWrap<GetAttachedShaders>);
  Nan::SetMethod(proto, "getParameter", glCallWrap<GetParameter>);
  Nan::SetMethod(proto, "getBufferParameter", glCallWrap<GetBufferParameter>);
  Nan::SetMethod(proto, "getFramebufferAttachmentParameter", glCallWrap<GetFramebufferAttachmentParameter>);
  Nan::SetMethod(proto, "getProgramInfoLog", glCallWrap<GetProgramInfoLog>);
  Nan::SetMethod(proto, "getRenderbufferParameter", glCallWrap<GetRenderbufferParameter>);
  Nan::SetMethod(proto, "getVertexAttrib", glCallWrap<GetVertexAttrib>);
  Nan::SetMethod(proto, "getShaderPrecisionFormat", glCallWrap<GetShaderPrecisionFormat>);
  Nan::SetMethod(proto, "getSupportedExtensions", glCallWrap<GetSupportedExtensions>);
  Nan::SetMethod(proto, "getExtension", glCallWrap<GetExtension>);
  Nan::SetMethod(proto, "checkFramebufferStatus", glCallWrap<CheckFramebufferStatus>);

  Nan::SetMethod(proto, "createVertexArray", glCallWrap<CreateVertexArray>);
  Nan::SetMethod(proto, "deleteVertexArray", glCallWrap<DeleteVertexArray>);
  Nan::SetMethod(proto, "bindVertexArray", glCallWrap<BindVertexArray>);

  Nan::SetMethod(proto, "fenceSync", glCallWrap<FenceSync>);
  Nan::SetMethod(proto, "deleteSync", glCallWrap<DeleteSync>);
  Nan::SetMethod(proto, "clientWaitSync", glCallWrap<ClientWaitSync>);
  Nan::SetMethod(proto, "waitSync", glCallWrap<WaitSync>);
  Nan::SetMethod(proto, "getSyncParameter", glCallWrap<GetSyncParameter>);

  Nan::SetMethod(proto, "frontFace", glCallWrap<FrontFace>);

  Nan::SetMethod(proto, "isContextLost", glCallWrap<IsContextLost>);

  Nan::SetAccessor(proto, JS_STR("drawingBufferWidth"), DrawingBufferWidthGetter);
  Nan::SetAccessor(proto, JS_STR("drawingBufferHeight"), DrawingBufferHeightGetter);

  Nan::SetMethod(proto, "setDefaultFramebuffer", glSwitchCallWrap<SetDefaultFramebuffer>);

  setGlConstants(proto);

  // ctor
  Local<Function> ctorFn = ctor->GetFunction();
  setGlConstants(ctorFn);

  return scope.Escape(ctorFn);
}

WebGLRenderingContext::WebGLRenderingContext() : live(true), windowHandle(nullptr), dirty(false), defaultFramebuffer(0), flipY(true), premultiplyAlpha(true), packAlignment(4), unpackAlignment(4), activeTexture(GL_TEXTURE0) {}

WebGLRenderingContext::~WebGLRenderingContext() {}

NAN_METHOD(WebGLRenderingContext::New) {
  WebGLRenderingContext *gl = new WebGLRenderingContext();
  Local<Object> glObj = info.This();
  gl->Wrap(glObj);

  info.GetReturnValue().Set(glObj);
}

NAN_METHOD(WebGLRenderingContext::Destroy) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  gl->live = false;
}

NAN_METHOD(WebGLRenderingContext::GetWindowHandle) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  if (gl->windowHandle) {
    info.GetReturnValue().Set(pointerToArray(gl->windowHandle));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(WebGLRenderingContext::SetWindowHandle) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  if (info[0]->IsArray()) {
    gl->windowHandle = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  } else {
    gl->windowHandle = nullptr;
  }
}

NAN_METHOD(WebGLRenderingContext::IsDirty) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  info.GetReturnValue().Set(JS_BOOL(gl->dirty));
}

NAN_METHOD(WebGLRenderingContext::ClearDirty) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  gl->dirty = false;
}

// GL CALLS

// A 32-bit and 64-bit compatible way of converting a pointer to a GLuint.
static GLuint ToGLuint(const void* ptr) {
  return static_cast<GLuint>(reinterpret_cast<size_t>(ptr));
}

template<typename Type>
inline Type* getArrayData(Local<Value> arg, int* num = NULL) {
  Type *data=NULL;
  if (num) {
    *num = 0;
  }

  if (!arg->IsNull()) {
    if (arg->IsArrayBufferView()) {
      Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(arg);
      if (num) {
        *num = arr->ByteLength()/sizeof(Type);
      }
      data = reinterpret_cast<Type*>((char *)arr->Buffer()->GetContents().Data() + arr->ByteOffset());
    } else {
      Nan::ThrowError("Bad array argument");
    }
  }

  return data;
}

inline void *getImageData(Local<Value> arg, int *num = nullptr) {
  void *pixels = nullptr;

  if (!arg->IsNull()) {
    Local<Object> obj = Local<Object>::Cast(arg);
    if (obj->IsObject()) {
      if (obj->IsArrayBufferView()) {
        pixels = getArrayData<unsigned char>(obj, num);
      } else {
        Local<String> dataString = String::NewFromUtf8(Isolate::GetCurrent(), "data", NewStringType::kInternalized).ToLocalChecked();
        if (obj->Has(dataString)) {
          Local<Value> data = obj->Get(dataString);
          pixels = getArrayData<unsigned char>(data, num);
        } else {
          Nan::ThrowError("Bad texture argument");
          // pixels = node::Buffer::Data(Nan::Get(obj, JS_STR("data")).ToLocalChecked());
        }
      }
    } else {
      Nan::ThrowError("Bad texture argument");
    }
  }
  return pixels;
}

inline void reformatImageData(char *dstData, char *srcData, size_t dstPixelSize, size_t srcPixelSize, size_t numPixels) {
  if (dstPixelSize < srcPixelSize) {
    // size_t clipSize = srcPixelSize - dstPixelSize;
    for (size_t i = 0; i < numPixels; i++) {
      memcpy(dstData + i * dstPixelSize, srcData + i * srcPixelSize, dstPixelSize);
      /* for (size_t j = 0; j < dstPixelSize; j++) {
        dstData[i * dstPixelSize + j] = srcData[i * srcPixelSize + srcPixelSize - clipSize - 1 - j];
      } */
      // memcpy(dstData + i * dstPixelSize, srcData + i * srcPixelSize + clipSize, dstPixelSize);
    }
  } else if (dstPixelSize > srcPixelSize) {
    size_t clipSize = dstPixelSize - srcPixelSize;
    for (size_t i = 0; i < numPixels; i++) {
      memcpy(dstData + i * dstPixelSize, srcData + i * srcPixelSize, srcPixelSize);
      memset(dstData + i * dstPixelSize + srcPixelSize, 0xFF, clipSize);
    }
  } else {
    // the call was pointless, but fulfill the contract anyway
    memcpy(dstData, srcData, dstPixelSize * numPixels);
  }
}

void flipImageData(char *dstData, char *srcData, size_t width, size_t height, size_t pixelSize) {
  size_t stride = width * pixelSize;
  size_t size = width * height * pixelSize;
  for (size_t i = 0; i < height; i++) {
    memcpy(dstData + (i * stride), srcData + size - stride - (i * stride), stride);
  }
}

template <typename T>
void expandLuminance(char *dstData, char *srcData, size_t width, size_t height) {
  size_t size = width * height;
  for (size_t i = 0; i < size; i++) {
    T value = ((T *)srcData)[i];
    ((T *)dstData)[i * 4 + 0] = value;
    ((T *)dstData)[i * 4 + 1] = value;
    ((T *)dstData)[i * 4 + 2] = value;
    ((T *)dstData)[i * 4 + 3] = value;
  }
}

template <typename T>
void expandLuminanceAlpha(char *dstData, char *srcData, size_t width, size_t height) {
  size_t size = width * height;
  for (size_t i = 0; i < size; i++) {
    T luminance = ((T *)srcData)[i*2 + 0];
    T alpha = ((T *)srcData)[i*2 + 1];
    ((T *)dstData)[i * 4 + 0] = luminance;
    ((T *)dstData)[i * 4 + 1] = luminance;
    ((T *)dstData)[i * 4 + 2] = luminance;
    ((T *)dstData)[i * 4 + 3] = alpha;
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform1f) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    float x = (float)info[1]->NumberValue();

    glUniform1f(location, x);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform2f) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    float x = (float)info[1]->NumberValue();
    float y = (float)info[2]->NumberValue();

    glUniform2f(location, x, y);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform3f) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    float x = (float)info[1]->NumberValue();
    float y = (float)info[2]->NumberValue();
    float z = (float)info[3]->NumberValue();

    glUniform3f(location, x, y, z);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform4f) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    float x = (float)info[1]->NumberValue();
    float y = (float)info[2]->NumberValue();
    float z = (float)info[3]->NumberValue();
    float w = (float)info[4]->NumberValue();

    glUniform4f(location, x, y, z, w);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform1i) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    GLint x = info[1]->Int32Value();

    glUniform1i(location, x);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform2i) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    GLint x = info[1]->Int32Value();
    GLint y = info[2]->Int32Value();

    glUniform2i(location, x, y);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform3i) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    GLint x = info[1]->Int32Value();
    GLint y = info[2]->Int32Value();
    GLint z = info[3]->Int32Value();

    glUniform3i(location, x, y, z);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform4i) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    GLint x = info[1]->Int32Value();
    GLint y = info[2]->Int32Value();
    GLint z = info[3]->Int32Value();
    GLint w = info[4]->Int32Value();

    glUniform4i(location, x, y, z, w);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform1ui) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    GLuint x = info[1]->Uint32Value();

    glUniform1ui(location, x);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform2ui) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    GLuint x = info[1]->Uint32Value();
    GLuint y = info[2]->Uint32Value();

    glUniform2ui(location, x, y);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform3ui) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    GLuint x = info[1]->Uint32Value();
    GLuint y = info[2]->Uint32Value();
    GLuint z = info[3]->Uint32Value();

    glUniform3ui(location, x, y, z);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform4ui) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    GLuint x = info[1]->Uint32Value();
    GLuint y = info[2]->Uint32Value();
    GLuint z = info[3]->Uint32Value();
    GLuint w = info[4]->Uint32Value();

    glUniform4ui(location, x, y, z, w);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform1fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    int num;
    if (info[1]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[1]);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      GLfloat *ptr = getArrayData<GLfloat>(float32Array, &num);
      glUniform1fv(location, num, ptr);
    } else {
      GLfloat *ptr = getArrayData<GLfloat>(info[1], &num);
      glUniform1fv(location, num, ptr);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform2fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    int num;
    if (info[1]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[1]);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      GLfloat *ptr = getArrayData<GLfloat>(float32Array, &num);
      num /= 2;
      glUniform2fv(location, num, ptr);
    } else {
      GLfloat *ptr = getArrayData<GLfloat>(info[1], &num);
      num /= 2;
      glUniform2fv(location, num, ptr);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform3fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    int num;
    if (info[1]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[1]);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      GLfloat *ptr = getArrayData<GLfloat>(float32Array, &num);
      num /= 3;
      glUniform3fv(location, num, ptr);
    } else {
      GLfloat *ptr = getArrayData<GLfloat>(info[1], &num);
      num /= 3;
      glUniform3fv(location, num, ptr);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform4fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    int num;
    if (info[1]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[1]);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      GLfloat *ptr=getArrayData<GLfloat>(float32Array, &num);
      num /= 4;
      glUniform4fv(location, num, ptr);
    } else {
      GLfloat *ptr=getArrayData<GLfloat>(info[1], &num);
      num /= 4;
      glUniform4fv(location, num, ptr);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform1iv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    int num;
    if (info[1]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[1]);
      unsigned int length = array->Length();
      Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        int32Array->Set(i, array->Get(i));
      }
      GLint *ptr = getArrayData<GLint>(int32Array, &num);
      glUniform1iv(location, num, ptr);
    } else {
      GLint *ptr = getArrayData<GLint>(info[1], &num);
      glUniform1iv(location, num, ptr);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform2iv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    int num;
    if (info[1]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[1]);
      unsigned int length = array->Length();
      Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        int32Array->Set(i, array->Get(i));
      }
      GLint *ptr = getArrayData<GLint>(int32Array, &num);
      num /= 2;
      glUniform2iv(location, num, ptr);
    } else {
      GLint *ptr = getArrayData<GLint>(info[1], &num);
      num /= 2;
      glUniform2iv(location, num, ptr);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform3iv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    int num;
    if (info[1]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[1]);
      unsigned int length = array->Length();
      Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        int32Array->Set(i, array->Get(i));
      }
      GLint *ptr = getArrayData<GLint>(int32Array, &num);
      num /= 3;
      glUniform3iv(location, num, ptr);
    } else {
      GLint *ptr=getArrayData<GLint>(info[1], &num);
      num /= 3;
      glUniform3iv(location, num, ptr);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform4iv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    int num;
    if (info[1]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[1]);
      unsigned int length = array->Length();
      Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        int32Array->Set(i, array->Get(i));
      }
      GLint *ptr = getArrayData<GLint>(int32Array, &num);
      num /= 4;
      glUniform4iv(location, num, ptr);
    } else {
      GLint *ptr = getArrayData<GLint>(info[1], &num);
      num /= 4;
      glUniform4iv(location, num, ptr);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform1uiv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    Local<Value> dataValue = info[1];

    GLuint *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Uint32Array> uint32Array = Uint32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        uint32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLuint>(uint32Array, &count);
    } else {
      data = getArrayData<GLuint>(dataValue, &count);
    }

    glUniform1uiv(location, count, data);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform2uiv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    Local<Value> dataValue = info[1];

    GLuint *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Uint32Array> uint32Array = Uint32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        uint32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLuint>(uint32Array, &count);
    } else {
      data = getArrayData<GLuint>(dataValue, &count);
    }

    glUniform2uiv(location, count, data);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform3uiv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    Local<Value> dataValue = info[1];

    GLuint *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Uint32Array> uint32Array = Uint32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        uint32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLuint>(uint32Array, &count);
    } else {
      data = getArrayData<GLuint>(dataValue, &count);
    }

    glUniform3uiv(location, count, data);
  }
}

NAN_METHOD(WebGLRenderingContext::Uniform4uiv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    Local<Value> dataValue = info[1];

    GLuint *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Uint32Array> uint32Array = Uint32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        uint32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLuint>(uint32Array, &count);
    } else {
      data = getArrayData<GLuint>(dataValue, &count);
    }

    glUniform4uiv(location, count, data);
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix2fv) {
  GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
  GLboolean transpose = info[1]->BooleanValue();

  GLfloat *data;
  GLsizei count;
  // GLfloat* data=getArrayData<GLfloat>(info[2],&count);

  if (info[2]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[2]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &count);
  } else {
    data = getArrayData<GLfloat>(info[2], &count);
  }

  if (count < 4) {
    Nan::ThrowError("Not enough data for UniformMatrix2fv");
  } else {
    count /= 4;
    glUniformMatrix2fv(location, count, transpose, data);

    // info.GetReturnValue().Set(Nan::Undefined());
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix3fv) {
  GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
  GLboolean transpose = info[1]->BooleanValue();

  GLfloat *data;
  GLsizei count;
  if (info[2]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[2]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &count);
  } else {
    data = getArrayData<GLfloat>(info[2], &count);
  }

  if (count < 9) {
    Nan::ThrowError("Not enough data for UniformMatrix3fv");
  }else{
    count /= 9;
    glUniformMatrix3fv(location, count, transpose, data);
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix4fv) {
  GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
  GLboolean transpose = info[1]->BooleanValue();

  GLfloat *data;
  GLsizei count;
  if (info[2]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[2]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &count);
  } else {
    data = getArrayData<GLfloat>(info[2], &count);
  }

  if (count < 16) {
    Nan::ThrowError("Not enough data for UniformMatrix4fv");
  } else {
    count /= 16;
    glUniformMatrix4fv(location, count, transpose, data);
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix3x2fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    bool transpose = info[1]->BooleanValue();
    Local<Value> dataValue = info[2];

    GLfloat  *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLfloat>(float32Array, &count);
    } else {
      data = getArrayData<GLfloat>(dataValue, &count);
    }

    if (count < 6) {
      Nan::ThrowError("Not enough data for UniformMatrix3x2fv");
    } else {
      count /= 6;
      glUniformMatrix3x2fv(location, count, transpose, data);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix4x2fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    bool transpose = info[1]->BooleanValue();
    Local<Value> dataValue = info[2];

    GLfloat  *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLfloat>(float32Array, &count);
    } else {
      data = getArrayData<GLfloat>(dataValue, &count);
    }

    if (count < 8) {
      Nan::ThrowError("Not enough data for UniformMatrix4x2fv");
    } else {
      count /= 8;
      glUniformMatrix4x2fv(location, count, transpose, data);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix2x3fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    bool transpose = info[1]->BooleanValue();
    Local<Value> dataValue = info[2];

    GLfloat  *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLfloat>(float32Array, &count);
    } else {
      data = getArrayData<GLfloat>(dataValue, &count);
    }

    if (count < 6) {
      Nan::ThrowError("Not enough data for UniformMatrix2x3fv");
    } else {
      count /= 6;
      glUniformMatrix2x3fv(location, count, transpose, data);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix4x3fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    bool transpose = info[1]->BooleanValue();
    Local<Value> dataValue = info[2];

    GLfloat  *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLfloat>(float32Array, &count);
    } else {
      data = getArrayData<GLfloat>(dataValue, &count);
    }

    if (count < 12) {
      Nan::ThrowError("Not enough data for UniformMatrix4x3fv");
    } else {
      count /= 12;
      glUniformMatrix4x3fv(location, count, transpose, data);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix2x4fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    bool transpose = info[1]->BooleanValue();
    Local<Value> dataValue = info[2];

    GLfloat  *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLfloat>(float32Array, &count);
    } else {
      data = getArrayData<GLfloat>(dataValue, &count);
    }

    if (count < 8) {
      Nan::ThrowError("Not enough data for UniformMatrix2x4fv");
    } else {
      count /= 8;
      glUniformMatrix2x4fv(location, count, transpose, data);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::UniformMatrix3x4fv) {
  if (info[0]->IsObject()) {
    GLuint location = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
    bool transpose = info[1]->BooleanValue();
    Local<Value> dataValue = info[2];

    GLfloat  *data;
    GLsizei count;
    if (dataValue->IsArray()) {
      Local<Array> array = Local<Array>::Cast(dataValue);
      unsigned int length = array->Length();
      Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
      for (unsigned int i = 0; i < length; i++) {
        float32Array->Set(i, array->Get(i));
      }
      data = getArrayData<GLfloat>(float32Array, &count);
    } else {
      data = getArrayData<GLfloat>(dataValue, &count);
    }

    if (count < 12) {
      Nan::ThrowError("Not enough data for UniformMatrix3x4fv");
    } else {
      count /= 12;
      glUniformMatrix3x4fv(location, count, transpose, data);
    }
  }
}

NAN_METHOD(WebGLRenderingContext::PixelStorei) {
  int pname = info[0]->Int32Value();
  int param = info[1]->Int32Value();

  if (pname == UNPACK_FLIP_Y_WEBGL) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
    gl->flipY = (bool)param;
  } else if (pname == UNPACK_PREMULTIPLY_ALPHA_WEBGL) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
    gl->premultiplyAlpha = (bool)param;
  } else if (pname == GL_PACK_ALIGNMENT) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
    gl->packAlignment = param;
    glPixelStorei(pname, param);
  } else if (pname == GL_UNPACK_ALIGNMENT) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
    gl->unpackAlignment = param;
    glPixelStorei(pname, param);
  } else {
    glPixelStorei(pname, param);
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BindAttribLocation) {
  GLuint programId = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
  int index = info[1]->Int32Value();
  String::Utf8Value name(info[2]);

  glBindAttribLocation(programId, index, *name);

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::GetError) {
  GLenum error = glGetError();
  info.GetReturnValue().Set(Nan::New<Integer>(error));
}


NAN_METHOD(WebGLRenderingContext::DrawArrays) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  int mode = info[0]->Int32Value();
  int first = info[1]->Int32Value();
  int count = info[2]->Int32Value();

  glDrawArrays(mode, first, count);

  gl->dirty = true;

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DrawArraysInstanced) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  int mode = info[0]->Int32Value();
  int first = info[1]->Int32Value();
  int count = info[2]->Int32Value();
  int primcount = info[3]->Int32Value();

  glDrawArraysInstanced(mode, first, count, primcount);

  gl->dirty = true;

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GenerateMipmap) {
  GLint target = info[0]->Int32Value();
  glGenerateMipmap(target);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetAttribLocation) {
  GLint programId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  String::Utf8Value name(info[1]);

  GLint result = glGetAttribLocation(programId, *name);

  info.GetReturnValue().Set(Nan::New<Number>(result));
}


NAN_METHOD(WebGLRenderingContext::DepthFunc) {
  GLint arg = info[0]->Int32Value();
  glDepthFunc(arg);

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::Viewport) {
  int x = info[0]->Int32Value();
  int y = info[1]->Int32Value();
  int width = info[2]->Int32Value();
  int height = info[3]->Int32Value();

  glViewport(x, y, width, height);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CreateShader) {
  GLint type = info[0]->Int32Value();

  GLuint shaderId = glCreateShader(type);
  Local<Object> shaderObject = Nan::New<Object>();
  shaderObject->Set(JS_STR("id"), JS_INT(shaderId));

  info.GetReturnValue().Set(shaderObject);
}


NAN_METHOD(WebGLRenderingContext::ShaderSource) {
  GLint shaderId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  String::Utf8Value code(info[1]);
  GLint length = code.length();

  const char* codes[] = {*code};
  const GLint lengths[] = {length};
  glShaderSource(shaderId, 1, codes, lengths);

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::CompileShader) {
  GLint shaderId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  glCompileShader(shaderId);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::FrontFace) {
  GLint arg = info[0]->Int32Value();
  glFrontFace(arg);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::IsContextLost) {
  info.GetReturnValue().Set(JS_BOOL(false));
}

NAN_GETTER(WebGLRenderingContext::DrawingBufferWidthGetter) {
  Nan::HandleScope scope;

  Local<Object> glObj = info.This();
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);

  int width, height;
  glfwGetWindowSize(gl->windowHandle, &width, &height);

  info.GetReturnValue().Set(JS_INT(width));
}

NAN_GETTER(WebGLRenderingContext::DrawingBufferHeightGetter) {
  Nan::HandleScope scope;

  Local<Object> glObj = info.This();
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);

  int width, height;
  glfwGetWindowSize(gl->windowHandle, &width, &height);

  info.GetReturnValue().Set(JS_INT(height));
}

NAN_METHOD(WebGLRenderingContext::SetDefaultFramebuffer) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  GLuint framebuffer = info[0]->Uint32Value();

  GLuint oldFramebuffer = gl->HasFramebufferBinding(GL_FRAMEBUFFER) ? gl->GetFramebufferBinding(GL_FRAMEBUFFER) : 0;
  GLuint oldDefaultFramebuffer = gl->defaultFramebuffer;
  if (oldFramebuffer == oldDefaultFramebuffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    gl->SetFramebufferBinding(GL_FRAMEBUFFER, framebuffer);
  }

  gl->defaultFramebuffer = framebuffer;
}

NAN_METHOD(WebGLRenderingContext::GetShaderParameter) {
  GLint shaderId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  GLint pname = info[1]->Int32Value();
  int value;
  switch (pname) {
    case GL_DELETE_STATUS:
    case GL_COMPILE_STATUS:
      glGetShaderiv(shaderId, pname, &value);
      info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value)));
      break;
    case GL_SHADER_TYPE:
      glGetShaderiv(shaderId, pname, &value);
      info.GetReturnValue().Set(JS_FLOAT(static_cast<unsigned long>(value)));
      break;
    case GL_INFO_LOG_LENGTH:
    case GL_SHADER_SOURCE_LENGTH:
      glGetShaderiv(shaderId, pname, &value);
      info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
      break;
    default:
      Nan::ThrowTypeError("GetShaderParameter: Invalid Enum");
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetShaderInfoLog) {
  GLint shaderId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  char Error[1024];
  int Len;

  glGetShaderInfoLog(shaderId, sizeof(Error), &Len, Error);

  info.GetReturnValue().Set(JS_STR(Error, Len));
}


NAN_METHOD(WebGLRenderingContext::CreateProgram) {
  GLuint programId = glCreateProgram();

  Local<Object> programObject = Nan::New<Object>();
  programObject->Set(JS_STR("id"), JS_INT(programId));
  info.GetReturnValue().Set(programObject);
}


NAN_METHOD(WebGLRenderingContext::AttachShader) {
  GLint programId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  GLint shaderId = info[1]->ToObject()->Get(JS_STR("id"))->Int32Value();

  glAttachShader(programId, shaderId);
}


NAN_METHOD(WebGLRenderingContext::LinkProgram) {
  GLint programId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  glLinkProgram(programId);
}


NAN_METHOD(WebGLRenderingContext::GetProgramParameter) {
  GLint programId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  int pname = info[1]->Int32Value();
  int value;

  switch (pname) {
    case GL_DELETE_STATUS:
    case GL_LINK_STATUS:
    case GL_VALIDATE_STATUS:
      glGetProgramiv(programId, pname, &value);
      info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value)));
      break;
    case GL_ATTACHED_SHADERS:
    case GL_ACTIVE_ATTRIBUTES:
    case GL_ACTIVE_UNIFORMS:
      glGetProgramiv(programId, pname, &value);
      info.GetReturnValue().Set(JS_FLOAT(static_cast<long>(value)));
      break;
    default:
      Nan::ThrowTypeError("GetProgramParameter: Invalid Enum");
  }
}


NAN_METHOD(WebGLRenderingContext::GetUniformLocation) {
  GLint programId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  v8::String::Utf8Value name(info[1]);

  GLint location = glGetUniformLocation(programId, *name);

  Local<Object> locationObject = Nan::New<Object>();
  locationObject->Set(JS_STR("id"), JS_INT(location));
  info.GetReturnValue().Set(locationObject);
}


NAN_METHOD(WebGLRenderingContext::ClearColor) {
  float red = (float)info[0]->NumberValue();
  float green = (float)info[1]->NumberValue();
  float blue = (float)info[2]->NumberValue();
  float alpha = (float)info[3]->NumberValue();

  glClearColor(red, green, blue, alpha);

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::ClearDepth) {
  GLfloat depth = info[0]->NumberValue();
  glClearDepthf(depth);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Disable) {
  GLint arg = info[0]->Int32Value();
  glDisable(arg);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Enable) {
  GLint arg = info[0]->Int32Value();
  glEnable(arg);

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::CreateTexture) {
  GLuint texture;
  glGenTextures(1, &texture);

  Local<Object> textureObject = Nan::New<Object>();
  textureObject->Set(JS_STR("id"), JS_INT(texture));
  info.GetReturnValue().Set(textureObject);
}


NAN_METHOD(WebGLRenderingContext::BindTexture) {
  Local<Object> glObj = info.This();
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);

  GLenum target = info[0]->Int32Value();
  GLuint texture = info[1]->IsObject() ? info[1]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glBindTexture(target, texture);

  gl->SetTextureBinding(gl->activeTexture, target, texture);

  // info.GetReturnValue().Set(Nan::Undefined());
}

/* char texPixels[4096 * 4096 * 4];
NAN_METHOD(WebGLRenderingContext::FlipTextureData) {
  Nan::HandleScope scope;

  int num;
  char *pixels=(char*)getArrayData<BYTE>(info[0], &num);
  int width = info[1]->Int32Value();
  int height = info[2]->Int32Value();

  int elementSize = num / width / height;
  for (int y = 0; y < height; y++) {
    memcpy(&(texPixels[(height - 1 - y) * width * elementSize]), &pixels[y * width * elementSize], width * elementSize);
  }
  memcpy(pixels, texPixels, num);
} */

inline bool hasWidthHeight(Local<Value> &value) {
  MaybeLocal<Object> valueObject(Nan::To<Object>(value));
  if (!valueObject.IsEmpty()) {
    Local<String> widthString = Nan::New<String>("width", sizeof("width") - 1).ToLocalChecked();
    Local<String> heightString = Nan::New<String>("height", sizeof("height") - 1).ToLocalChecked();

    MaybeLocal<Number> widthValue(Nan::To<Number>(valueObject.ToLocalChecked()->Get(widthString)));
    MaybeLocal<Number> heightValue(Nan::To<Number>(valueObject.ToLocalChecked()->Get(heightString)));
    return !widthValue.IsEmpty() && !heightValue.IsEmpty();
  } else {
    return false;
  }
}

size_t getFormatSize(int format) {
  switch (format) {
    case GL_RED:
    case GL_RED_INTEGER:
    case GL_DEPTH_COMPONENT:
    case GL_LUMINANCE:
    case GL_ALPHA:
      return 1;
    case GL_RG:
    case GL_RG_INTEGER:
    case GL_DEPTH_STENCIL:
    case GL_LUMINANCE_ALPHA:
      return 2;
    case GL_RGB:
    case GL_RGB_INTEGER:
      return 3;
    case GL_RGBA:
    case GL_RGBA_INTEGER:
      return 4;
    default:
      return 4;
  }
}

size_t getTypeSize(int type) {
  switch (type) {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      return 1;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
    case GL_HALF_FLOAT:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
      return 2;
    case GL_UNSIGNED_INT:
    case GL_INT:
    case GL_FLOAT:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
    case GL_UNSIGNED_INT_10F_11F_11F_REV:
    case GL_UNSIGNED_INT_5_9_9_9_REV:
    case GL_UNSIGNED_INT_24_8:
    case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      return 4;
    default:
      return 4;
  }
}

int formatMap[] = {
  GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
  GL_RGBA8_SNORM, GL_RGBA, GL_BYTE,
  GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4,
  GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV,
  GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1,
  GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT,
  GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT_OES,
  GL_RGBA32F, GL_RGBA, GL_FLOAT,
  GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,
  GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE,
  GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT,
  GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT,
  GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT,
  GL_RGBA32I, GL_RGBA_INTEGER, GL_INT,
  GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV,
  GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE,
  GL_RGB8_SNORM, GL_RGB, GL_BYTE,
  GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5,
  GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV,
  GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV,
  GL_RGB16F, GL_RGB, GL_HALF_FLOAT,
  GL_RGB16F, GL_RGB, GL_HALF_FLOAT_OES,
  GL_RGB32F, GL_RGB, GL_FLOAT,
  GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE,
  GL_RGB8I, GL_RGB_INTEGER, GL_BYTE,
  GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT,
  GL_RGB16I, GL_RGB_INTEGER, GL_SHORT,
  GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT,
  GL_RGB32I, GL_RGB_INTEGER, GL_INT,
  GL_RG8, GL_RG, GL_UNSIGNED_BYTE,
  GL_RG8_SNORM, GL_RG, GL_BYTE,
  GL_RG16F, GL_RG, GL_HALF_FLOAT,
  GL_RG16F, GL_RG, GL_HALF_FLOAT_OES,
  GL_RG32F, GL_RG, GL_FLOAT,
  GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE,
  GL_RG8I, GL_RG_INTEGER, GL_BYTE,
  GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT,
  GL_RG16I, GL_RG_INTEGER, GL_SHORT,
  GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT,
  GL_RG32I, GL_RG_INTEGER, GL_INT,
  GL_R8, GL_RED, GL_UNSIGNED_BYTE,
  GL_R8_SNORM, GL_RED, GL_BYTE,
  GL_R16F, GL_RED, GL_HALF_FLOAT,
  GL_R16F, GL_RED, GL_HALF_FLOAT_OES,
  GL_R32F, GL_RED, GL_FLOAT,
  GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
  GL_R8I, GL_RED_INTEGER, GL_BYTE,
  GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT,
  GL_R16I, GL_RED_INTEGER, GL_SHORT,
  GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT,
  GL_R32I, GL_RED_INTEGER, GL_INT,
};

int normalizeInternalFormat(int internalformat, int format, int type) {
  if (internalformat == GL_RED || internalformat == GL_RG || internalformat == GL_RGB || internalformat == GL_RGBA) {
    for (size_t i = 0; i < sizeof(formatMap)/3; i++) {
      int b = formatMap[i * 3 + 1];
      int c = formatMap[i * 3 + 2];
      if (format == b && type == c) {
        int a = formatMap[i * 3 + 0];
        internalformat = a;
        break;
      }
    }
  }
  return internalformat;
}

int getImageFormat(Local<Value> arg) {
  if (arg->IsArrayBufferView()) {
    return -1;
  } else {
    Local<Value> constructorName = arg->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"));
    if (
      constructorName->StrictEquals(JS_STR("HTMLImageElement")) ||
      constructorName->StrictEquals(JS_STR("HTMLVideoElement")) ||
      constructorName->StrictEquals(JS_STR("ImageData")) ||
      constructorName->StrictEquals(JS_STR("ImageBitmap")) ||
      constructorName->StrictEquals(JS_STR("HTMLCanvasElement"))
    ) {
      return GL_RGBA;
    } else {
      return -1;
    }
  }
}

size_t getArrayBufferViewElementSize(Local<ArrayBufferView> arrayBufferView) {
  if (arrayBufferView->IsFloat64Array()) {
    return 8;
  } else if (arrayBufferView->IsFloat32Array() || arrayBufferView->IsUint32Array() || arrayBufferView->IsInt32Array()) {
    return 4;
  } else if (arrayBufferView->IsUint16Array() || arrayBufferView->IsInt16Array()) {
    return 2;
  } else {
    return 1;
  }
}

NAN_METHOD(WebGLRenderingContext::TexImage2D) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<Value> target = info[0];
  Local<Value> level = info[1];
  Local<Value> internalformat = info[2];
  Local<Value> width = info[3];
  Local<Value> height = info[4];
  Local<Value> border = info[5];
  Local<Value> format = info[6];
  Local<Value> type = info[7];
  Local<Value> pixels = info[8];
  Local<Value> srcOffset = info[9];

  Local<String> widthString = String::NewFromUtf8(isolate, "width", NewStringType::kInternalized).ToLocalChecked();
  Local<String> heightString = String::NewFromUtf8(isolate, "height", NewStringType::kInternalized).ToLocalChecked();

  if (info.Length() == 6) {
    // width is now format, height is now type, and border is now pixels
    MaybeLocal<Number> targetNumber(Nan::To<Number>(target));
    MaybeLocal<Number> levelNumber(Nan::To<Number>(level));
    MaybeLocal<Number> internalformatNumber(Nan::To<Number>(internalformat));
    MaybeLocal<Number> widthNumber(Nan::To<Number>(width));
    MaybeLocal<Number> heightNumber(Nan::To<Number>(height));
    if (
      !targetNumber.IsEmpty() &&
      !levelNumber.IsEmpty() && !internalformatNumber.IsEmpty() &&
      !widthNumber.IsEmpty() && !heightNumber.IsEmpty() &&
      (border->IsNull() || hasWidthHeight(border))
    ) {
      pixels=border;
      /* if (pixels) {
        pixels = _getImageData(pixels);
      } */
      type=height;
      format=width;
      width = border->BooleanValue() ? border->ToObject()->Get(widthString) : Number::New(isolate, 1).As<Value>();
      height = border->BooleanValue() ? border->ToObject()->Get(heightString) : Number::New(isolate, 1).As<Value>();
      // return _texImage2D(target, level, internalformat, width, height, 0, format, type, pixels);
    } else {
      /* LOGI("Loaded string asset %d %d %d %d %d %d %d %d %d",
        target->TypeOf(isolate)->StrictEquals(numberString),
        level->TypeOf(isolate)->StrictEquals(numberString),
        internalformat->TypeOf(isolate)->StrictEquals(numberString),
        width->TypeOf(isolate)->StrictEquals(numberString),
        height->TypeOf(isolate)->StrictEquals(numberString),
        border->IsNull(), // 0
        !border->IsNull() && border->TypeOf(isolate)->StrictEquals(objectString), // 1
        !border->IsNull() && border->TypeOf(isolate)->StrictEquals(objectString) && border->ToObject()->Get(widthString)->TypeOf(isolate)->StrictEquals(numberString), // 0
        !border->IsNull() && border->TypeOf(isolate)->StrictEquals(objectString) && border->ToObject()->Get(heightString)->TypeOf(isolate)->StrictEquals(numberString) // 0
      ); */

      Nan::ThrowError("Expected texImage2D(number target, number level, number internalformat, number format, number type, Image pixels)");
      return;
    }
  } else if (info.Length() == 9) {
    MaybeLocal<Number> targetNumber(Nan::To<Number>(target));
    MaybeLocal<Number> levelNumber(Nan::To<Number>(level));
    MaybeLocal<Number> internalformatNumber(Nan::To<Number>(internalformat));
    MaybeLocal<Number> widthNumber(Nan::To<Number>(width));
    MaybeLocal<Number> heightNumber(Nan::To<Number>(height));
    MaybeLocal<Number> formatNumber(Nan::To<Number>(format));
    MaybeLocal<Number> typeNumber(Nan::To<Number>(type));
    if (
      !targetNumber.IsEmpty() &&
      !levelNumber.IsEmpty() && !internalformat.IsEmpty() &&
      !widthNumber.IsEmpty() && !heightNumber.IsEmpty() &&
      !formatNumber.IsEmpty() && !typeNumber.IsEmpty() &&
      (pixels->IsNull() || pixels->IsObject() || pixels->IsNumber())
    ) {
      // nothing
    } else {
      Nan::ThrowError("Expected texImage2D(number target, number level, number internalformat, number width, number height, number border, number format, number type, ArrayBufferView pixels)");
      return;
    }
  } else if (info.Length() == 10) {
    MaybeLocal<Number> targetNumber(Nan::To<Number>(target));
    MaybeLocal<Number> levelNumber(Nan::To<Number>(level));
    MaybeLocal<Number> internalformatNumber(Nan::To<Number>(internalformat));
    MaybeLocal<Number> widthNumber(Nan::To<Number>(width));
    MaybeLocal<Number> heightNumber(Nan::To<Number>(height));
    MaybeLocal<Number> formatNumber(Nan::To<Number>(format));
    MaybeLocal<Number> typeNumber(Nan::To<Number>(type));
    MaybeLocal<Number> srcOffsetNumber(Nan::To<Number>(srcOffset));
    if (
      !targetNumber.IsEmpty() &&
      !levelNumber.IsEmpty() && !internalformat.IsEmpty() &&
      !widthNumber.IsEmpty() && !heightNumber.IsEmpty() &&
      !formatNumber.IsEmpty() && !typeNumber.IsEmpty()
    ) {
      if (pixels->IsArrayBufferView() && !srcOffsetNumber.IsEmpty()) {
        Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(pixels);
        size_t srcOffsetInt = srcOffset->Uint32Value();
        size_t elementSize = getArrayBufferViewElementSize(arrayBufferView);
        size_t extraOffset = srcOffsetInt * elementSize;
        pixels = Uint8Array::New(arrayBufferView->Buffer(), arrayBufferView->ByteOffset() + extraOffset, arrayBufferView->ByteLength() - extraOffset);
      } else if (pixels->IsNull() || pixels->IsObject()) {
        // nothing
      } else {
        Nan::ThrowError("Expected texImage2D(number target, number level, number internalformat, number width, number height, number border, number format, number type, ArrayBufferView srcData, number srcOffset)");
        return;
      }
    } else {
      Nan::ThrowError("Expected texImage2D(number target, number level, number internalformat, number width, number height, number border, number format, number type, ArrayBufferView srcData, number srcOffset)");
      return;
    }
  } else {
    Nan::ThrowError("Bad texture argument");
    return;
  }

  GLenum targetV = target->Uint32Value();
  GLenum levelV = level->Uint32Value();
  GLenum internalformatV = internalformat->Uint32Value();
  GLsizei widthV = width->Uint32Value();
  GLsizei heightV = height->Uint32Value();
  GLint borderV = border->Int32Value();
  GLenum formatV = format->Uint32Value();
  GLenum typeV = type->Uint32Value();

  internalformatV = normalizeInternalFormat(internalformatV, formatV, typeV);

  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());

  char *pixelsV;
  if (pixels->IsNull()) {
    glTexImage2D(targetV, levelV, internalformatV, widthV, heightV, borderV, formatV, typeV, nullptr);
  } else if (pixels->IsNumber()) {
    GLintptr offsetV = pixels->Uint32Value();
    glTexImage2D(targetV, levelV, internalformatV, widthV, heightV, borderV, formatV, typeV, (void *)offsetV);
  } else if ((pixelsV = (char *)getImageData(pixels)) != nullptr) {
    size_t formatSize = getFormatSize(formatV);
    size_t typeSize = getTypeSize(typeV);
    size_t pixelSize = formatSize * typeSize;
    int srcFormatV = getImageFormat(pixels);
    size_t srcFormatSize = getFormatSize(srcFormatV);
    char *pixelsV2;
    unique_ptr<char[]> pixelsV2Buffer;
    bool needsReformat = srcFormatV != -1 && formatSize != srcFormatSize;
    if (needsReformat) {
      pixelsV2Buffer.reset(new char[widthV * heightV * pixelSize]);
      pixelsV2 = pixelsV2Buffer.get();
      reformatImageData(pixelsV2, pixelsV, formatSize * typeSize, srcFormatSize * typeSize, widthV * heightV);

      glPixelStorei(GL_PACK_ALIGNMENT, 1);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    } else {
      pixelsV2 = pixelsV;
    }

    if (canvas::ImageData::getFlip() && gl->flipY && !pixels->IsArrayBufferView()) {
      unique_ptr<char[]> pixelsV3Buffer(new char[widthV * heightV * pixelSize]);

      flipImageData(pixelsV3Buffer.get(), pixelsV2, widthV, heightV, pixelSize);

      glTexImage2D(targetV, levelV, internalformatV, widthV, heightV, borderV, formatV, typeV, pixelsV3Buffer.get());
    } else if (formatV == GL_LUMINANCE || formatV == GL_ALPHA) {
      unique_ptr<char[]> pixelsV3Buffer(new char[widthV * heightV * 4]);

      if (typeV == GL_UNSIGNED_BYTE) {
        expandLuminance<unsigned char>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_UNSIGNED_INT) {
        expandLuminance<unsigned int>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_INT) {
        expandLuminance<int>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_UNSIGNED_SHORT) {
        expandLuminance<unsigned short>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_SHORT) {
        expandLuminance<short>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_FLOAT) {
        expandLuminance<float>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else {
        expandLuminance<unsigned char>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      }

      glTexImage2D(targetV, levelV, GL_RGBA8, widthV, heightV, borderV, GL_RGBA, typeV, pixelsV3Buffer.get());
    } else if (formatV == GL_LUMINANCE_ALPHA) {
      unique_ptr<char[]> pixelsV3Buffer(new char[widthV * heightV * 4]);

      if (typeV == GL_UNSIGNED_BYTE) {
        expandLuminanceAlpha<unsigned char>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_UNSIGNED_INT) {
        expandLuminanceAlpha<unsigned int>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_INT) {
        expandLuminanceAlpha<int>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_UNSIGNED_SHORT) {
        expandLuminanceAlpha<unsigned short>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_SHORT) {
        expandLuminanceAlpha<short>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else if (typeV == GL_FLOAT) {
        expandLuminanceAlpha<float>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      } else {
        expandLuminanceAlpha<unsigned char>(pixelsV3Buffer.get(), pixelsV2, widthV, heightV);
      }

      glTexImage2D(targetV, levelV, GL_RGBA8, widthV, heightV, borderV, GL_RGBA, typeV, pixelsV3Buffer.get());
    } else {
      glTexImage2D(targetV, levelV, internalformatV, widthV, heightV, borderV, formatV, typeV, pixelsV2);
    }

    if (needsReformat) {
      glPixelStorei(GL_PACK_ALIGNMENT, gl->packAlignment);
      glPixelStorei(GL_UNPACK_ALIGNMENT, gl->unpackAlignment);
    }
  } else {
    Nan::ThrowError(String::Concat(JS_STR("Invalid texture argument: "), pixels->ToString()));
  }
}

NAN_METHOD(WebGLRenderingContext::CompressedTexImage2D) {
  Isolate *isolate = Isolate::GetCurrent();

  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber() && info[4]->IsNumber() && info[5]->IsNumber()) {
    char *dataV;
    size_t dataLengthV;
    if (info[6]->IsArrayBufferView()) {
      Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(info[6]);
      Local<ArrayBuffer> buffer = arrayBufferView->Buffer();
      dataV = (char *)buffer->GetContents().Data() + arrayBufferView->ByteOffset();
      dataLengthV = arrayBufferView->ByteLength();
    } else if (info[6]->IsNull()) {
      dataV = nullptr;
      dataLengthV = 0;
    } else {
      return Nan::ThrowError("compressedTexImage2D: invalid arguments");
    }

    Local<Value> target = info[0];
    Local<Value> level = info[1];
    Local<Value> internalformat = info[2];
    Local<Value> width = info[3];
    Local<Value> height = info[4];
    Local<Value> border = info[5];

    int targetV = target->Int32Value();
    int levelV = level->Int32Value();
    int internalformatV = internalformat->Int32Value();
    int widthV = width->Int32Value();
    int heightV = height->Int32Value();
    int borderV = border->Int32Value();

    glCompressedTexImage2D(targetV, levelV, internalformatV, widthV, heightV, borderV, dataLengthV, dataV);
  } else {
    Nan::ThrowError("compressedTexImage2D: invalid arguments");
  }
}

NAN_METHOD(WebGLRenderingContext::TexParameteri) {
  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int param = info[2]->Int32Value();

  glTexParameteri(target, pname, param);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::TexParameterf) {
  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  float param = (float) info[2]->NumberValue();

  glTexParameterf(target, pname, param);
}


NAN_METHOD(WebGLRenderingContext::Clear) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  GLint arg = info[0]->Int32Value();

  glClear(arg);

  gl->dirty = true;
}


NAN_METHOD(WebGLRenderingContext::UseProgram) {
  GLint programId = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Int32Value() : 0;
  glUseProgram(programId);
}

NAN_METHOD(WebGLRenderingContext::CreateBuffer) {
  GLuint buffer;
  glGenBuffers(1, &buffer);

  Local<Object> bufferObject = Nan::New<Object>();
  bufferObject->Set(JS_STR("id"), JS_INT(buffer));
  info.GetReturnValue().Set(bufferObject);
}

NAN_METHOD(WebGLRenderingContext::BindBuffer) {
  if (info.Length() < 2) {
    Nan::ThrowError("BindBuffer requires at least 2 arguments");
  } else if (!info[0]->IsNumber()) {
    Nan::ThrowError("First argument to BindBuffer must be a number");
  } else if (info[1]->IsObject() && info[1]->ToObject()->Get(JS_STR("id"))->IsNumber()) {
    GLint target = info[0]->Int32Value();
    GLint buffer = info[1]->ToObject()->Get(JS_STR("id"))->Int32Value();
    glBindBuffer(target, buffer);
  } else if (info[1]->IsNull()) {
    GLint target = info[0]->Int32Value();
    glBindBuffer(target, 0);
  } else {
    Nan::ThrowError(String::Concat(JS_STR("Second argument to BindBuffer must be null or a WebGLBuffer; was "), info[1]->ToString()));
  }
}


NAN_METHOD(WebGLRenderingContext::CreateFramebuffer) {
  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);

  Local<Object> framebufferObject = Nan::New<Object>();
  framebufferObject->Set(JS_STR("id"), JS_INT(framebuffer));
  info.GetReturnValue().Set(framebufferObject);
}


NAN_METHOD(WebGLRenderingContext::BindFramebuffer) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());

  GLenum target = info[0]->Uint32Value();
  GLuint framebuffer = info[1]->IsObject() ? info[1]->ToObject()->Get(JS_STR("id"))->Uint32Value() : gl->defaultFramebuffer;

  glBindFramebuffer(target, framebuffer);

  gl->SetFramebufferBinding(target, framebuffer);
}


NAN_METHOD(WebGLRenderingContext::FramebufferTexture2D) {
  GLenum target = info[0]->Uint32Value();
  GLenum attachment = info[1]->Int32Value();
  GLenum textarget = info[2]->Int32Value();
  GLuint texture = info[3]->IsObject() ? info[3]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;
  GLint level = info[4]->Int32Value();

  glFramebufferTexture2D(target, attachment, textarget, texture, level);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlitFramebuffer) {
  int sx = info[0]->Uint32Value();
  int sy = info[1]->Uint32Value();
  int sw = info[2]->Uint32Value();
  int sh = info[3]->Uint32Value();
  int dx = info[4]->Uint32Value();
  int dy = info[5]->Uint32Value();
  int dw = info[6]->Uint32Value();
  int dh = info[7]->Uint32Value();
  GLbitfield mask = info[8]->Uint32Value();
  GLenum filter = info[9]->Uint32Value();

  glBlitFramebuffer(
    sx, sy,
    sw, sh,
    dx, dy,
    dw, dh,
    mask,
    filter
  );

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BufferData) {
  GLenum target = info[0]->Uint32Value();
  if (info[1]->IsObject()) {
    Local<Object> obj = Local<Object>::Cast(info[1]);
    GLenum usage = info[2]->Int32Value();

    int element_size = 1;
    Local<ArrayBufferView> arr = Local<ArrayBufferView>::Cast(obj);
    int size = arr->ByteLength() * element_size;
    char *data = (char *)arr->Buffer()->GetContents().Data() + arr->ByteOffset();

    glBufferData(target, size, data, usage);
  } else if(info[1]->IsNumber()) {
    GLsizeiptr size = info[1]->Uint32Value();
    GLenum usage = info[2]->Int32Value();

    glBufferData(target, size, NULL, usage);
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::BufferSubData) {
  GLenum target = info[0]->Uint32Value();
  GLint dstOffset = info[1]->Int32Value();
  Local<Object> obj = Local<Object>::Cast(info[2]);

  char *data;
  GLuint size;
  if (obj->IsArrayBufferView()) {
    Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(obj);
    data = (char *)arrayBufferView->Buffer()->GetContents().Data() + arrayBufferView->ByteOffset();

    if (info[3]->IsNumber()) {
      size_t srcOffset = info[3]->Uint32Value() * getArrayBufferViewElementSize(arrayBufferView);
      data += srcOffset;
      size = info[4]->IsNumber() ? info[4]->NumberValue() : 0;
    } else {
      size = arrayBufferView->ByteLength();
    }
  } else if (obj->IsArrayBuffer()) {
    Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(obj);
    data = (char *)arrayBuffer->GetContents().Data();
    size = arrayBuffer->ByteLength();
  } else {
    Nan::ThrowError("Invalid texture argument");
    return;
  }

  glBufferSubData(target, dstOffset, size, data);
}


NAN_METHOD(WebGLRenderingContext::BlendEquation) {
  GLint mode = info[0]->Int32Value();
  glBlendEquation(mode);

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::BlendFunc) {
  GLint sfactor = info[0]->Int32Value();
  GLint dfactor = info[1]->Int32Value();

  glBlendFunc(sfactor, dfactor);

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::EnableVertexAttribArray) {
  GLint arg = info[0]->Int32Value();
  glEnableVertexAttribArray(arg);

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::VertexAttribPointer) {
  GLuint indx = info[0]->Uint32Value();
  GLint size = info[1]->Int32Value();
  GLenum type = info[2]->Uint32Value();
  GLboolean normalized = info[3]->BooleanValue();
  GLint stride = info[4]->Int32Value();
  GLint offset = info[5]->Int32Value();

  //    printf("VertexAttribPointer %d %d %d %d %d %d\n", indx, size, type, normalized, stride, offset);
  glVertexAttribPointer(indx, size, type, normalized, stride, (const GLvoid *)offset);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttribIPointer) {
  GLuint indx = info[0]->Uint32Value();
  GLint size = info[1]->Int32Value();
  GLenum type = info[2]->Uint32Value();
  GLint stride = info[3]->Int32Value();
  GLint offset = info[4]->Int32Value();

  glVertexAttribIPointer(indx, size, type, stride, (const GLvoid *)offset);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ActiveTexture) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  GLenum activeTexture = info[0]->Uint32Value();

  glActiveTexture(activeTexture);

  gl->activeTexture = activeTexture;

  // info.GetReturnValue().Set(Nan::Undefined());
}


NAN_METHOD(WebGLRenderingContext::DrawElements) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  GLenum mode = info[0]->Uint32Value();
  GLsizei count = info[1]->Int32Value();
  GLenum type = info[2]->Uint32Value();
  GLvoid *offset = reinterpret_cast<GLvoid*>(info[3]->Uint32Value());

  glDrawElements(mode, count, type, offset);

  gl->dirty = true;

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DrawElementsInstanced) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  GLenum mode = info[0]->Uint32Value();
  GLsizei count = info[1]->Int32Value();
  GLenum type = info[2]->Uint32Value();
  GLvoid *offset = reinterpret_cast<GLvoid*>(info[3]->Uint32Value());
  GLsizei primcount = info[4]->Int32Value();

  glDrawElementsInstanced(mode, count, type, offset, primcount);

  gl->dirty = true;

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DrawRangeElements) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  GLenum mode = info[0]->Uint32Value();
  GLuint start = info[1]->Uint32Value();
  GLuint end = info[2]->Uint32Value();
  GLsizei count = info[3]->Uint32Value();
  GLenum type = info[4]->Uint32Value();
  GLintptr offset = info[5]->Int32Value();

  glDrawRangeElements(mode, start, end, count, type, (void *)offset);

  gl->dirty = true;
}

NAN_METHOD(WebGLRenderingContext::Flush) {
  // Nan::HandleScope scope;

  glFlush();

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Finish) {
  // Nan::HandleScope scope;

  glFinish();

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib1f) {
  GLuint indx = info[0]->Int32Value();
  GLfloat x = info[1]->NumberValue();

  glVertexAttrib1f(indx, x);
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib2f) {
  GLuint indx = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();

  glVertexAttrib2f(indx, x, y);
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib3f) {
  GLuint indx = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();
  float z = (float)info[3]->NumberValue();

  glVertexAttrib3f(indx, x, y, z);
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib4f) {
  GLuint indx = info[0]->Int32Value();
  float x = (float)info[1]->NumberValue();
  float y = (float)info[2]->NumberValue();
  float z = (float)info[3]->NumberValue();
  float w = (float)info[4]->NumberValue();

  glVertexAttrib4f(indx, x, y, z, w);
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib1fv) {
  int indx = info[0]->Int32Value();
  
  GLfloat *data;
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &num);
  } else {
    data = getArrayData<GLfloat>(info[1], &num);
  }

  glVertexAttrib1fv(indx, data);
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib2fv) {
  int indx = info[0]->Int32Value();
  
  GLfloat *data;
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data=getArrayData<GLfloat>(float32Array, &num);
  } else {
    data=getArrayData<GLfloat>(info[1], &num);
  }

  glVertexAttrib2fv(indx, data);
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib3fv) {
  int indx = info[0]->Int32Value();
  
  GLfloat *data;
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLfloat>(float32Array, &num);
  } else {
    data = getArrayData<GLfloat>(info[1], &num);
  }

  glVertexAttrib3fv(indx, data);
}

NAN_METHOD(WebGLRenderingContext::VertexAttrib4fv) {
  int indx = info[0]->Int32Value();
  
  GLfloat *data;
  int num;
  if (info[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(info[1]);
    unsigned int length = array->Length();
    Local<Float32Array> float32Array = Float32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      float32Array->Set(i, array->Get(i));
    }
    data=getArrayData<GLfloat>(float32Array, &num);
  } else {
    data=getArrayData<GLfloat>(info[1], &num);
  }

  glVertexAttrib4fv(indx, data);
}

NAN_METHOD(WebGLRenderingContext::VertexAttribI4i) {
  GLint index = info[0]->Int32Value();
  GLint v0 = info[1]->Int32Value();
  GLint v1 = info[2]->Int32Value();
  GLint v2 = info[3]->Int32Value();
  GLint v3 = info[4]->Int32Value();

  glVertexAttribI4i(index, v0, v1, v2, v3);
}

NAN_METHOD(WebGLRenderingContext::VertexAttribI4iv) {
  GLuint index = info[0]->Uint32Value();
  Local<Value> dataValue = info[1];

  GLint *data;
  GLsizei count;
  if (dataValue->IsArray()) {
    Local<Array> array = Local<Array>::Cast(dataValue);
    unsigned int length = array->Length();
    Local<Int32Array> int32Array = Int32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      int32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLint>(int32Array, &count);
  } else {
    data = getArrayData<GLint>(dataValue, &count);
  }

  glVertexAttribI4iv(index, data);
}

NAN_METHOD(WebGLRenderingContext::VertexAttribI4ui) {
  GLuint index = info[0]->Uint32Value();
  GLuint v0 = info[1]->Uint32Value();
  GLuint v1 = info[2]->Uint32Value();
  GLuint v2 = info[3]->Uint32Value();
  GLuint v3 = info[4]->Uint32Value();

  glVertexAttribI4ui(index, v0, v1, v2, v3);
}

NAN_METHOD(WebGLRenderingContext::VertexAttribI4uiv) {
  GLuint index = info[0]->Uint32Value();
  Local<Value> dataValue = info[1];

  GLuint *data;
  GLsizei count;
  if (dataValue->IsArray()) {
    Local<Array> array = Local<Array>::Cast(dataValue);
    unsigned int length = array->Length();
    Local<Uint32Array> uint32Array = Uint32Array::New(ArrayBuffer::New(Isolate::GetCurrent(), length * 4), 0, length);
    for (unsigned int i = 0; i < length; i++) {
      uint32Array->Set(i, array->Get(i));
    }
    data = getArrayData<GLuint>(uint32Array, &count);
  } else {
    data = getArrayData<GLuint>(dataValue, &count);
  }

  glVertexAttribI4uiv(index, data);
}

NAN_METHOD(WebGLRenderingContext::VertexAttribDivisor) {
  GLuint index = info[0]->Uint32Value();
  GLuint divisor = info[1]->Uint32Value();

  glVertexAttribDivisor(index, divisor);
}

NAN_METHOD(WebGLRenderingContext::DrawBuffers) {
  Local<Array> buffersArray = Local<Array>::Cast(info[0]);
  GLenum buffers[32];
  size_t numBuffers = std::min<size_t>(buffersArray->Length(), sizeof(buffers)/sizeof(buffers[0]));
  for (size_t i = 0; i < numBuffers; i++) {
    buffers[i] = buffersArray->Get(i)->Uint32Value();
  }

  glDrawBuffers(numBuffers, buffers);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendColor) {
  GLclampf r = (float)info[0]->NumberValue();
  GLclampf g = (float)info[1]->NumberValue();
  GLclampf b = (float)info[2]->NumberValue();
  GLclampf a = (float)info[3]->NumberValue();

  glBlendColor(r, g, b, a);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendEquationSeparate) {
  GLenum modeRGB = info[0]->Int32Value();
  GLenum modeAlpha = info[1]->Int32Value();

  glBlendEquationSeparate(modeRGB, modeAlpha);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BlendFuncSeparate) {
  GLenum srcRGB = info[0]->Int32Value();
  GLenum dstRGB = info[1]->Int32Value();
  GLenum srcAlpha = info[2]->Int32Value();
  GLenum dstAlpha = info[3]->Int32Value();

  glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ClearStencil) {
  GLint s = info[0]->Int32Value();

  glClearStencil(s);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::ColorMask) {
  GLboolean r = info[0]->BooleanValue();
  GLboolean g = info[1]->BooleanValue();
  GLboolean b = info[2]->BooleanValue();
  GLboolean a = info[3]->BooleanValue();

  glColorMask(r, g, b, a);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CopyTexImage2D) {
  GLenum target = info[0]->Uint32Value();
  GLint level = info[1]->Int32Value();
  GLenum internalformat = info[2]->Uint32Value();
  GLint x = info[3]->Int32Value();
  GLint y = info[4]->Int32Value();
  GLsizei width = info[5]->Uint32Value();
  GLsizei height = info[6]->Uint32Value();
  GLint border = info[7]->Int32Value();

  glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CopyTexSubImage2D) {
  GLenum target = info[0]->Uint32Value();
  GLint level = info[1]->Int32Value();
  GLint xoffset = info[2]->Int32Value();
  GLint yoffset = info[3]->Int32Value();
  GLint x = info[4]->Int32Value();
  GLint y = info[5]->Int32Value();
  GLsizei width = info[6]->Uint32Value();
  GLsizei height = info[7]->Uint32Value();

  glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CullFace) {
  GLenum mode = info[0]->Int32Value();

  glCullFace(mode);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DepthMask) {
  GLboolean flag = info[0]->BooleanValue();

  glDepthMask(flag);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DepthRange) {
  GLclampf zNear = (float) info[0]->NumberValue();
  GLclampf zFar = (float) info[1]->NumberValue();

  glDepthRangef(zNear, zFar);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DisableVertexAttribArray) {
  GLuint index = info[0]->Int32Value();
  glDisableVertexAttribArray(index);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Hint) {
  GLenum target = info[0]->Int32Value();
  GLenum mode = info[1]->Int32Value();

  glHint(target, mode);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::IsEnabled) {
  GLenum cap = info[0]->Uint32Value();
  bool ret = glIsEnabled(cap);

  info.GetReturnValue().Set(Nan::New<Boolean>(ret));
}

NAN_METHOD(WebGLRenderingContext::LineWidth) {
  GLfloat width = (float) info[0]->NumberValue();
  glLineWidth(width);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::PolygonOffset) {
  GLfloat factor = (float) info[0]->NumberValue();
  GLfloat units = (float) info[1]->NumberValue();

  glPolygonOffset(factor, units);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::SampleCoverage) {
  GLclampf value = info[0]->NumberValue();
  GLboolean invert = info[1]->BooleanValue();

  glSampleCoverage(value, invert);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::Scissor) {
  GLint x = info[0]->Int32Value();
  GLint y = info[1]->Int32Value();
  GLsizei width = info[2]->Uint32Value();
  GLsizei height = info[3]->Uint32Value();

  glScissor(x, y, width, height);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilFunc) {
  GLenum func = info[0]->Int32Value();
  GLint ref = info[1]->Int32Value();
  GLuint mask = info[2]->Int32Value();

  glStencilFunc(func, ref, mask);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilFuncSeparate) {
  GLenum face = info[0]->Int32Value();
  GLenum func = info[1]->Int32Value();
  GLint ref = info[2]->Int32Value();
  GLuint mask = info[3]->Int32Value();

  glStencilFuncSeparate(face, func, ref, mask);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilMask) {
  GLuint mask = info[0]->Uint32Value();

  glStencilMask(mask);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilMaskSeparate) {
  GLenum face = info[0]->Int32Value();
  GLuint mask = info[1]->Uint32Value();

  glStencilMaskSeparate(face, mask);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilOp) {
  GLenum fail = info[0]->Int32Value();
  GLenum zfail = info[1]->Int32Value();
  GLenum zpass = info[2]->Int32Value();

  glStencilOp(fail, zfail, zpass);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::StencilOpSeparate) {
  GLenum face = info[0]->Int32Value();
  GLenum fail = info[1]->Int32Value();
  GLenum zfail = info[2]->Int32Value();
  GLenum zpass = info[3]->Int32Value();

  glStencilOpSeparate(face, fail, zfail, zpass);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BindRenderbuffer) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());

  GLenum target = info[0]->Int32Value();
  GLuint renderbuffer = info[1]->IsObject() ? info[1]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glBindRenderbuffer(target, renderbuffer);

  gl->SetRenderbufferBinding(target, renderbuffer);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::CreateRenderbuffer) {
  GLuint renderbuffer;
  glGenRenderbuffers(1, &renderbuffer);

  Local<Object> renderbufferObject = Nan::New<Object>();
  renderbufferObject->Set(JS_STR("id"), JS_INT(renderbuffer));
  info.GetReturnValue().Set(renderbufferObject);
}

NAN_METHOD(WebGLRenderingContext::DeleteBuffer) {
  GLuint buffer = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glDeleteBuffers(1, &buffer);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteFramebuffer) {
  GLuint framebuffer = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glDeleteFramebuffers(1, &framebuffer);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteProgram) {
  GLint programId = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Int32Value() : 0;

  glDeleteProgram(programId);
}

NAN_METHOD(WebGLRenderingContext::DeleteRenderbuffer) {
  GLuint renderbuffer = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glDeleteRenderbuffers(1, &renderbuffer);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteShader) {
  GLuint shaderId = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glDeleteShader(shaderId);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DeleteTexture) {
  GLuint texture = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glDeleteTextures(1, &texture);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::DetachShader) {
  GLuint programId = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
  GLuint shaderId = info[1]->ToObject()->Get(JS_STR("id"))->Uint32Value();

  glDetachShader(programId, shaderId);
}

NAN_METHOD(WebGLRenderingContext::FramebufferRenderbuffer) {
  GLenum target = info[0]->Int32Value();
  GLenum attachment = info[1]->Int32Value();
  GLenum renderbuffertarget = info[2]->Int32Value();
  GLuint renderbuffer = info[3]->IsObject() ? info[3]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetVertexAttribOffset) {
  GLuint index = info[0]->Uint32Value();
  GLenum pname = info[1]->Uint32Value();
  void *ret = nullptr;

  glGetVertexAttribPointerv(index, pname, &ret);

  info.GetReturnValue().Set(JS_INT(ToGLuint(ret)));
}

NAN_METHOD(WebGLRenderingContext::GetShaderPrecisionFormat) {
  GLenum shaderType = info[0]->Uint32Value();
  GLenum precisionType = info[1]->Uint32Value();
  GLint range[2];
  GLint precision;

  glGetShaderPrecisionFormat(shaderType, precisionType, range, &precision);

  Local<Object> result = Object::New(Isolate::GetCurrent());
  result->Set(JS_STR("rangeMin"), JS_INT(range[0]));
  result->Set(JS_STR("rangeMax"), JS_INT(range[1]));
  result->Set(JS_STR("precision"), JS_INT(precision));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(WebGLRenderingContext::IsBuffer) {
  if (info[0]->IsObject()) {
    GLuint arg = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;
    bool ret = glIsBuffer(arg);

    info.GetReturnValue().Set(Nan::New<Boolean>(ret));
  } else {
    info.GetReturnValue().Set(Nan::New<Boolean>(false));
  }
}

NAN_METHOD(WebGLRenderingContext::IsFramebuffer) {
  if (info[0]->IsObject()) {
    GLuint arg = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;
    bool ret = glIsFramebuffer(arg);

    info.GetReturnValue().Set(JS_BOOL(ret));
  } else {
    info.GetReturnValue().Set(Nan::New<Boolean>(false));
  }
}

NAN_METHOD(WebGLRenderingContext::IsProgram) {
  if (info[0]->IsObject()) {
    GLuint arg = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;
    bool ret = glIsProgram(arg);

    info.GetReturnValue().Set(JS_BOOL(ret));
  } else {
    info.GetReturnValue().Set(Nan::New<Boolean>(false));
  }
}

NAN_METHOD(WebGLRenderingContext::IsRenderbuffer) {
  if (info[0]->IsObject()) {
    GLuint arg = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;
    bool ret = glIsRenderbuffer(arg);

    info.GetReturnValue().Set(JS_BOOL(ret));
  } else {
    info.GetReturnValue().Set(Nan::New<Boolean>(false));
  }
}

NAN_METHOD(WebGLRenderingContext::IsShader) {
  if (info[0]->IsObject()) {
    GLuint arg = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;
    bool ret = glIsShader(arg);

    info.GetReturnValue().Set(JS_BOOL(ret));
  } else {
    info.GetReturnValue().Set(Nan::New<Boolean>(false));
  }
}

NAN_METHOD(WebGLRenderingContext::IsTexture) {
  if (info[0]->IsObject()) {
    GLuint arg = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;
    bool ret = glIsTexture(arg);

    info.GetReturnValue().Set(JS_BOOL(ret));
  } else {
    info.GetReturnValue().Set(Nan::New<Boolean>(false));
  }
}

NAN_METHOD(WebGLRenderingContext::IsVertexArray) {
  if (info[0]->IsObject()) {
    GLuint arg = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;
    bool ret = glIsVertexArray(arg);

    info.GetReturnValue().Set(JS_BOOL(ret));
  } else {
    info.GetReturnValue().Set(Nan::New<Boolean>(false));
  }
}

NAN_METHOD(WebGLRenderingContext::IsSync) {
  if (info[0]->IsObject()) {
    Local<Value> syncId = info[0]->ToObject()->Get(JS_STR("id"));
    if (syncId->IsArray()) {
      Local<Array> syncArray = Local<Array>::Cast(syncId);
      if (syncArray->Get(0)->IsNumber() && syncArray->Get(1)->IsNumber()) {
        GLsync sync = (GLsync)arrayToPointer(syncArray);
        bool ret = glIsSync(sync);

        info.GetReturnValue().Set(JS_BOOL(ret));
      } else {
        info.GetReturnValue().Set(Nan::New<Boolean>(false));
      }
    } else {
      info.GetReturnValue().Set(Nan::New<Boolean>(false));
    }
  } else {
    info.GetReturnValue().Set(Nan::New<Boolean>(false));
  }
}

NAN_METHOD(WebGLRenderingContext::RenderbufferStorage) {
  GLenum target = info[0]->Int32Value();
  GLenum internalformat = info[1]->Int32Value();
  GLsizei width = info[2]->Uint32Value();
  GLsizei height = info[3]->Uint32Value();

  glRenderbufferStorage(target, internalformat, width, height);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetShaderSource) {
  GLuint shaderId = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();

  GLint len;
  glGetShaderiv(shaderId, GL_SHADER_SOURCE_LENGTH, &len);
  GLchar *source = new GLchar[len];

  glGetShaderSource(shaderId, len, nullptr, source);

  Local<String> str = JS_STR(source, len);
  delete[] source;

  info.GetReturnValue().Set(str);
}

NAN_METHOD(WebGLRenderingContext::ValidateProgram) {
  GLuint programId = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();

  glValidateProgram(programId);
}

NAN_METHOD(WebGLRenderingContext::TexSubImage2D) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  GLenum targetV = info[0]->Uint32Value();
  GLint levelV = info[1]->Int32Value();
  GLint xoffsetV = info[2]->Int32Value();
  GLint yoffsetV = info[3]->Int32Value();
  GLsizei widthV = info[4]->Uint32Value();
  GLsizei heightV = info[5]->Uint32Value();
  GLenum formatV = info[6]->Uint32Value();
  GLenum typeV = info[7]->Uint32Value();
  Local<Value> pixels = info[8];
  Local<Value> srcOffset = info[9];

  if (pixels->IsArrayBufferView() && srcOffset->IsNumber()) {
    Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(pixels);
    size_t srcOffsetInt = srcOffset->Uint32Value();
    size_t elementSize = getArrayBufferViewElementSize(arrayBufferView);
    size_t extraOffset = srcOffsetInt * elementSize;
    pixels = Uint8Array::New(arrayBufferView->Buffer(), arrayBufferView->ByteOffset() + extraOffset, arrayBufferView->ByteLength() - extraOffset);
  }

  char *pixelsV;
  if (pixels->IsNull()) {
    glTexSubImage2D(targetV, levelV, xoffsetV, yoffsetV, widthV, heightV, formatV, typeV, nullptr);
  } else if (pixels->IsNumber()) {
    GLintptr offsetV = pixels->Uint32Value();
    glTexSubImage2D(targetV, levelV, xoffsetV, yoffsetV, widthV, heightV, formatV, typeV, (void *)offsetV);
  } else if ((pixelsV = (char *)getImageData(pixels)) != nullptr) {
    size_t formatSize = getFormatSize(formatV);
    size_t typeSize = getTypeSize(typeV);
    size_t pixelSize = formatSize * typeSize;
    char *pixelsV2;
    unique_ptr<char[]> pixelsV2Buffer;
    if (formatSize != 4 && !pixels->IsArrayBufferView()) {
      pixelsV2Buffer.reset(new char[widthV * heightV * pixelSize]);
      pixelsV2 = pixelsV2Buffer.get();
      reformatImageData(pixelsV2, pixelsV, formatSize * typeSize, 4 * typeSize, widthV * heightV);
    } else {
      pixelsV2 = pixelsV;
    }

    if (canvas::ImageData::getFlip() && gl->flipY && !pixels->IsArrayBufferView()) {
      unique_ptr<char[]> pixelsV3Buffer(new char[widthV * heightV * pixelSize]);
      flipImageData(pixelsV3Buffer.get(), pixelsV2, widthV, heightV, pixelSize);

      glTexSubImage2D(targetV, levelV, xoffsetV, yoffsetV, widthV, heightV, formatV, typeV, pixelsV3Buffer.get());
    } else {
      glTexSubImage2D(targetV, levelV, xoffsetV, yoffsetV, widthV, heightV, formatV, typeV, pixelsV2);
    }
  } else {
    Nan::ThrowError("Invalid texture argument");
  }

  /* if (pixels != nullptr) {
    int elementSize = num / width / height;
    for (int y = 0; y < height; y++) {
      memcpy(&(texPixels[(height - 1 - y) * width * elementSize]), &pixels[y * width * elementSize], width * elementSize);
    }
  } */
}

NAN_METHOD(WebGLRenderingContext::TexStorage2D) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
  GLenum target = info[0]->Uint32Value();
  GLint levels = info[1]->Int32Value();
  GLenum internalFormat = info[2]->Uint32Value();
  GLsizei width = info[3]->Uint32Value();
  GLsizei height = info[4]->Uint32Value();

  glTexStorage2D(target, levels, internalFormat, width, height);
}

NAN_METHOD(WebGLRenderingContext::ReadPixels) {
  GLint x = info[0]->Int32Value();
  GLint y = info[1]->Int32Value();
  GLsizei width = info[2]->Uint32Value();
  GLsizei height = info[3]->Uint32Value();
  GLenum format = info[4]->Uint32Value();
  GLenum type = info[5]->Uint32Value();
  char *pixels = (char *)getImageData(info[6]);

  if (pixels != nullptr) {
    glReadPixels(x, y, width, height, format, type, pixels);
  } else {
    Nan::ThrowError("Invalid texture argument");
  }

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetTexParameter) {
  GLenum target = info[0]->Int32Value();
  GLenum pname = info[1]->Int32Value();
  GLint value;

  glGetTexParameteriv(target, pname, &value);

  info.GetReturnValue().Set(Nan::New<Number>(value));
}

NAN_METHOD(WebGLRenderingContext::GetActiveAttrib) {
  GLint programId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  GLuint index = info[1]->Int32Value();

  char name[1024];
  GLsizei length = 0;
  GLsizei size;
  GLenum type;

  glGetActiveAttrib(programId, index, sizeof(name), &length, &size, &type, name);

  if (length > 0) {
    Local<Object> activeInfo = Nan::New<Object>();
    activeInfo->Set(JS_STR("size"), JS_INT(size));
    activeInfo->Set(JS_STR("type"), JS_INT((int)type));
    activeInfo->Set(JS_STR("name"), JS_STR(name, length));

    info.GetReturnValue().Set(activeInfo);
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(WebGLRenderingContext::GetActiveUniform) {
  GLint programId = info[0]->ToObject()->Get(JS_STR("id"))->Int32Value();
  GLuint index = info[1]->Int32Value();

  char name[1024];
  GLsizei length = 0;
  GLsizei size;
  GLenum type;

  glGetActiveUniform(programId, index, sizeof(name), &length, &size, &type, name);

  if (length > 0) {
    Local<Object> activeInfo = Nan::New<Object>();
    activeInfo->Set(JS_STR("size"), JS_INT(size));
    activeInfo->Set(JS_STR("type"), JS_INT((int)type));
    activeInfo->Set(JS_STR("name"), JS_STR(name, length));

    info.GetReturnValue().Set(activeInfo);
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(WebGLRenderingContext::GetAttachedShaders) {
  GLuint programId = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
  GLuint shaders[1024];
  GLsizei count;

  glGetAttachedShaders(programId, sizeof(shaders)/sizeof(shaders[0]), &count, shaders);

  Local<Array> shadersArr = Nan::New<Array>(count);
  for(int i = 0; i < count; i++) {
    Local<Object> shaderObject = Nan::New<Object>();
    shaderObject->Set(JS_STR("id"), JS_INT(shaders[i]));
    shadersArr->Set(i, shaderObject);
  }

  info.GetReturnValue().Set(shadersArr);
}

NAN_METHOD(WebGLRenderingContext::GetParameter) {
  GLenum name = info[0]->Int32Value();

  switch (name) {
    case GL_BLEND:
    case GL_CULL_FACE:
    case GL_DEPTH_TEST:
    case GL_DEPTH_WRITEMASK:
    case GL_DITHER:
    case GL_POLYGON_OFFSET_FILL:
    case GL_SAMPLE_COVERAGE_INVERT:
    case GL_SCISSOR_TEST:
    case GL_STENCIL_TEST:
    {
      // return a boolean
      GLboolean params;
      glGetBooleanv(name, &params);
      info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(params)));
      break;
    }
    case GL_ALPHA_BITS:
    case GL_BLEND_DST_ALPHA:
    case GL_BLEND_DST_RGB:
    case GL_BLEND_EQUATION:
    case GL_BLEND_EQUATION_ALPHA:
    // case GL_BLEND_EQUATION_RGB: // === GL_BLEND_EQUATION
    case GL_BLEND_SRC_ALPHA:
    case GL_BLEND_SRC_RGB:
    case GL_BLUE_BITS:
    case GL_CULL_FACE_MODE:
    case GL_DEPTH_BITS:
    case GL_DEPTH_FUNC:
    case GL_FRONT_FACE:
    case GL_GENERATE_MIPMAP_HINT:
    case GL_GREEN_BITS:
    case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
    case GL_IMPLEMENTATION_COLOR_READ_TYPE:
    case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
    case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
    case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
    case GL_MAX_RENDERBUFFER_SIZE:
    case GL_MAX_TEXTURE_IMAGE_UNITS:
    case GL_MAX_TEXTURE_SIZE:
    case GL_MAX_VARYING_VECTORS:
    case GL_MAX_VERTEX_ATTRIBS:
    case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
    case GL_MAX_VERTEX_UNIFORM_VECTORS:
    case GL_PACK_ALIGNMENT:
    case GL_RED_BITS:
    case GL_SAMPLE_BUFFERS:
    case GL_SAMPLES:
    case GL_STENCIL_BACK_FAIL:
    case GL_STENCIL_BACK_FUNC:
    case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
    case GL_STENCIL_BACK_PASS_DEPTH_PASS:
    case GL_STENCIL_BACK_REF:
    case GL_STENCIL_BACK_VALUE_MASK:
    case GL_STENCIL_BACK_WRITEMASK:
    case GL_STENCIL_BITS:
    case GL_STENCIL_CLEAR_VALUE:
    case GL_STENCIL_FAIL:
    case GL_STENCIL_FUNC:
    case GL_STENCIL_PASS_DEPTH_FAIL:
    case GL_STENCIL_PASS_DEPTH_PASS:
    case GL_STENCIL_REF:
    case GL_STENCIL_VALUE_MASK:
    case GL_STENCIL_WRITEMASK:
    case GL_SUBPIXEL_BITS:
    case GL_UNPACK_ALIGNMENT:
    case UNPACK_COLORSPACE_CONVERSION_WEBGL:
    {
      // return an int
      GLint param;
      glGetIntegerv(name, &param);
      info.GetReturnValue().Set(JS_INT(param));
      break;
    }
    case GL_DEPTH_CLEAR_VALUE:
    case GL_LINE_WIDTH:
    case GL_POLYGON_OFFSET_FACTOR:
    case GL_POLYGON_OFFSET_UNITS:
    case GL_SAMPLE_COVERAGE_VALUE:
    case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
    {
      // return a float
      GLfloat params;
      glGetFloatv(name, &params);
      info.GetReturnValue().Set(JS_FLOAT(params));
      break;
    }
    case GL_RENDERER:
    case GL_SHADING_LANGUAGE_VERSION:
    case GL_VENDOR:
    case GL_EXTENSIONS:
    {
      // return a string
      char *params = (char *)glGetString(name);

      if (params != NULL) {
        info.GetReturnValue().Set(JS_STR(params));
      } else {
        info.GetReturnValue().Set(Nan::Undefined());
      }

      break;
    }
    case GL_VERSION:
    {
      Local<Value> constructorName = info.This()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"));
      if (constructorName->StrictEquals(JS_STR("WebGL2RenderingContext"))) {
        info.GetReturnValue().Set(JS_STR("WebGL 2"));
      } else {
        info.GetReturnValue().Set(JS_STR("WebGL 1"));
      }
      break;
    }
    case GL_MAX_VIEWPORT_DIMS:
    {
      // return a int32[2]
      GLint params[2];
      glGetIntegerv(name, params);

      Local<Array> arr = Nan::New<Array>(2);
      arr->Set(0,JS_INT(params[0]));
      arr->Set(1,JS_INT(params[1]));
      info.GetReturnValue().Set(arr);
      break;
    }
    case GL_SCISSOR_BOX:
    case GL_VIEWPORT:
    {
      // return a int32[4]
      GLint params[4];
      glGetIntegerv(name, params);

      Local<Array> arr = Nan::New<Array>(4);
      arr->Set(0,JS_INT(params[0]));
      arr->Set(1,JS_INT(params[1]));
      arr->Set(2,JS_INT(params[2]));
      arr->Set(3,JS_INT(params[3]));
      info.GetReturnValue().Set(arr);
      break;
    }
    case GL_ALIASED_LINE_WIDTH_RANGE:
    case GL_ALIASED_POINT_SIZE_RANGE:
    case GL_DEPTH_RANGE:
    {
      // return a float[2]
      GLfloat params[2];
      glGetFloatv(name, params);

      Local<Array> arr = Nan::New<Array>(2);
      arr->Set(0,JS_FLOAT(params[0]));
      arr->Set(1,JS_FLOAT(params[1]));
      info.GetReturnValue().Set(arr);
      break;
    }
    case GL_BLEND_COLOR:
    case GL_COLOR_CLEAR_VALUE:
    {
      // return a float[4]
      GLfloat params[4];
      glGetFloatv(name, params);

      Local<Array> arr = Nan::New<Array>(4);
      arr->Set(0,JS_FLOAT(params[0]));
      arr->Set(1,JS_FLOAT(params[1]));
      arr->Set(2,JS_FLOAT(params[2]));
      arr->Set(3,JS_FLOAT(params[3]));
      info.GetReturnValue().Set(arr);
      break;
    }
    case GL_COLOR_WRITEMASK:
    {
      // return a boolean[4]
      GLboolean params[4];
      glGetBooleanv(name, params);

      Local<Array> arr = Nan::New<Array>(4);
      arr->Set(0,JS_BOOL(params[0]==1));
      arr->Set(1,JS_BOOL(params[1]==1));
      arr->Set(2,JS_BOOL(params[2]==1));
      arr->Set(3,JS_BOOL(params[3]==1));
      info.GetReturnValue().Set(arr);
      break;
    }
    case GL_ARRAY_BUFFER_BINDING:
    case GL_ELEMENT_ARRAY_BUFFER_BINDING:
    case GL_FRAMEBUFFER_BINDING:
    case GL_RENDERBUFFER_BINDING:
    case GL_TEXTURE_BINDING_2D:
    case GL_TEXTURE_BINDING_CUBE_MAP:
    case GL_ACTIVE_TEXTURE:
    case GL_CURRENT_PROGRAM:
    {
      GLint param;
      glGetIntegerv(name, &param);

      if (param != 0) {
        Local<Object> object = Nan::New<Object>();
        object->Set(JS_STR("id"), JS_INT(param));
        info.GetReturnValue().Set(object);
      } else {
        info.GetReturnValue().Set(Nan::Null());
      }
      break;
    }
    case GL_COMPRESSED_TEXTURE_FORMATS:
    {
      GLint numFormats;
      glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numFormats);

      unique_ptr<GLint[]> params(new GLint[numFormats]);
      glGetIntegerv(name, params.get());

      Local<Array> arr = Nan::New<Array>(numFormats);
      for (size_t i = 0; i < numFormats; i++) {
        arr->Set(i, JS_INT(params[i]));
      }
      info.GetReturnValue().Set(arr);
      break;
    }
    case UNPACK_FLIP_Y_WEBGL: {
      WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
      // return a boolean
      GLboolean params;
      glGetBooleanv(name, &params);
      info.GetReturnValue().Set(JS_BOOL(gl->flipY));
      break;
    }
    case UNPACK_PREMULTIPLY_ALPHA_WEBGL: {
      WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(info.This());
      // return a boolean
      GLboolean params;
      glGetBooleanv(name, &params);
      info.GetReturnValue().Set(JS_BOOL(gl->premultiplyAlpha));
      break;
    }
    default: {
      /* // return a long
      GLint params;
      glGetIntegerv(name, &params);

      info.GetReturnValue().Set(JS_INT(params)); */
      Nan::ThrowError("invalid arguments");
      break;
    }
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::GetBufferParameter) {
  GLenum target = info[0]->Int32Value();
  GLenum pname = info[1]->Int32Value();
  GLint params;

  glGetBufferParameteriv(target, pname, &params);

  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(WebGLRenderingContext::GetFramebufferAttachmentParameter) {
  GLenum target = info[0]->Int32Value();
  GLenum attachment = info[1]->Int32Value();
  GLenum pname = info[2]->Int32Value();
  GLint params;

  glGetFramebufferAttachmentParameteriv(target,attachment, pname, &params);

  info.GetReturnValue().Set(JS_INT(params));
}

NAN_METHOD(WebGLRenderingContext::GetProgramInfoLog) {
  GLuint program = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
  char Error[1024];
  int Len;

  glGetProgramInfoLog(program, sizeof(Error), &Len, Error);

  info.GetReturnValue().Set(JS_STR(Error, Len));
}

NAN_METHOD(WebGLRenderingContext::GetRenderbufferParameter) {
  int target = info[0]->Int32Value();
  int pname = info[1]->Int32Value();
  int value;

  glGetRenderbufferParameteriv(target, pname, &value);

  info.GetReturnValue().Set(JS_INT(value));
}

NAN_METHOD(WebGLRenderingContext::GetUniform) {
  GLuint program = info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value();
  GLuint location = info[1]->ToObject()->Get(JS_STR("id"))->Uint32Value();

  char name[1024];
  GLsizei length = 0;
  GLsizei size;
  GLenum type;

  glGetActiveUniform(program, location, sizeof(name), &length, &size, &type, name);

  if (length > 0) {
    GLfloat fData[16];
    GLint iData[16];
    GLuint uiData[16];
    GLdouble dData[16];
    switch(type) {
      case GL_DOUBLE:
      case GL_DOUBLE_VEC2:
      case GL_DOUBLE_VEC3:
      case GL_DOUBLE_VEC4:
        glGetUniformdv(program, location, dData);
        break;
      case GL_FLOAT:
      case GL_FLOAT_VEC2:
      case GL_FLOAT_VEC3:
      case GL_FLOAT_VEC4:
      case GL_FLOAT_MAT2:
      case GL_FLOAT_MAT3:
      case GL_FLOAT_MAT4:
      case GL_FLOAT_MAT2x3:
      case GL_FLOAT_MAT2x4:
      case GL_FLOAT_MAT3x2:
      case GL_FLOAT_MAT3x4:
      case GL_FLOAT_MAT4x2:
      case GL_FLOAT_MAT4x3:
        glGetUniformfv(program, location, fData);
        break;
      case GL_INT:
      case GL_INT_VEC2:
      case GL_INT_VEC3:
      case GL_INT_VEC4:
      case GL_BOOL:
      case GL_BOOL_VEC2:
      case GL_BOOL_VEC3:
      case GL_BOOL_VEC4:
        glGetUniformiv(program, location, iData);
        break;
      case GL_UNSIGNED_INT:
      case GL_UNSIGNED_INT_VEC2:
      case GL_UNSIGNED_INT_VEC3:
      case GL_UNSIGNED_INT_VEC4:
        glGetUniformuiv(program, location, uiData);
        break;
      case GL_SAMPLER_2D:
      case GL_SAMPLER_CUBE:
      case GL_SAMPLER_3D:
      case GL_SAMPLER_2D_SHADOW:
      case GL_SAMPLER_2D_ARRAY:
      case GL_SAMPLER_2D_ARRAY_SHADOW:
      case GL_SAMPLER_CUBE_SHADOW:
      case GL_INT_SAMPLER_2D:
      case GL_INT_SAMPLER_3D:
      case GL_INT_SAMPLER_CUBE:
      case GL_INT_SAMPLER_2D_ARRAY:
      case GL_UNSIGNED_INT_SAMPLER_2D:
      case GL_UNSIGNED_INT_SAMPLER_3D:
      case GL_UNSIGNED_INT_SAMPLER_CUBE:
      case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        glGetUniformiv(program, location, iData);
        break;
      default:
        Nan::ThrowError("Not implemented");
        break;
    }
    switch(type) {
      case GL_FLOAT: info.GetReturnValue().Set(fData[0]); break;
      case GL_FLOAT_VEC2: info.GetReturnValue().Set(createTypedArray<Float32Array>(2, fData)); break;
      case GL_FLOAT_VEC3: info.GetReturnValue().Set(createTypedArray<Float32Array>(3, fData)); break;
      case GL_FLOAT_VEC4: info.GetReturnValue().Set(createTypedArray<Float32Array>(4, fData)); break;
      case GL_FLOAT_MAT2: info.GetReturnValue().Set(createTypedArray<Float32Array>(2*2, fData)); break;
      case GL_FLOAT_MAT3: info.GetReturnValue().Set(createTypedArray<Float32Array>(3*3, fData)); break;
      case GL_FLOAT_MAT4: info.GetReturnValue().Set(createTypedArray<Float32Array>(4*4, fData)); break;
      case GL_FLOAT_MAT2x3: info.GetReturnValue().Set(createTypedArray<Float32Array>(2*3, fData)); break;
      case GL_FLOAT_MAT2x4: info.GetReturnValue().Set(createTypedArray<Float32Array>(2*4, fData)); break;
      case GL_FLOAT_MAT3x2: info.GetReturnValue().Set(createTypedArray<Float32Array>(3*2, fData)); break;
      case GL_FLOAT_MAT3x4: info.GetReturnValue().Set(createTypedArray<Float32Array>(3*4, fData)); break;
      case GL_FLOAT_MAT4x2: info.GetReturnValue().Set(createTypedArray<Float32Array>(4*2, fData)); break;
      case GL_FLOAT_MAT4x3: info.GetReturnValue().Set(createTypedArray<Float32Array>(4*3, fData)); break;
      case GL_INT: info.GetReturnValue().Set(iData[0]); break;
      case GL_INT_VEC2: info.GetReturnValue().Set(createTypedArray<Int32Array>(2, iData)); break;
      case GL_INT_VEC3: info.GetReturnValue().Set(createTypedArray<Int32Array>(3, iData)); break;
      case GL_INT_VEC4: info.GetReturnValue().Set(createTypedArray<Int32Array>(4, iData)); break;
      case GL_BOOL: info.GetReturnValue().Set(!!iData[0]); break;
      case GL_UNSIGNED_INT: info.GetReturnValue().Set(uiData[0]); break;
      case GL_UNSIGNED_INT_VEC2: info.GetReturnValue().Set(createTypedArray<Uint32Array>(2, uiData)); break;
      case GL_UNSIGNED_INT_VEC3: info.GetReturnValue().Set(createTypedArray<Uint32Array>(3, uiData)); break;
      case GL_UNSIGNED_INT_VEC4: info.GetReturnValue().Set(createTypedArray<Uint32Array>(4, uiData)); break;
      case GL_SAMPLER_2D:
      case GL_SAMPLER_CUBE:
      case GL_SAMPLER_3D:
      case GL_SAMPLER_2D_SHADOW:
      case GL_SAMPLER_2D_ARRAY:
      case GL_SAMPLER_2D_ARRAY_SHADOW:
      case GL_SAMPLER_CUBE_SHADOW:
      case GL_INT_SAMPLER_2D:
      case GL_INT_SAMPLER_3D:
      case GL_INT_SAMPLER_CUBE:
      case GL_INT_SAMPLER_2D_ARRAY:
      case GL_UNSIGNED_INT_SAMPLER_2D:
      case GL_UNSIGNED_INT_SAMPLER_3D:
      case GL_UNSIGNED_INT_SAMPLER_CUBE:
      case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
      info.GetReturnValue().Set(iData[0]);
      break;
      default:
        Nan::ThrowError("Not implemented");
        break;
    }
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(WebGLRenderingContext::GetVertexAttrib) {
  GLuint index = info[0]->Int32Value();
  GLuint pname = info[1]->Int32Value();
  GLint value;

  switch (pname) {
    case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
    case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      glGetVertexAttribiv(index, pname, &value);
      info.GetReturnValue().Set(JS_BOOL(static_cast<bool>(value)));
      break;
    case GL_VERTEX_ATTRIB_ARRAY_SIZE:
    case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
    case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      glGetVertexAttribiv(index, pname, &value);
      info.GetReturnValue().Set(JS_INT(value));
      break;
    case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      glGetVertexAttribiv(index, pname, &value);
      info.GetReturnValue().Set(JS_INT(value));
      break;
    case GL_CURRENT_VERTEX_ATTRIB: {
      float vextex_attribs[4];
      glGetVertexAttribfv(index, pname, vextex_attribs);
      Local<Array> arr = Nan::New<Array>(4);
      arr->Set(0,JS_FLOAT(vextex_attribs[0]));
      arr->Set(1,JS_FLOAT(vextex_attribs[1]));
      arr->Set(2,JS_FLOAT(vextex_attribs[2]));
      arr->Set(3,JS_FLOAT(vextex_attribs[3]));
      info.GetReturnValue().Set(arr);
      break;
    }
    default:
      Nan::ThrowError("GetVertexAttrib: Invalid Enum");
  }

  //info.GetReturnValue().Set(Nan::Undefined());
}

const char *webglExtensions[] = {
  "ANGLE_instanced_arrays",
  "EXT_blend_minmax",
  "EXT_color_buffer_float",
  "EXT_color_buffer_half_float",
  "EXT_disjoint_timer_query",
  "EXT_frag_depth",
  "EXT_sRGB",
  "EXT_shader_texture_lod",
  "EXT_texture_filter_anisotropic",
  "OES_element_index_uint",
  "OES_standard_derivatives",
  "OES_texture_float",
  "OES_texture_float_linear",
  "OES_texture_half_float",
  "OES_texture_half_float_linear",
  "OES_vertex_array_object",
  "WEBGL_color_buffer_float",
  "WEBGL_compressed_texture_astc",
  "WEBGL_compressed_texture_atc",
  "WEBGL_compressed_texture_etc",
  "WEBGL_compressed_texture_etc1",
  "WEBGL_compressed_texture_pvrtc",
  "WEBGL_compressed_texture_s3tc",
  "WEBGL_compressed_texture_s3tc_srgb",
  "WEBGL_debug_renderer_info",
  "WEBGL_debug_shaders",
  "WEBGL_depth_texture",
  "WEBGL_draw_buffers",
  "WEBGL_lose_context",
};
NAN_METHOD(WebGLRenderingContext::GetSupportedExtensions) {
  // GLint numExtensions;
  // glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

  int numExtensions = sizeof(webglExtensions)/sizeof(webglExtensions[0]);
  Local<Array> result = Nan::New<Array>(numExtensions);
  for (GLint i = 0; i < numExtensions; i++) {
    // char *extension = (char *)glGetStringi(GL_EXTENSIONS, i);
    result->Set(i, JS_STR(webglExtensions[i]));
  }

  /* for (GLint i = 0; i < numExtensions; i++) {
    char *extension = (char *)glGetStringi(GL_EXTENSIONS, i);
    result->Set(i, JS_STR(extension));
  } */

  info.GetReturnValue().Set(result);
}

// TODO GetExtension(name) return the extension name if found, should be an object...
NAN_METHOD(WebGLRenderingContext::GetExtension) {
  String::Utf8Value name(info[0]);
  char *sname = *name;

  if (
    strcmp(sname, "OES_texture_float") == 0 ||
    strcmp(sname, "OES_texture_float_linear") == 0 ||
    strcmp(sname, "OES_texture_half_float_linear") == 0 ||
    strcmp(sname, "OES_element_index_uint") == 0 ||
    strcmp(sname, "EXT_shader_texture_lod") == 0
  ) {
    info.GetReturnValue().Set(Object::New(Isolate::GetCurrent()));
  } else if (strcmp(sname, "OES_texture_half_float") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "HALF_FLOAT_OES"), Number::New(Isolate::GetCurrent(), GL_HALF_FLOAT_OES));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "OES_standard_derivatives") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "FRAGMENT_SHADER_DERIVATIVE_HINT_OES"), Number::New(Isolate::GetCurrent(), GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "WEBGL_depth_texture") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "UNSIGNED_INT_24_8_WEBGL"), Number::New(Isolate::GetCurrent(), GL_UNSIGNED_INT_24_8_OES));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "EXT_texture_filter_anisotropic") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "MAX_TEXTURE_MAX_ANISOTROPY_EXT"), Number::New(Isolate::GetCurrent(), GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "TEXTURE_MAX_ANISOTROPY_EXT"), Number::New(Isolate::GetCurrent(), GL_TEXTURE_MAX_ANISOTROPY_EXT));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "WEBGL_compressed_texture_s3tc") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGB_S3TC_DXT1_EXT"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGB_S3TC_DXT1_EXT));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_S3TC_DXT1_EXT"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_S3TC_DXT1_EXT));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_S3TC_DXT3_EXT"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_S3TC_DXT3_EXT));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_S3TC_DXT5_EXT"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_S3TC_DXT5_EXT));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "WEBGL_compressed_texture_pvrtc") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGB_PVRTC_4BPPV1_IMG"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_PVRTC_4BPPV1_IMG"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGB_PVRTC_2BPPV1_IMG"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG));
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGBA_PVRTC_2BPPV1_IMG"), Number::New(Isolate::GetCurrent(), GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "WEBGL_compressed_texture_etc1") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "COMPRESSED_RGB_ETC1_WEBGL"), Number::New(Isolate::GetCurrent(), GL_ETC1_RGB8_OES));
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "ANGLE_instanced_arrays") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());
    result->Set(String::NewFromUtf8(Isolate::GetCurrent(), "GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE"), Number::New(Isolate::GetCurrent(), GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE));
    Nan::SetMethod(result, "drawArraysInstancedANGLE", DrawArraysInstanced);
    Nan::SetMethod(result, "drawElementsInstancedANGLE", DrawElementsInstanced);
    Nan::SetMethod(result, "vertexAttribDivisorANGLE", VertexAttribDivisor);
    info.GetReturnValue().Set(result);
  } else if (strcmp(sname, "WEBGL_draw_buffers") == 0) {
    Local<Object> result = Object::New(Isolate::GetCurrent());

    Nan::SetMethod(result, "drawBuffersWEBGL", DrawBuffers);

    result->Set(JS_STR("COLOR_ATTACHMENT0_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT0));
    result->Set(JS_STR("COLOR_ATTACHMENT1_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT1));
    result->Set(JS_STR("COLOR_ATTACHMENT2_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT2));
    result->Set(JS_STR("COLOR_ATTACHMENT3_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT3));
    result->Set(JS_STR("COLOR_ATTACHMENT4_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT4));
    result->Set(JS_STR("COLOR_ATTACHMENT5_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT5));
    result->Set(JS_STR("COLOR_ATTACHMENT6_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT6));
    result->Set(JS_STR("COLOR_ATTACHMENT7_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT7));
    result->Set(JS_STR("COLOR_ATTACHMENT8_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT8));
    result->Set(JS_STR("COLOR_ATTACHMENT9_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT9));
    result->Set(JS_STR("COLOR_ATTACHMENT10_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT10));
    result->Set(JS_STR("COLOR_ATTACHMENT11_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT11));
    result->Set(JS_STR("COLOR_ATTACHMENT12_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT12));
    result->Set(JS_STR("COLOR_ATTACHMENT13_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT13));
    result->Set(JS_STR("COLOR_ATTACHMENT14_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT14));
    result->Set(JS_STR("COLOR_ATTACHMENT15_WEBGL"), JS_INT(GL_COLOR_ATTACHMENT15));

    result->Set(JS_STR("DRAW_BUFFER0_WEBGL"), JS_INT(GL_DRAW_BUFFER0));
    result->Set(JS_STR("DRAW_BUFFER1_WEBGL"), JS_INT(GL_DRAW_BUFFER1));
    result->Set(JS_STR("DRAW_BUFFER2_WEBGL"), JS_INT(GL_DRAW_BUFFER2));
    result->Set(JS_STR("DRAW_BUFFER3_WEBGL"), JS_INT(GL_DRAW_BUFFER3));
    result->Set(JS_STR("DRAW_BUFFER4_WEBGL"), JS_INT(GL_DRAW_BUFFER4));
    result->Set(JS_STR("DRAW_BUFFER5_WEBGL"), JS_INT(GL_DRAW_BUFFER5));
    result->Set(JS_STR("DRAW_BUFFER6_WEBGL"), JS_INT(GL_DRAW_BUFFER6));
    result->Set(JS_STR("DRAW_BUFFER7_WEBGL"), JS_INT(GL_DRAW_BUFFER7));
    result->Set(JS_STR("DRAW_BUFFER8_WEBGL"), JS_INT(GL_DRAW_BUFFER8));
    result->Set(JS_STR("DRAW_BUFFER9_WEBGL"), JS_INT(GL_DRAW_BUFFER9));
    result->Set(JS_STR("DRAW_BUFFER10_WEBGL"), JS_INT(GL_DRAW_BUFFER10));
    result->Set(JS_STR("DRAW_BUFFER11_WEBGL"), JS_INT(GL_DRAW_BUFFER11));
    result->Set(JS_STR("DRAW_BUFFER12_WEBGL"), JS_INT(GL_DRAW_BUFFER12));
    result->Set(JS_STR("DRAW_BUFFER13_WEBGL"), JS_INT(GL_DRAW_BUFFER13));
    result->Set(JS_STR("DRAW_BUFFER14_WEBGL"), JS_INT(GL_DRAW_BUFFER14));
    result->Set(JS_STR("DRAW_BUFFER15_WEBGL"), JS_INT(GL_DRAW_BUFFER15));

    result->Set(JS_STR("MAX_COLOR_ATTACHMENTS_WEBGL"), JS_INT(GL_MAX_COLOR_ATTACHMENTS));
    result->Set(JS_STR("MAX_DRAW_BUFFERS_WEBGL"), JS_INT(GL_MAX_DRAW_BUFFERS));

    info.GetReturnValue().Set(result);
  } else {
    info.GetReturnValue().Set(Null(Isolate::GetCurrent()));
  }
}

NAN_METHOD(WebGLRenderingContext::CheckFramebufferStatus) {
  GLenum target = info[0]->Int32Value();
  GLint ret = glCheckFramebufferStatus(target);

  info.GetReturnValue().Set(JS_INT(ret));
}

NAN_METHOD(WebGLRenderingContext::CreateVertexArray) {
  GLuint vao;
  glGenVertexArrays(1, &vao);

  Local<Object> vaoObject = Nan::New<Object>();
  vaoObject->Set(JS_STR("id"), JS_INT(vao));
  info.GetReturnValue().Set(vaoObject);
}

NAN_METHOD(WebGLRenderingContext::DeleteVertexArray) {
  GLuint vao = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glDeleteVertexArrays(1, &vao);

  // info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(WebGLRenderingContext::BindVertexArray) {
  GLuint vao = info[0]->IsObject() ? info[0]->ToObject()->Get(JS_STR("id"))->Uint32Value() : 0;

  glBindVertexArray(vao);
}

NAN_METHOD(WebGLRenderingContext::FenceSync) {
  GLenum condition = info[0]->Uint32Value();
  GLbitfield flags = info[1]->Uint32Value();

  GLsync sync = (GLsync)glFenceSync(condition, flags);
  Local<Array> syncArray = pointerToArray(sync);

  Local<Object> syncObject = Nan::New<Object>();
  syncObject->Set(JS_STR("id"), syncArray);
  info.GetReturnValue().Set(syncObject);
}

NAN_METHOD(WebGLRenderingContext::DeleteSync) {
  Local<Array> syncArray = Local<Array>::Cast(info[0]->ToObject()->Get(JS_STR("id")));
  GLsync sync = (GLsync)arrayToPointer(syncArray);

  glDeleteSync(sync);
}

NAN_METHOD(WebGLRenderingContext::ClientWaitSync) {
  Local<Array> syncArray = Local<Array>::Cast(info[0]->ToObject()->Get(JS_STR("id")));
  GLsync sync = (GLsync)arrayToPointer(syncArray);
  GLbitfield flags = info[1]->Uint32Value();
  double timeoutValue = info[2]->NumberValue();
  GLint64 timeout = *(GLint64 *)(&timeoutValue);

  GLenum ret = glClientWaitSync(sync, flags, timeout);

  info.GetReturnValue().Set(JS_INT(ret));
}

NAN_METHOD(WebGLRenderingContext::WaitSync) {
  Local<Array> syncArray = Local<Array>::Cast(info[0]->ToObject()->Get(JS_STR("id")));
  GLsync sync = (GLsync)arrayToPointer(syncArray);
  GLbitfield flags = info[1]->Uint32Value();
  double timeoutValue = info[2]->NumberValue();
  GLint64 timeout = *(GLint64 *)(&timeoutValue);

  glWaitSync(sync, flags, timeout);
}

NAN_METHOD(WebGLRenderingContext::GetSyncParameter) {
  Local<Array> syncArray = Local<Array>::Cast(info[0]->ToObject()->Get(JS_STR("id")));
  GLsync sync = (GLsync)arrayToPointer(syncArray);
  GLbitfield pname = info[1]->Uint32Value();

  GLint result = 0;
  GLsizei len;
  glGetSynciv(sync, pname, 1, &len, &result);

  info.GetReturnValue().Set(JS_INT(result));
}

Nan::Persistent<FunctionTemplate> WebGLRenderingContext::s_ct;

// WebGL2RenderingContext

WebGL2RenderingContext::WebGL2RenderingContext() {}

WebGL2RenderingContext::~WebGL2RenderingContext() {}

Handle<Object> WebGL2RenderingContext::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(WebGL2RenderingContext::New);

  s_ct.Reset(ctor);
  Local<FunctionTemplate> baseSCT = Nan::New(WebGLRenderingContext::s_ct);
  ctor->Inherit(baseSCT);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("WebGL2RenderingContext"));

  Local<Function> ctorFn = ctor->GetFunction();
  setGlConstants(ctorFn);

  return scope.Escape(ctorFn);
}

NAN_METHOD(WebGL2RenderingContext::New) {
  WebGL2RenderingContext *gl2 = new WebGL2RenderingContext();
  Local<Object> gl2Obj = info.This();
  gl2->Wrap(gl2Obj);

  info.GetReturnValue().Set(gl2Obj);
}

Nan::Persistent<FunctionTemplate> WebGL2RenderingContext::s_ct;

/* struct GLObj {
  GLObjectType type;
  GLuint obj;
  GLObj(GLObjectType type, GLuint obj) {
    this->type=type;
    this->obj=obj;
  }
};

vector<GLObj*> globjs;
static bool atExit=false;

void registerGLObj(GLObjectType type, GLuint obj) {
  globjs.push_back(new GLObj(type,obj));
}


void unregisterGLObj(GLuint obj) {
  if(atExit) return;

  vector<GLObj*>::iterator it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    if(globj->obj==obj) {
      delete globj;
      globjs.erase(it);
      break;
    }
    ++it;
  }
}

void AtExit() {
  atExit=true;
  //glFinish();

  vector<GLObj*>::iterator it;

  #ifdef LOGGING
  cout<<"WebGL AtExit() called"<<endl;
  cout<<"  # objects allocated: "<<globjs.size()<<endl;
  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *obj=*it;
    cout<<"[";
    switch(obj->type) {
    case GLOBJECT_TYPE_BUFFER: cout<<"buffer"; break;
    case GLOBJECT_TYPE_FRAMEBUFFER: cout<<"framebuffer"; break;
    case GLOBJECT_TYPE_PROGRAM: cout<<"program"; break;
    case GLOBJECT_TYPE_RENDERBUFFER: cout<<"renderbuffer"; break;
    case GLOBJECT_TYPE_SHADER: cout<<"shader"; break;
    case GLOBJECT_TYPE_TEXTURE: cout<<"texture"; break;
    };
    cout<<": "<<obj->obj<<"] ";
    ++it;
  }
  cout<<endl;
  #endif

  it = globjs.begin();
  while(globjs.size() && it != globjs.end()) {
    GLObj *globj=*it;
    GLuint obj=globj->obj;

    switch(globj->type) {
    case GLOBJECT_TYPE_PROGRAM:
      #ifdef LOGGING
      cout<<"  Destroying GL program "<<obj<<endl;
      #endif
      glDeleteProgram(obj);
      break;
    case GLOBJECT_TYPE_BUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL buffer "<<obj<<endl;
      #endif
      glDeleteBuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_FRAMEBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL frame buffer "<<obj<<endl;
      #endif
      glDeleteFramebuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_RENDERBUFFER:
      #ifdef LOGGING
      cout<<"  Destroying GL render buffer "<<obj<<endl;
      #endif
      glDeleteRenderbuffers(1,&obj);
      break;
    case GLOBJECT_TYPE_SHADER:
      #ifdef LOGGING
      cout<<"  Destroying GL shader "<<obj<<endl;
      #endif
      glDeleteShader(obj);
      break;
    case GLOBJECT_TYPE_TEXTURE:
      #ifdef LOGGING
      cout<<"  Destroying GL texture "<<obj<<endl;
      #endif
      glDeleteTextures(1,&obj);
      break;
    default:
      #ifdef LOGGING
      cout<<"  Unknown object "<<obj<<endl;
      #endif
      break;
    }
    delete globj;
    ++it;
  }

  globjs.clear();
} */
