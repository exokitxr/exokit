#include <egl/include/egl.h>
#include <webgl.h>

namespace egl {

// @Module: Window handling
NATIVEwindow *currentWindow = nullptr;
int lastX = 0, lastY = 0; // XXX track this per-window
std::unique_ptr<Nan::Persistent<Function>> eventHandler;

void NAN_INLINE(CallEmitter(int argc, Local<Value> argv[])) {
  if (eventHandler && !(*eventHandler).IsEmpty()) {
    Local<Function> eventHandlerFn = Nan::New(*eventHandler);
    eventHandlerFn->Call(Nan::Null(), argc, argv);
  }
}

void Initialize() {
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major = 4;
  EGLint minor = 0;
  eglInitialize(display, &major, &minor);
  eglBindAPI(EGL_OPENGL_API);
}

void Uninitialize() {
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglTerminate(display);
}

bool CreateRenderTarget(WebGLRenderingContext *gl, int width, int height, GLuint sharedColorTex, GLuint sharedDepthStencilTex, GLuint sharedMsColorTex, GLuint sharedMsDepthStencilTex, GLuint *pfbo, GLuint *pcolorTex, GLuint *pdepthStencilTex, GLuint *pmsFbo, GLuint *pmsColorTex, GLuint *pmsDepthStencilTex) {
  const int samples = 4;

  GLuint &fbo = *pfbo;
  GLuint &colorTex = *pcolorTex;
  GLuint &depthStencilTex = *pdepthStencilTex;
  GLuint &msFbo = *pmsFbo;
  GLuint &msColorTex = *pmsColorTex;
  GLuint &msDepthStencilTex = *pmsDepthStencilTex;

  {
    glGenFramebuffers(1, &msFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    if (!sharedMsDepthStencilTex) {
      glGenTextures(1, &msDepthStencilTex);
    } else {
      msDepthStencilTex = sharedMsDepthStencilTex;
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, width, height, true);
    // glFramebufferTexture2DMultisampleEXT(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, msDepthStencilTex, 0, samples);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

    if (!sharedMsColorTex) {
      glGenTextures(1, &msColorTex);
    } else {
      msColorTex = sharedMsColorTex;
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, true);
    // glFramebufferTexture2DMultisampleEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, msColorTex, 0, samples);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);
  }
  {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    if (!sharedDepthStencilTex) {
      glGenTextures(1, &depthStencilTex);
    } else {
      depthStencilTex = sharedDepthStencilTex;
    }
    glBindTexture(GL_TEXTURE_2D, depthStencilTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);

    if (!sharedColorTex) {
      glGenTextures(1, &colorTex);
    } else {
      colorTex = sharedColorTex;
    }
    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
  }

  bool framebufferOk = (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D)) {
    glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE)) {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE));
  } else {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP)) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP));
  } else {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }

  return framebufferOk;
}

NAN_METHOD(CreateRenderTarget) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = info[1]->Uint32Value();
  int height = info[2]->Uint32Value();
  GLuint sharedColorTex = info[3]->Uint32Value();
  GLuint sharedDepthStencilTex = info[4]->Uint32Value();
  GLuint sharedMsColorTex = info[5]->Uint32Value();
  GLuint sharedMsDepthStencilTex = info[6]->Uint32Value();

  GLuint fbo;
  GLuint colorTex;
  GLuint depthStencilTex;
  GLuint msFbo;
  GLuint msColorTex;
  GLuint msDepthStencilTex;
  bool ok = CreateRenderTarget(gl, width, height, sharedColorTex, sharedDepthStencilTex, sharedMsColorTex, sharedMsDepthStencilTex, &fbo, &colorTex, &depthStencilTex, &msFbo, &msColorTex, &msDepthStencilTex);

  Local<Value> result;
  if (ok) {
    Local<Array> array = Array::New(Isolate::GetCurrent(), 6);
    array->Set(0, JS_NUM(fbo));
    array->Set(1, JS_NUM(colorTex));
    array->Set(2, JS_NUM(depthStencilTex));
    array->Set(3, JS_NUM(msFbo));
    array->Set(4, JS_NUM(msColorTex));
    array->Set(5, JS_NUM(msDepthStencilTex));
    result = array;
  } else {
    result = Null(Isolate::GetCurrent());
  }
  info.GetReturnValue().Set(result);
}

