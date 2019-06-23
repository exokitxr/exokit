#if defined(ANDROID) && !defined(LUMIN) 

#include <browser-android.h>

extern struct android_app *androidApp;
extern JNIEnv *androidJniEnv;
extern jobject androidJniContext;

namespace browser {

const char *renderVsh = ""
"#version 300 es\n"
"\n\
in vec2 position;\n\
in vec2 uv;\n\
out vec2 vUv;\n\
\n\
void main() {\n\
  vUv = uv;\n\
  gl_Position = vec4(position.xy, 0., 1.);\n\
}\n\
";
const char *renderFsh = ""
"#version 300 es\n"
"#extension GL_OES_EGL_image_external_essl3 : require\n"
"\n\
in vec2 vUv;\n\
uniform samplerExternalOES externalTex;\n\
out vec4 fragColor;\n\
\n\
void main() {\n\
  fragColor = texture(externalTex, vUv);\n\
}\n\
";

BrowserJava::BrowserJava(JNIEnv *env, jobject context, GLuint externalTex, GLuint tex, int width, int height, const std::string &urlString) :
  env(env),
  context(context),
  externalTex(externalTex),
  tex(tex)
{
  jobject nativeActivity = androidApp->activity->clazz;
  jclass acl = env->GetObjectClass(nativeActivity);
  jmethodID getClassLoader = env->GetMethodID(acl, "getClassLoader", "()Ljava/lang/ClassLoader;");
  jobject cls = env->CallObjectMethod(nativeActivity, getClassLoader);
  jclass classLoader = env->FindClass("java/lang/ClassLoader");
  jmethodID findClass = env->GetMethodID(classLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
  jstring strClassName = env->NewStringUTF("com/webmr/exokit/ExokitWebView");
  jclass exokitWebViewClass = (jclass)(env->CallObjectMethod(cls, findClass, strClassName));

  jmethodID makeFnId = env->GetStaticMethodID(exokitWebViewClass, "make", "(Landroid/app/Activity;Landroid/content/Context;IIILjava/lang/String;)Lcom/webmr/exokit/ExokitWebView;");
  jmethodID drawFnId = env->GetMethodID(exokitWebViewClass, "draw", "()V");
  // jmethodID updateTexImageFnId = env->GetMethodID(exokitWebViewClass, "updateTexImage", "()V");
  jint externalTexInt = externalTex;
  jstring url = env->NewStringUTF(urlString.c_str());
  exokitWebView = env->CallStaticObjectMethod(exokitWebViewClass, makeFnId, androidApp->activity->clazz, context, width, height, externalTexInt, url);

  // runJsFnId = env->GetMethodID(exokitWebView, "runJs", "(Ljava/lang/String;)V");
  keyDownFnId = env->GetMethodID(exokitWebViewClass, "keyDown", "(I)V");
  keyUpFnId = env->GetMethodID(exokitWebViewClass, "keyUp", "(I)V");
  keyPressFnId = env->GetMethodID(exokitWebViewClass, "keyPress", "(I)V");
  mouseDownFnId = env->GetMethodID(exokitWebViewClass, "mouseDown", "(III)V");
  mouseUpFnId = env->GetMethodID(exokitWebViewClass, "mouseUp", "(III)V");
  clickFnId = env->GetMethodID(exokitWebViewClass, "click", "(III)V");
  mouseMoveFnId = env->GetMethodID(exokitWebViewClass, "mouseMove", "(II)V");
  mouseWheelFnId = env->GetMethodID(exokitWebViewClass, "mouseWheel", "(IIII)V");

  glGenFramebuffers(1, &renderFbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderFbo);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

  glGenVertexArrays(1, &renderVao);

  // vertex array
  glBindVertexArray(renderVao);

  // vertex shader
  renderVertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(renderVertex, 1, &renderVsh, NULL);
  glCompileShader(renderVertex);
  GLint success;
  glGetShaderiv(renderVertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(renderVertex, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    exout << "render vertex shader compilation failed:\n" << infoLog << std::endl;
    return;
  };

  // fragment shader
  renderFragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(renderFragment, 1, &renderFsh, NULL);
  glCompileShader(renderFragment);
  glGetShaderiv(renderFragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(renderFragment, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    exout << "render fragment shader compilation failed:\n" << infoLog << std::endl;
    return;
  };

  // shader program
  renderProgram = glCreateProgram();
  glAttachShader(renderProgram, renderVertex);
  glAttachShader(renderProgram, renderFragment);
  glLinkProgram(renderProgram);
  glGetProgramiv(renderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[4096];
    GLsizei length;
    glGetShaderInfoLog(renderProgram, sizeof(infoLog), &length, infoLog);
    infoLog[length] = '\0';
    exout << "render program linking failed\n" << infoLog << std::endl;
    return;
  }

  positionLocation = glGetAttribLocation(renderProgram, "position");
  if (positionLocation == -1) {
    exout << "render program failed to get attrib location for 'position'" << std::endl;
    return;
  }
  GLuint uvLocation = glGetAttribLocation(renderProgram, "uv");
  if (uvLocation == -1) {
    exout << "render program failed to get attrib location for 'uv'" << std::endl;
    return;
  }
  GLuint externalTexLocation = glGetUniformLocation(renderProgram, "externalTex");
  if (externalTexLocation == -1) {
    exout << "render program failed to get uniform location for 'externalTex'" << std::endl;
    return;
  }

  // delete the shaders as they're linked into our program now and no longer necessary
  glDeleteShader(renderVertex);
  glDeleteShader(renderFragment);

  GLuint positionBuffer;
  glGenBuffers(1, &positionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  static const float positions[] = {
    -1.0f, 1.0f,
    1.0f, 1.0f,
    -1.0f, -1.0f,
    1.0f, -1.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(positionLocation);
  glVertexAttribPointer(positionLocation, 2, GL_FLOAT, false, 0, 0);

  GLuint uvBuffer;
  glGenBuffers(1, &uvBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
  static const float uvs[] = {
    0.0f, 1.0f,
    1.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
  glEnableVertexAttribArray(uvLocation);
  glVertexAttribPointer(uvLocation, 2, GL_FLOAT, false, 0, 0);

  GLuint indexBuffer;
  glGenBuffers(1, &indexBuffer);
  static const uint16_t indices[] = {0, 2, 1, 2, 3, 1};
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  persistentMainThreadFns.push_back([this, drawFnId, /* updateTexImageFnId, */externalTex, tex, width, height, externalTexLocation]() -> void {
    this->env->CallVoidMethod(this->exokitWebView, drawFnId);
    // this->env->CallVoidMethod(this->exokitWebView, updateTexImageFnId);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->renderFbo);

    glBindVertexArray(this->renderVao);
    glUseProgram(this->renderProgram);

    glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, externalTex);
    glUniform1i(externalTexLocation, 0);

    glViewport(0, 0, width, height);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
  });
}
BrowserJava::~BrowserJava() {
  glDeleteFramebuffers(1, &renderFbo);
  glDeleteVertexArrays(1, &renderVao);
  glDeleteProgram(renderProgram);
}

void BrowserJava::KeyDown(int keyCode) {
  QueueInMainThread([this, keyCode]() -> void {
    env->CallVoidMethod(exokitWebView, keyDownFnId, keyCode);
  });
}
void BrowserJava::KeyUp(int keyCode) {
  QueueInMainThread([this, keyCode]() -> void {
    env->CallVoidMethod(exokitWebView, keyUpFnId, keyCode);
  });
}
void BrowserJava::KeyPress(int keyCode) {
  QueueInMainThread([this, keyCode]() -> void {
    env->CallVoidMethod(exokitWebView, keyPressFnId, keyCode);
  });
}
void BrowserJava::MouseDown(int x, int y, int button) {
  QueueInMainThread([this, x, y, button]() -> void {
    env->CallVoidMethod(exokitWebView, mouseDownFnId, x, y, button);
  });
}
void BrowserJava::MouseUp(int x, int y, int button) {
  QueueInMainThread([this, x, y, button]() -> void {
    env->CallVoidMethod(exokitWebView, mouseUpFnId, x, y, button);
  });
}
void BrowserJava::Click(int x, int y, int button) {
  QueueInMainThread([this, x, y, button]() -> void {
    env->CallVoidMethod(exokitWebView, clickFnId, x, y, button);
  });
}
void BrowserJava::MouseMove(int x, int y) {
  QueueInMainThread([this, x, y]() -> void {
    env->CallVoidMethod(exokitWebView, mouseMoveFnId, x, y);
  });
}
void BrowserJava::MouseWheel(int x, int y, int deltaX, int deltaY) {
  QueueInMainThread([this, x, y, deltaX, deltaY]() -> void {
    env->CallVoidMethod(exokitWebView, mouseWheelFnId, x, y, deltaX, deltaY);
  });
}

Browser::Browser(BrowserJava *browser) : browser(browser) {}
Browser::~Browser() {}

NAN_METHOD(Browser::New) {
  if (
    info[0]->IsObject() &&
    info[1]->IsNumber() &&
    info[2]->IsNumber() &&
    info[3]->IsString()
  ) {
    WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
    int width = TO_INT32(info[1]);
    int height = TO_INT32(info[2]);
    Nan::Utf8String urlValue(Local<String>::Cast(info[3]));
    std::string url(*urlValue, urlValue.length());

    GLuint externalTex;
    GLuint tex;
    {
      windowsystem::SetCurrentWindowContext(gl->windowHandle);

      glGenTextures(1, &externalTex);

      glBindTexture( GL_TEXTURE_EXTERNAL_OES, externalTex );
      glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      glBindTexture( GL_TEXTURE_EXTERNAL_OES, 0 );

      glGenTextures(1, &tex);
      glBindTexture( GL_TEXTURE_2D, tex );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
      glBindTexture( GL_TEXTURE_2D, 0 );
    }

    BrowserJava *browserJava;
    RunInMainThread([&]() -> void {
      browserJava = new BrowserJava(androidJniEnv, androidJniContext, externalTex, tex, width, height, url);
    });
    Browser *browser = new Browser(browserJava);
    Local<Object> browserObj = info.This();
    browser->Wrap(browserObj);
    Nan::SetAccessor(browserObj, JS_STR("texture"), TextureGetter);

    return info.GetReturnValue().Set(browserObj);
  } else {
    return Nan::ThrowError("Browser::New: invalid arguments");
  }
}

NAN_GETTER(Browser::TextureGetter) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());

  Local<Object> textureObj = Nan::New<Object>();
  textureObj->Set(JS_STR("id"), JS_INT(browser->browser->tex));
  info.GetReturnValue().Set(textureObj);
}

NAN_METHOD(Browser::KeyDown) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int keyCode = TO_INT32(info[0]);
  browser->browser->KeyDown(keyCode);
}
NAN_METHOD(Browser::KeyUp) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int keyCode = TO_INT32(info[0]);
  browser->browser->KeyUp(keyCode);
}
NAN_METHOD(Browser::KeyPress) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int keyCode = TO_INT32(info[0]);
  browser->browser->KeyPress(keyCode);
}
NAN_METHOD(Browser::MouseDown) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int x = TO_INT32(info[0]);
  int y = TO_INT32(info[1]);
  int button = TO_INT32(info[2]);
  browser->browser->MouseDown(x, y, button);
}
NAN_METHOD(Browser::MouseUp) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int x = TO_INT32(info[0]);
  int y = TO_INT32(info[1]);
  int button = TO_INT32(info[2]);
  browser->browser->MouseUp(x, y, button);
}
NAN_METHOD(Browser::Click) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int x = TO_INT32(info[0]);
  int y = TO_INT32(info[1]);
  int button = TO_INT32(info[2]);
  browser->browser->Click(x, y, button);
}
NAN_METHOD(Browser::MouseMove) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int x = TO_INT32(info[0]);
  int y = TO_INT32(info[1]);
  browser->browser->MouseMove(x, y);
}
NAN_METHOD(Browser::MouseWheel) {
  Browser *browser = ObjectWrap::Unwrap<Browser>(info.This());
  int x = TO_INT32(info[0]);
  int y = TO_INT32(info[1]);
  int deltaX = TO_INT32(info[2]);
  int deltaY = TO_INT32(info[3]);
  browser->browser->MouseWheel(x, y, deltaX, deltaY);
}

