#include <egl/include/egl.h>
#include <windowsystem.h>
#include <webgl.h>

namespace egl {

constexpr EGLint glMajorVersion = 3;
constexpr EGLint glMinorVersion = 2;

thread_local NATIVEwindow *currentWindow = nullptr;
int lastX = 0, lastY = 0; // XXX track this per-window
std::unique_ptr<Nan::Persistent<Function>> eventHandler;

void Initialize() {
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major, minor;
  eglInitialize(display, &major, &minor);
  eglBindAPI(EGL_OPENGL_API);
}

void Uninitialize() {
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  eglTerminate(display);
}

NAN_METHOD(BlitFrameBuffer) {
  Local<Object> glObj = Local<Object>::Cast(info[0]);
  GLuint fbo1 = TO_UINT32(info[1]);
  GLuint fbo2 = TO_UINT32(info[2]);
  int sw = TO_INT32(info[3]);
  int sh = TO_INT32(info[4]);
  int dw = TO_INT32(info[5]);
  int dh = TO_INT32(info[6]);
  bool color = TO_BOOL(info[7]);
  bool depth = TO_BOOL(info[8]);
  bool stencil = TO_BOOL(info[9]);

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

void SetWindowSize(NATIVEwindow *window, int width, int height) {
  window->width = width;
  window->height = height;
}

NAN_METHOD(SetWindowSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int width = TO_INT32(info[1]);
  int height = TO_INT32(info[2]);

  SetWindowSize(window, width, height);
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

void GetFramebufferSize(NATIVEwindow *window, int *width, int *height) {
  *width = window->width;
  *height = window->height;
}

NAN_METHOD(GetFramebufferSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));

  int width, height;
  GetFramebufferSize(window, &width, &height);

  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"),JS_INT(width));
  result->Set(JS_STR("height"),JS_INT(height));
  info.GetReturnValue().Set(result);
}

void GetScreenSize(int *width, int *height) {
  *width = 2048;
  *width = 2048;
}

NAN_METHOD(GetScreenSize) {
  int width, height;
  GetScreenSize(&width, &height);

  Local<Array> result = Nan::New<Array>(2);
  result->Set(0, JS_INT(width));
  result->Set(1, JS_INT(height));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(GetDevicePixelRatio) {
  info.GetReturnValue().Set(JS_NUM(1));
}

EGLContext GetGLContext(NATIVEwindow *window) {
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

NATIVEwindow *CreateNativeWindow(unsigned int width, unsigned int height, bool visible, NATIVEwindow *sharedWindow) {
  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint config_attribs[] = {
#ifdef ANDROID
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8, // need alpha for the multi-pass timewarp compositor
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_SAMPLES, 0,
    EGL_NONE
#endif
#ifdef LUMIN
    EGL_RED_SIZE, 5,
    EGL_GREEN_SIZE, 6,
    EGL_BLUE_SIZE, 5,
    EGL_ALPHA_SIZE, 0,
    EGL_DEPTH_SIZE, 24,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE
#endif
  };
  EGLConfig egl_config = nullptr;
  EGLint config_size = 0;
  eglChooseConfig(display, config_attribs, &egl_config, 1, &config_size);

  EGLint context_attribs[] = {
    EGL_CONTEXT_MAJOR_VERSION_KHR, glMajorVersion,
    EGL_CONTEXT_MINOR_VERSION_KHR, glMinorVersion,
    EGL_NONE
  };

  EGLContext context = eglCreateContext(display, egl_config, sharedWindow ? GetGLContext(sharedWindow) : EGL_NO_CONTEXT, context_attribs);

  return new NATIVEwindow{display, context, width, height};
}

NAN_METHOD(Create3D) {
  unsigned int width = TO_UINT32(info[0]);
  unsigned int height = TO_UINT32(info[1]);
  bool initialVisible = TO_BOOL(info[2]);
  NATIVEwindow *sharedWindow = info[3]->IsArray() ? (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[3])) : nullptr;
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[4]));

  NATIVEwindow *windowHandle = CreateNativeWindow(width, height, initialVisible, sharedWindow);
  
  GLuint framebuffers[2];
  GLuint framebufferTextures[4];
  if (sharedWindow != nullptr) {
    SetCurrentWindowContext(sharedWindow);

    glGenFramebuffers(sizeof(framebuffers)/sizeof(framebuffers[0]), framebuffers);
    glGenTextures(sizeof(framebufferTextures)/sizeof(framebufferTextures[0]), framebufferTextures);
    
    SetCurrentWindowContext(windowHandle);
  } else {
    SetCurrentWindowContext(windowHandle);
    
    glGenFramebuffers(sizeof(framebuffers)/sizeof(framebuffers[0]), framebuffers);
    glGenTextures(sizeof(framebufferTextures)/sizeof(framebufferTextures[0]), framebufferTextures);
  }

  windowsystembase::InitializeLocalGlState(gl);

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

NAN_METHOD(Create2D) {
  unsigned int width = TO_UINT32(info[0]);
  unsigned int height = TO_UINT32(info[1]);
  NATIVEwindow *sharedWindow = info[2]->IsArray() ? (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[2])) : nullptr;

  NATIVEwindow *windowHandle = CreateNativeWindow(width, height, false, sharedWindow);

  GLuint tex;
  if (sharedWindow != nullptr) {
    SetCurrentWindowContext(sharedWindow);

    glGenTextures(1, &tex);
    
    SetCurrentWindowContext(windowHandle);
  } else {
    SetCurrentWindowContext(windowHandle);
    
    glGenTextures(1, &tex);
  }

  Local<Array> result = Nan::New<Array>(2);
  result->Set(0, pointerToArray(windowHandle));
  result->Set(1, JS_INT(tex));
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

  windowsystembase::Decorate(target);

  Nan::SetMethod(target, "create3d", egl::Create3D);
  Nan::SetMethod(target, "create2d", egl::Create2D);
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
  Nan::SetMethod(target, "getScreenSize", egl::GetScreenSize);
  Nan::SetMethod(target, "getDevicePixelRatio", egl::GetDevicePixelRatio);
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
  Nan::SetMethod(target, "blitFrameBuffer", egl::BlitFrameBuffer);
  Nan::SetMethod(target, "setCurrentWindowContext", egl::SetCurrentWindowContext);

  return scope.Escape(target);
}
