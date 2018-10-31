#include <egl/include/egl.h>
#include <windowsystem.h>
#include <webgl.h>

namespace egl {

// @Module: Window handling
thread_local NATIVEwindow *currentWindow = nullptr;
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

void SetWindowSize(NATIVEwindow *window, int width, int height) {
  window->width = width;
  window->height = height;
}

NAN_METHOD(SetWindowSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int width = info[1]->Int32Value();
  int height = info[2]->Int32Value();

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

NAN_METHOD(GetFramebufferSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));

  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"),JS_INT(window->width));
  result->Set(JS_STR("height"),JS_INT(window->height));
  info.GetReturnValue().Set(result);
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

NATIVEwindow *CreateGLWindow(unsigned int width, unsigned int height, bool visible, NATIVEwindow *sharedWindow) {
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
  
  EGLContext context = eglCreateContext(display, egl_config, sharedWindow ? GetGLContext(sharedWindow) : EGL_NO_CONTEXT, context_attribs);

  eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);

  return new NATIVEwindow{display, context, width, height};
}

NAN_METHOD(Create) {
  unsigned int width = info[0]->Uint32Value();
  unsigned int height = info[1]->Uint32Value();
  bool initialVisible = info[2]->BooleanValue();
  bool hidden = info[3]->BooleanValue();
  NATIVEwindow *sharedWindow = info[4]->IsArray() ? (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[4])) : nullptr;
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[5]));
  WebGLRenderingContext *sharedGl = info[6]->IsObject() ? ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[6])) : nullptr;

  GLuint framebuffers[] = {0, 0};
  GLuint framebufferTextures[] = {0, 0, 0, 0};
  bool shared = hidden && sharedWindow != nullptr && sharedGl != nullptr;
  if (shared) {
    SetCurrentWindowContext(sharedWindow);

    glGenFramebuffers(sizeof(framebuffers)/sizeof(framebuffers[0]), framebuffers);
    glGenTextures(sizeof(framebufferTextures)/sizeof(framebufferTextures[0]), framebufferTextures);
  }

  NATIVEwindow *windowHandle = CreateNativeWindow(width, height, initialVisible, shared ? sharedWindow : nullptr);

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
  Nan::SetMethod(target, "blitFrameBuffer", egl::BlitFrameBuffer);
  Nan::SetMethod(target, "setCurrentWindowContext", egl::SetCurrentWindowContext);

  return scope.Escape(target);
}
