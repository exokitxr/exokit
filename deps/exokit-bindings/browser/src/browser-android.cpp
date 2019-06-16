#if defined(ANDROID) && !defined(LUMIN) 

#include <browser-android.h>

extern struct android_app *androidApp;
extern JNIEnv *androidJniEnv;
extern jobject androidJniContext;

namespace browser {

Browser::Browser(WebGLRenderingContext *gl, int width, int height, const std::string &urlString) {
  {
    windowsystem::SetCurrentWindowContext(gl->windowHandle);

    glGenTextures(1, &tex);

    glBindTexture( GL_TEXTURE_EXTERNAL_OES, tex );
    // Notice the use of GL_TEXTURE_2D for texture creation
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glBindTexture( GL_TEXTURE_EXTERNAL_OES, 0 );
  }

  {
    JNIEnv *env = androidJniEnv;
    jobject context = androidJniContext;

    jclass exokitWebViewClass = env->FindClass("com/webmr/ExokitWebView");
    // jmethodID constructor = env->GetMethodID(exokitWebViewClass, "<init>", "(Landroid/content/Context;)V");
    // jobject exokitWebView = env->NewObject(exokitWebViewClass, constructor, context);
    jmethodID makeFnId = env->GetStaticMethodID(exokitWebViewClass, "make", "(Landroid/app/Activity;Landroid/content/Context;I;I;I;Ljava/lang/String;)Lcom/webmr/ExokitWebView;");
    jint colorTex = 0;
    jstring url = env->NewStringUTF(urlString.c_str());
    jobject exokitWebView = env->CallStaticObjectMethod(exokitWebViewClass, makeFnId, androidApp->activity->clazz, context, width, height, colorTex, url);
    (void)exokitWebView; // XXX
  }
}

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

    Browser *browser = new Browser(gl, width, height, url);
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
  textureObj->Set(JS_STR("id"), JS_INT(browser->tex));
  info.GetReturnValue().Set(textureObj);
}

Local<Object> Browser::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Browser"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  // Nan::SetMethod(proto, "resize", Resize);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();
  // Nan::SetMethod(ctorFn, "updateAll", UpdateAll);

  return scope.Escape(ctorFn);
}

}

#endif