Local<Object> Browser::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Browser"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "sendKeyDown", KeyDown);
  Nan::SetMethod(proto, "sendKeyUp", KeyUp);
  Nan::SetMethod(proto, "sendKeyPress", KeyPress);
  Nan::SetMethod(proto, "sendMouseDown", MouseDown);
  Nan::SetMethod(proto, "sendMouseUp", MouseUp);
  Nan::SetMethod(proto, "sendClick", Click);
  Nan::SetMethod(proto, "sendMouseMove", MouseMove);
  Nan::SetMethod(proto, "sendMouseWheel", MouseWheel);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();
  Nan::SetMethod(ctorFn, "pollEvents", PollEvents);

  return scope.Escape(ctorFn);
}

std::mutex mainThreadFnMutex;
std::vector<std::function<void()>> mainThreadFns;
std::vector<std::function<void()>> persistentMainThreadFns;
void RunInMainThread(std::function<void()> fn) {
  uv_sem_t sem;
  uv_sem_init(&sem, 0);

  {
    std::lock_guard<mutex> lock(mainThreadFnMutex);

    mainThreadFns.push_back([&]() -> void {
      fn();

      uv_sem_post(&sem);
    });
  }

  uv_sem_wait(&sem);
  uv_sem_destroy(&sem);
}
void QueueInMainThread(std::function<void()> fn) {
  std::lock_guard<mutex> lock(mainThreadFnMutex);

  mainThreadFns.push_back(fn);
}
NAN_METHOD(PollEvents) {
  std::vector<std::function<void()>> localMainThreadFns;
  {
    std::lock_guard<mutex> lock(mainThreadFnMutex);

    localMainThreadFns = std::move(mainThreadFns);
    mainThreadFns.clear();
  }
  for (auto iter = localMainThreadFns.begin(); iter != localMainThreadFns.end(); iter++) {
    std::function<void()> &fn = *iter;
    fn();
  }

  for (auto iter = persistentMainThreadFns.begin(); iter != persistentMainThreadFns.end(); iter++) {
    std::function<void()> &fn = *iter;
    fn();
  }
}

}

#endif