NAN_METHOD(ResizeRenderTarget) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = info[1]->Uint32Value();
  int height = info[2]->Uint32Value();
  GLuint fbo = info[3]->Uint32Value();
  GLuint colorTex = info[4]->Uint32Value();
  GLuint depthStencilTex = info[5]->Uint32Value();
  GLuint msFbo = info[6]->Uint32Value();
  GLuint msColorTex = info[7]->Uint32Value();
  GLuint msDepthStencilTex = info[8]->Uint32Value();

  const int samples = 4;

  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_DEPTH24_STENCIL8, width, height, true);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, true);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);
  }
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D, depthStencilTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);

    glBindTexture(GL_TEXTURE_2D, colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
  }

  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D)) {
    glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE)) {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE));
  } else {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  }
  if (gl->HasTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP)) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, gl->GetTextureBinding(gl->activeTexture, GL_TEXTURE_CUBE_MAP));
  } else {
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  }
}

NAN_METHOD(DestroyRenderTarget) {
  if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber()) {
    GLuint fbo = info[0]->Uint32Value();
    GLuint tex = info[1]->Uint32Value();
    GLuint depthTex = info[2]->Uint32Value();

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
    glDeleteTextures(1, &depthTex);
  } else {
    Nan::ThrowError("DestroyRenderTarget: invalid arguments");
  }
}

NAN_METHOD(BlitFrameBuffer) {
  Local<Object> glObj = Local<Object>::Cast(info[0]);
  GLuint fbo1 = info[1]->Uint32Value();
  GLuint fbo2 = info[2]->Uint32Value();
  int sw = info[3]->Uint32Value();
  int sh = info[4]->Uint32Value();
  int dw = info[5]->Uint32Value();
  int dh = info[6]->Uint32Value();
  bool color = info[7]->BooleanValue();
  bool depth = info[8]->BooleanValue();
  bool stencil = info[9]->BooleanValue();

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo1);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);

  glBlitFramebuffer(0, 0,
    sw, sh,
    0, 0,
    dw, dh,
    (color ? GL_COLOR_BUFFER_BIT : 0) |
    (depth ? GL_DEPTH_BUFFER_BIT : 0) |
    (stencil ? GL_STENCIL_BUFFER_BIT : 0),
    (depth || stencil) ? GL_NEAREST : GL_LINEAR);

  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);
  if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->defaultFramebuffer);
  }
  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
}

NATIVEwindow *GetCurrentWindowContext() {
  return currentWindow;
}

void SetCurrentWindowContext(NATIVEwindow *window) {
  if (currentWindow != window) {
    eglMakeCurrent(window->display, EGL_NO_SURFACE, EGL_NO_SURFACE, window->context);
    currentWindow = window;
  }
}

void ReadPixels(WebGLRenderingContext *gl, unsigned int fbo, int x, int y, int width, int height, unsigned int format, unsigned int type, unsigned char *data) {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

  if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->defaultFramebuffer);
  }
}

NAN_METHOD(SetCurrentWindowContext) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  SetCurrentWindowContext(window);
}

NAN_METHOD(SetWindowTitle) {
  // nothing
}

void GetWindowSize(NATIVEwindow *window, int *width, int *height) {
  *width = window->width;
  *height = window->height;
}

NAN_METHOD(GetWindowSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int width, height;

  GetWindowSize(window, &width, &height);

  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"),JS_INT(width));
  result->Set(JS_STR("height"),JS_INT(height));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(SetWindowSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  window->width = info[1]->Uint32Value();
  window->height = info[2]->Uint32Value();
}

NAN_METHOD(SetWindowPos) {
  // nothing
}

NAN_METHOD(GetWindowPos) {
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("xpos"),JS_INT(0));
  result->Set(JS_STR("ypos"),JS_INT(0));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(GetFramebufferSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));

  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"),JS_INT(window->width));
  result->Set(JS_STR("height"),JS_INT(window->height));
  info.GetReturnValue().Set(result);
}

void *GetGLContext(NATIVEwindow *window) {
  return window->context;
}

NAN_METHOD(IconifyWindow) {
  // nothing
}

NAN_METHOD(RestoreWindow) {
  // nothing
}

NAN_METHOD(Show) {
  // nothing
}

NAN_METHOD(Hide) {
  // nothing
}

NAN_METHOD(IsVisible) {
  info.GetReturnValue().Set(JS_BOOL(true));
}

NAN_METHOD(SetFullscreen) {
  info.GetReturnValue().Set(JS_BOOL(false));
}

NAN_METHOD(ExitFullscreen) {
  // nothing
}

NAN_METHOD(Create) {
  unsigned int width = info[0]->Uint32Value();
  unsigned int height = info[1]->Uint32Value();
  bool initialVisible = info[2]->BooleanValue();
  bool hidden = info[3]->BooleanValue();
  NATIVEwindow *sharedWindow = info[4]->IsArray() ? (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[4])) : nullptr;
  WebGLRenderingContext *gl = info[5]->IsObject() ? ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[5])) : nullptr;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint config_attribs[] = {
    EGL_RED_SIZE, 5,
    EGL_GREEN_SIZE, 6,
    EGL_BLUE_SIZE, 5,
    EGL_ALPHA_SIZE, 0,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE
  };
  EGLConfig egl_config = nullptr;
  EGLint config_size = 0;
  eglChooseConfig(display, config_attribs, &egl_config, 1, &config_size);

  EGLint context_attribs[] = {
    EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
    EGL_CONTEXT_MINOR_VERSION_KHR, 2,
    EGL_NONE
  };
  EGLContext context = eglCreateContext(display, egl_config, EGL_NO_CONTEXT, context_attribs);

  eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);

  NATIVEwindow *windowHandle = new NATIVEwindow{display, context, width, height};

  GLuint framebuffers[] = {0, 0};
  GLuint framebufferTextures[] = {0, 0, 0, 0};

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

#ifdef GL_VERTEX_PROGRAM_POINT_SIZE
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif

#ifdef GL_PROGRAM_POINT_SIZE
  glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  Local<Array> result = Nan::New<Array>(8);
  result->Set(0, pointerToArray(windowHandle));
  result->Set(1, JS_INT(framebuffers[0]));
  result->Set(2, JS_INT(framebufferTextures[0]));
  result->Set(3, JS_INT(framebufferTextures[1]));
  result->Set(4, JS_INT(framebuffers[1]));
  result->Set(5, JS_INT(framebufferTextures[2]));
  result->Set(6, JS_INT(framebufferTextures[3]));
  result->Set(7, JS_INT(vao));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(Destroy) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  eglDestroyContext(window->display, window->context);
  delete window;
}

NAN_METHOD(SetEventHandler) {
  // nothing
}

NAN_METHOD(PollEvents) {
  // nothing
}

NAN_METHOD(SwapBuffers) {
  // buffer swapping is implicit on device (MLGraphicsEndFrame)
}

NAN_METHOD(GetRefreshRate) {
  info.GetReturnValue().Set(JS_INT(60));
}

NAN_METHOD(SetCursorMode) {
  // nothing
}

NAN_METHOD(SetCursorPosition) {
  // nothing
}

NAN_METHOD(GetClipboard) {
  info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(SetClipboard) {
  // nothing
}

}

///////////////////////////////////////////////////////////////////////////////
//
// bindings
//
///////////////////////////////////////////////////////////////////////////////

Local<Object> makeWindow() {
  egl::Initialize();

  Isolate *isolate = Isolate::GetCurrent();
  v8::EscapableHandleScope scope(isolate);

  Local<Object> target = Object::New(isolate);

  Nan::SetMethod(target, "create", egl::Create);
  Nan::SetMethod(target, "destroy", egl::Destroy);
  Nan::SetMethod(target, "show", egl::Show);
  Nan::SetMethod(target, "hide", egl::Hide);
  Nan::SetMethod(target, "isVisible", egl::IsVisible);
  Nan::SetMethod(target, "setFullscreen", egl::SetFullscreen);
  Nan::SetMethod(target, "exitFullscreen", egl::ExitFullscreen);
  Nan::SetMethod(target, "setWindowTitle", egl::SetWindowTitle);
  Nan::SetMethod(target, "getWindowSize", egl::GetWindowSize);
  Nan::SetMethod(target, "setWindowSize", egl::SetWindowSize);
  Nan::SetMethod(target, "setWindowPos", egl::SetWindowPos);
  Nan::SetMethod(target, "getWindowPos", egl::GetWindowPos);
  Nan::SetMethod(target, "getFramebufferSize", egl::GetFramebufferSize);
  Nan::SetMethod(target, "iconifyWindow", egl::IconifyWindow);
  Nan::SetMethod(target, "restoreWindow", egl::RestoreWindow);
  Nan::SetMethod(target, "setEventHandler", egl::SetEventHandler);
  Nan::SetMethod(target, "pollEvents", egl::PollEvents);
  Nan::SetMethod(target, "swapBuffers", egl::SwapBuffers);
  Nan::SetMethod(target, "getRefreshRate", egl::GetRefreshRate);
  Nan::SetMethod(target, "setCursorMode", egl::SetCursorMode);
  Nan::SetMethod(target, "setCursorPosition", egl::SetCursorPosition);
  Nan::SetMethod(target, "getClipboard", egl::GetClipboard);
  Nan::SetMethod(target, "setClipboard", egl::SetClipboard);
  Nan::SetMethod(target, "createRenderTarget", egl::CreateRenderTarget);
  Nan::SetMethod(target, "resizeRenderTarget", egl::ResizeRenderTarget);
  Nan::SetMethod(target, "destroyRenderTarget", egl::DestroyRenderTarget);
  Nan::SetMethod(target, "blitFrameBuffer", egl::BlitFrameBuffer);
  Nan::SetMethod(target, "setCurrentWindowContext", egl::SetCurrentWindowContext);

  return scope.Escape(target);
}
