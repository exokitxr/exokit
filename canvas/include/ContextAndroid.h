#ifndef _CANVAS_CONTEXTANDROID_H_
#define _CANVAS_CONTEXTANDROID_H_

#include "Context.h"

#include <jni.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "../src/stb_image.h"

namespace canvas {
class AndroidCache {
  public:
    AndroidCache(JNIEnv * myEnv, jobject _assetManager);
    ~AndroidCache();

  JNIEnv * getEnv() {
    JNIEnv * env = 0;
    javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    return env;
  }

  JavaVM * getJavaVM(){ return javaVM; }

  jobject & getAssetManager() {
    return assetManager;
  }
  jobject & getFactoryOptions(){
    return factoryOptions;
  }

  jmethodID paintConstructor;
  jmethodID paintSetAntiAliasMethod;
  jmethodID paintSetStyleMethod;
  jmethodID paintSetStrokeWidthMethod;
  jmethodID paintSetStrokeJoinMethod;
  jmethodID paintSetColorMethod;
  jmethodID paintSetShadowMethod;
  jmethodID paintSetTextSizeMethod;
  jmethodID paintSetShaderMethod;
  jmethodID textAlignMethod;
  jmethodID bitmapCreateMethod;
  jmethodID bitmapCreateMethod2;
  jmethodID canvasConstructor;
  jmethodID managerOpenMethod;
  jmethodID factoryDecodeMethod;
  jmethodID factoryDecodeMethod2;
  jmethodID bitmapCopyMethod;
  jmethodID pathMoveToMethod;
  jmethodID pathConstructor;
  jmethodID canvasTextDrawMethod;
  jmethodID pathLineToMethod;
  jmethodID pathCloseMethod;
  jmethodID pathArcToMethod;
  jmethodID canvasPathDrawMethod;
  jmethodID rectFConstructor;
  jmethodID rectConstructor;
  jmethodID canvasBitmapDrawMethod;
  jmethodID canvasBitmapDrawMethod2;
  jmethodID factoryDecodeByteMethod;
  jmethodID bitmapCreateScaledMethod;
  jmethodID bitmapGetWidthMethod;
  jmethodID bitmapGetHeightMethod;
  jmethodID bitmapOptionsConstructor;
  jmethodID typefaceCreator;
  jmethodID setTypefaceMethod;
  jmethodID setAlphaMethod;
  jmethodID measureTextMethod;
  jmethodID measureDescentMethod;
  jmethodID measureAscentMethod;
  jmethodID fileConstructor;
  jmethodID fileInputStreamConstructor;
  jmethodID stringConstructor;
  jmethodID stringConstructor2;
  jmethodID stringGetBytesMethod;
  jmethodID stringByteConstructor;
  jmethodID linearGradientConstructor;

  // jclass frameClass;
  jclass typefaceClass;
  jclass rectFClass;
  jclass rectClass;
  jclass canvasClass;
  jclass paintClass;
  jclass pathClass;
  jclass bitmapClass;
  jclass assetManagerClass;
  jclass factoryClass;
  jclass paintStyleClass;
  jclass alignClass;
  jclass bitmapConfigClass;
  jclass bitmapOptionsClass;
  jclass fileClass;
  jclass fileInputStreamClass;
  jclass stringClass;
  jstring charsetString;
  jclass linearGradientClass;
  jclass shaderTileModeClass;

  jfieldID field_argb_8888;
  // jfieldID field_rgb_565;
  jfieldID optionsMutableField;
  jfieldID field_alpha_8;
  jfieldID alignEnumRight;
  jfieldID alignEnumLeft;
  jfieldID alignEnumCenter;
  jfieldID paintStyleEnumStroke;
  jfieldID paintStyleEnumFill;
  jfieldID shaderTileModeMirrorField;

  jobject joinField_ROUND;

private:
  jobject factoryOptions;
  jobject assetManager;
  JavaVM * javaVM;
};

class AndroidPaint {
 public:
 AndroidPaint(AndroidCache * _cache) : cache(_cache) { }
  
  ~AndroidPaint() {
    if (is_valid) {
      cache->getEnv()->DeleteGlobalRef(obj);
    }
  }
  
  void setRenderMode(RenderMode mode) {
    create();
    auto * env = cache->getEnv();
    switch (mode) {
    case STROKE: {
      jobject styleObject = env->GetStaticObjectField(cache->paintStyleClass, cache->paintStyleEnumStroke);
      env->CallVoidMethod(obj, cache->paintSetStyleMethod, styleObject);
      env->DeleteLocalRef(styleObject);
      break;
    }
    case FILL: {
      jobject styleObject = env->GetStaticObjectField(cache->paintStyleClass, cache->paintStyleEnumFill);
      env->CallVoidMethod(obj, cache->paintSetStyleMethod, styleObject);
      env->DeleteLocalRef(styleObject);
      break;
    }
    }
  }

  void setLineWidth(float lineWidth) {
    create();
    cache->getEnv()->CallVoidMethod(obj, cache->paintSetStrokeWidthMethod, lineWidth);
  }

  void setStyle(const Style & style, float displayScale) {
    create();
    JNIEnv * env = cache->getEnv();
    jobject resultGradient = env->CallObjectMethod(obj, cache->paintSetShaderMethod, NULL);
    env->DeleteLocalRef(resultGradient);

    switch (style.getType()) {
    case Style::LINEAR_GRADIENT: {
      const std::map<float, Color> & colors = style.getColors();
      if (!colors.empty()) {
        std::map<float, Color>::const_iterator it0 = colors.begin(), it1 = colors.end();
        it1--;

        int colorOne = getAndroidColor(it0->second);
        int colorTwo = getAndroidColor(it1->second);
	float x0 = style.x0 * displayScale;
	float y0 = style.y0 * displayScale;
	float x1 = style.x1 * displayScale;
	float y1 = style.y1 * displayScale;
        jobject tileFieldObject = env->GetStaticObjectField(cache->shaderTileModeClass, cache->shaderTileModeMirrorField);
        jobject linearGradient = env->NewObject(cache->linearGradientClass, cache->linearGradientConstructor, x0, y0, x1, y1, colorOne, colorTwo, tileFieldObject);
        jobject resultGradient = env->CallObjectMethod(obj, cache->paintSetShaderMethod, linearGradient);
        env->DeleteLocalRef(tileFieldObject);
        env->DeleteLocalRef(linearGradient);
        env->DeleteLocalRef(resultGradient);
      }
      break;
    }
    default:
    case Style::SOLID:
      env->CallVoidMethod(obj, cache->paintSetColorMethod, getAndroidColor(style.color, globalAlpha));
      break;
    }
  }

  void setGlobalAlpha(float alpha) {
    if (alpha != globalAlpha) {
      create();
      globalAlpha = alpha;
      cache->getEnv()->CallVoidMethod(obj, cache->setAlphaMethod, (int) (255 * globalAlpha));
    }
  }

  void setShadow(float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor) {
    create();
    cache->getEnv()->CallVoidMethod(getObject(), cache->paintSetShadowMethod, shadowBlur, shadowOffsetX, shadowOffsetY, getAndroidColor(shadowColor, globalAlpha));
  }

  void setFont(const Font & font, float displayScale) {
    create();
    JNIEnv * env = cache->getEnv();

    if (font.size != current_font_size) {
      current_font_size = font.size;
      env->CallVoidMethod(obj, cache->paintSetTextSizeMethod, font.size * displayScale);
    }

    int textProperty = 0;
    if (font.weight.isBold()) {
      if (font.style == Font::Style::ITALIC || font.style == Font::Style::OBLIQUE) {
          textProperty = 3;
      } else {
        textProperty = 1;
      }
    } else if (font.style == Font::Style::ITALIC || font.style == Font::Style::OBLIQUE) {
      textProperty = 2;
    }

    if (font.family != current_font_family || textProperty != current_text_property) {
      current_font_family = font.family;
      current_text_property = textProperty;
      jstring jfamily = env->NewStringUTF(font.family.c_str());
      jobject typef = env->CallStaticObjectMethod(cache->typefaceClass, cache->typefaceCreator, jfamily, textProperty);
      jobject extratypef = env->CallObjectMethod(obj, cache->setTypefaceMethod, typef);
      env->DeleteLocalRef(typef);
      env->DeleteLocalRef(jfamily);
      env->DeleteLocalRef(extratypef);
    }    
  }

  void setTextAlign(TextAlign textAlign) {
    if (textAlign != currentTextAlign) {
      create();
      currentTextAlign = textAlign;
      JNIEnv * env = cache->getEnv();
      switch (textAlign) {
      case ALIGN_LEFT: {
        jobject alignObject =  env->GetStaticObjectField(cache->alignClass, cache->alignEnumLeft);
        env->CallVoidMethod(obj, cache->textAlignMethod, alignObject);
        env->DeleteLocalRef(alignObject);
        break;
      }
      case ALIGN_RIGHT: {
        jobject alignObject = env->GetStaticObjectField(cache->alignClass, cache->alignEnumRight);
        env->CallVoidMethod(obj, cache->textAlignMethod, alignObject);
        env->DeleteLocalRef(alignObject);
        break;
      }
      case ALIGN_CENTER: {
        jobject alignObject = env->GetStaticObjectField(cache->alignClass, cache->alignEnumCenter);
        env->CallVoidMethod(obj, cache->textAlignMethod, alignObject);
        env->DeleteLocalRef(alignObject);
      }
      default:
        break;
      }
    }
  }

  float measureText(const std::string & text) {    
    create();

    JNIEnv * env = cache->getEnv();

    jbyteArray array = env->NewByteArray(text.size());
    env->SetByteArrayRegion(array, 0, text.size(), (const jbyte*) text.c_str());

    jstring convertableString = (jstring) env->NewObject(cache->stringClass, cache->stringByteConstructor, array, cache->charsetString);

    jobject bytes = env->CallObjectMethod(convertableString, cache->stringGetBytesMethod);
    jobject jtext = env->NewObject(cache->stringClass, cache->stringConstructor, bytes, cache->charsetString);
    float measure = env->CallFloatMethod(obj, cache->measureTextMethod, convertableString);
    env->DeleteLocalRef(convertableString);
    env->DeleteLocalRef(bytes);
    env->DeleteLocalRef(jtext);
    env->DeleteLocalRef(array);
    return measure;
  }
  float getTextDescent() {
    create();
    return cache->getEnv()->CallFloatMethod(obj, cache->measureDescentMethod);
  }
  float getTextAscent() {
    create();
    return cache->getEnv()->CallFloatMethod(obj, cache->measureAscentMethod);
  }
  
  jobject & getObject() {
    create();
    return obj;
  }

protected:
  void create() {
    if (!is_valid) {
      is_valid = true;
      JNIEnv * env = cache->getEnv();
      auto localObj = (jobject)env->NewObject(cache->paintClass, cache->paintConstructor);
      obj = (jobject)env->NewGlobalRef(localObj);
      env->DeleteLocalRef(localObj);

      env->CallVoidMethod(obj, cache->paintSetAntiAliasMethod, JNI_TRUE);
      env->CallVoidMethod(obj, cache->paintSetStrokeJoinMethod, cache->joinField_ROUND);
    }
  }

  static int getAndroidColor(const Color & color, float globalAlpha = 1.0f) {
   return (int(color.alpha * globalAlpha * 0xff) << 24) | (int(color.red * 0xff) << 16) | (int(color.green * 0xff) << 8) | int(color.blue * 0xff);
  }

 private:
  AndroidCache * cache;
  JNIEnv * stored_env = 0;
  jobject obj;
  float globalAlpha = 1.0f;
  float current_font_size = 0;
  int current_font_property = 0;
  std::string current_font_family;
  int current_text_property;
  TextAlign currentTextAlign = ALIGN_LEFT;
  bool is_valid = false;
};

class AndroidSurface: public Surface {
public:
  friend class ContextAndroid;

  AndroidSurface(AndroidCache * _cache, unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels);
  AndroidSurface(AndroidCache * _cache, const ImageData & image);
  AndroidSurface(AndroidCache * _cache, const std::string & filename);
  AndroidSurface(AndroidCache * _cache, const unsigned char * buffer, size_t size); // Create a bitmap from bytearray

  ~AndroidSurface() {
    __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "Destructor on ContextAndroid");
    //check if these have been made global references and delete if they are.

    JNIEnv * env = cache->getEnv();

    switch (env->GetObjectRefType(bitmap)){
    case 0:
      __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "invalid ref type on bitmap deletion");
      break;
    case 1:
      env->DeleteLocalRef(bitmap);
      break;
    case 2:
      env->DeleteGlobalRef(bitmap);
      break;
    case 3:
      env->DeleteWeakGlobalRef(bitmap);
      break;
    }


    switch (env->GetObjectRefType(canvas)){
    case 0:
      __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "invalid ref type on canvas deletion");
      break;
    case 1:
      env->DeleteLocalRef(canvas);
      break;
    case 2:
      env->DeleteGlobalRef(canvas);
      break;
    case 3:
      env->DeleteWeakGlobalRef(canvas);
      break;
    }

    paint = NULL;
  }

  void renderPath(RenderMode mode, const Path2D & path, const Style & style, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) override {

    JNIEnv * env = cache->getEnv();

    checkForCanvas();
    paint.setRenderMode(mode);
    paint.setStyle(style, displayScale);
    paint.setGlobalAlpha(globalAlpha);
    paint.setShadow(shadowBlur * displayScale, shadowOffsetX * displayScale, shadowOffsetY * displayScale, shadowColor);       
    if (mode == STROKE) paint.setLineWidth(lineWidth);


#if 0
    // set font
    jobject typef = env->CallObjectMethod(cache->typefaceClass, cache->typefaceCreator, NULL, 0);
#endif

    jobject jpath = env->NewObject(cache->pathClass, cache->pathConstructor);

    for (auto pc : path.getData()) {
      switch (pc.type) {
      case PathComponent::MOVE_TO: {
        env->CallVoidMethod(jpath, cache->pathMoveToMethod, pc.x0 * displayScale, pc.y0 * displayScale);
      }
        break;
      case PathComponent::LINE_TO: {
        env->CallVoidMethod(jpath, cache->pathLineToMethod, pc.x0 * displayScale, pc.y0 * displayScale);
      }
        break;
      case PathComponent::ARC: {

        float span = 0;

#if 0
        // FIXME: If ea = 0, and sa = 2 * M_PI => span = 0
        if (!pc.anticlockwise && (pc.ea < pc.sa)) {
          span += 2 * M_PI;
        } else if (pc.anticlockwise && (pc.sa < pc.ea)) {
          span -= 2 * M_PI;
        }
#endif

        span += pc.ea - pc.sa;
        float left = pc.x0 * displayScale - pc.radius * displayScale;
        float right = pc.x0 * displayScale + pc.radius * displayScale;
        float bottom = pc.y0 * displayScale + pc.radius * displayScale;
        float top = pc.y0 * displayScale - pc.radius * displayScale;

        jobject jrect = env->NewObject(cache->rectFClass, cache->rectFConstructor, left, top, right, bottom);

        env->CallVoidMethod(jpath, cache->pathArcToMethod, jrect, (float) (pc.sa / M_PI * 180), (float) (span / M_PI * 180));
        env->DeleteLocalRef(jrect);
      }
        break;
      case PathComponent::CLOSE: {
        env->CallVoidMethod(jpath, cache->pathCloseMethod);
      }
        break;
      }
    }

    // Draw path to canvas
    env->CallVoidMethod(canvas, cache->canvasPathDrawMethod, jpath, paint.getObject());
    env->DeleteLocalRef(jpath);
  }

  void resize(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels) override {
    Surface::resize(_logical_width, _logical_height, _actual_width, _actual_height, _num_channels);
    // do resize the surface and discard the old data

    JNIEnv * env = cache->getEnv();

    __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "resize called Start");
    if (canvas) {
      env->DeleteGlobalRef(canvas);
      canvas = 0;
    }
    if (bitmap != 0) {
      jobject newBitmap = (jobject)env->CallStaticObjectMethod(cache->bitmapClass, cache->bitmapCreateScaledMethod, bitmap, _actual_width, _actual_height, JNI_FALSE);
      env->DeleteGlobalRef(bitmap);
      bitmap = env->NewGlobalRef(newBitmap);
      env->DeleteLocalRef(newBitmap);
    } else {
      __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "creating new bitmap on resize");
      jobject argbObject = createBitmapConfig(_num_channels);
      jobject localBitmap = env->CallStaticObjectMethod(cache->bitmapClass, cache->bitmapCreateMethod, _actual_width, _actual_height, argbObject);
      if (localBitmap) {
//        bitmap = (jobject) env->NewGlobalRef(localBitmap);
//        env->DeleteLocalRef(localBitmap);
        bitmap = makeGlobalReference(localBitmap);
      } else {
        bitmap = 0;
      }
      env->DeleteLocalRef(argbObject);
    }
  }

  void renderText(RenderMode mode, const Font & font, const Style & style, TextBaseline textBaseline, TextAlign textAlign, const std::string & text, const Point & p, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) override {


    JNIEnv * env = cache->getEnv();
    checkForCanvas();

    paint.setRenderMode(mode);
    paint.setFont(font, displayScale);
    paint.setStyle(style, displayScale);
    paint.setGlobalAlpha(globalAlpha);
    paint.setShadow(shadowBlur * displayScale, shadowOffsetX * displayScale, shadowOffsetY * displayScale, shadowColor);
    paint.setTextAlign(textAlign);
    if (mode == STROKE) paint.setLineWidth(lineWidth);

    jbyteArray array = env->NewByteArray(text.size());
    env->SetByteArrayRegion(array, 0, text.size(), (const jbyte*) text.c_str());

    jstring jtext = (jstring) env->NewObject(cache->stringClass, cache->stringByteConstructor, array, cache->charsetString);
    env->DeleteLocalRef(array);

    float descent = paint.getTextDescent();
    float ascent = paint.getTextAscent();
    double x = p.x * displayScale, y = p.y * displayScale;

    if (textBaseline == TextBaseline::MIDDLE || textBaseline == TextBaseline::TOP) {
      if (textBaseline == TextBaseline::MIDDLE) {
        env->CallVoidMethod(canvas, cache->canvasTextDrawMethod, jtext, x, y - (descent + ascent) / 2, paint.getObject());
      } else if (textBaseline == TextBaseline::TOP) {
        env->CallVoidMethod(canvas, cache->canvasTextDrawMethod, jtext, x, y - (descent + ascent), paint.getObject());
      }
    } else {
      env->CallVoidMethod(canvas, cache->canvasTextDrawMethod, jtext, x, y, paint.getObject());
    }

    env->DeleteLocalRef(jtext);
  }

  TextMetrics measureText(const Font & font, const std::string & text, TextBaseline textBaseline, float displayScale) override {

    paint.setFont(font, displayScale);
    
    float textWidth = paint.measureText(text);   
    float descent = paint.getTextDescent();
    float ascent = paint.getTextAscent();

    //Change ascent and descent according to baseline
    float baseline = 0;
    if (textBaseline == TextBaseline::MIDDLE) {
      baseline = (ascent + descent) / 2;
    } else if (textBaseline == TextBaseline::TOP) {
      baseline = (ascent + descent);
    }

    ascent -= baseline;
    descent -= baseline;

    return TextMetrics(textWidth / displayScale, descent / displayScale, ascent / displayScale);
  }

  void drawImage(Surface & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true) override {
    __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "DrawImage (Surface) called");
    AndroidSurface * native_surface = dynamic_cast<canvas::AndroidSurface *>(&_img);
    if (native_surface) {
      JNIEnv * env = cache->getEnv();
      checkForCanvas();
      paint.setGlobalAlpha(globalAlpha);
      paint.setShadow(shadowBlur * displayScale, shadowOffsetX * displayScale, shadowOffsetY * displayScale, shadowColor);

      jobject dstRect = env->NewObject(cache->rectFClass, cache->rectFConstructor, displayScale * p.x, displayScale * p.y, displayScale * (p.x + w), displayScale * (p.y + h));
      env->CallVoidMethod(canvas, cache->canvasBitmapDrawMethod2, native_surface->bitmap, NULL, dstRect, paint.getObject());
      env->DeleteLocalRef(dstRect);
    } else {
      auto img = native_surface->createImage(displayScale);
      drawImage(img->getData(), p, w, h, displayScale, globalAlpha, shadowBlur, shadowOffsetX, shadowOffsetY, shadowColor, clipPath, imageSmoothingEnabled);
    }
  }

  void drawImage(const ImageData & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true) override {

    __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "DrawImage (Image) called");

    JNIEnv * env = cache->getEnv();

    checkForCanvas();

    paint.setGlobalAlpha(globalAlpha);
    paint.setShadow(shadowBlur * displayScale, shadowOffsetX * displayScale, shadowOffsetY * displayScale, shadowColor);
    
    if (env->ExceptionCheck()) {
      __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "error1");
      env->ExceptionClear();
      return;
    }

    jobject drawableBitmap = imageToBitmap(_img);

    if (!drawableBitmap){
      __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "null bitmap");
      return;
    }

    // Create new Canvas from the mutable bitmap
    jobject srcRect = env->NewObject(cache->rectClass, cache->rectConstructor, 0, 0, _img.getWidth(), _img.getHeight());
    jobject dstRect = env->NewObject(cache->rectFClass, cache->rectFConstructor, displayScale * p.x, displayScale * p.y, displayScale * (p.x + w), displayScale * (p.y + h));

    __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "drawing the bitmap");
    env->CallVoidMethod(canvas, cache->canvasBitmapDrawMethod2, drawableBitmap, srcRect, dstRect, paint.getObject());
    env->DeleteLocalRef(drawableBitmap);
    env->DeleteLocalRef(srcRect);
    env->DeleteLocalRef(dstRect);
  }

  std::unique_ptr<Image> createImage(float display_scale) override;

  jobject getBitmap() { return bitmap; }

  void * lockMemory(bool write_access = false) override {
    __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "lockMemory called");

    JNIEnv * env = cache->getEnv();

    uint32_t *pixels = 0;
    AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&pixels));

    __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "pixels = %p", pixels);
    return pixels;
  }

  void releaseMemory() override {
    JNIEnv * env = cache->getEnv();
    AndroidBitmap_unlockPixels(env, bitmap);
  }

 protected:
  jobject createBitmapConfig(unsigned int num_channels);
  jobject imageToBitmap(const ImageData & _img);
  jobject imageToBitmapRGBA8(const ImageData & img);

  void checkForCanvas() {
    if (!canvas && bitmap) {
      // Create new Canvas from the mutable bitmap
      JNIEnv * env = cache->getEnv();
      jobject localCanvas = env->NewObject(cache->canvasClass, cache->canvasConstructor, bitmap);
      canvas = (jobject) env->NewGlobalRef(localCanvas);
      env->DeleteLocalRef(localCanvas);
    }
  }

  jobject makeGlobalReference(jobject localReference) {
    JNIEnv * env = cache->getEnv();
    jobject globalRefence = (jobject) env->NewGlobalRef(localReference);
    env->DeleteLocalRef(localReference);
    return globalRefence;
  }

private:
  jobject bitmap = 0;
  jobject canvas = 0;
  JNIEnv * stored_env = 0;

  AndroidCache * cache;
  AndroidPaint paint;
};

class ContextAndroid: public Context {
public:
  ContextAndroid(AndroidCache * _cache, unsigned int _width, unsigned int _height, unsigned int _num_channels, float _displayScale) :
      Context(_displayScale), cache(_cache), default_surface(_cache, _width, _height, (unsigned int) (_width * _displayScale), (unsigned int) (_height * _displayScale), _num_channels) {
  }

  std::unique_ptr<Surface> createSurface(const ImageData & image) override {
    return std::unique_ptr<Surface>(new AndroidSurface(cache, image));
  }
  std::unique_ptr<Surface> createSurface(unsigned int _width, unsigned int _height, unsigned int _num_channels) override {
    return std::unique_ptr<Surface>(new AndroidSurface(cache, _width, _height, (unsigned int) (_width * getDisplayScale()), (unsigned int) (_height * getDisplayScale()), _num_channels));
  }

  Surface & getDefaultSurface() override {
    return default_surface;
  }
  const Surface & getDefaultSurface() const override {
    return default_surface;
  }

protected:
  bool hasNativeShadows() const override {
    return true;
  }

private:
  AndroidCache * cache;
  AndroidSurface default_surface;
};


class AndroidContextFactory: public ContextFactory {
public:
  AndroidContextFactory(AAssetManager * _asset_manager, float _displayScale) :
    ContextFactory(_displayScale), asset_manager(_asset_manager) {
    stbi_set_flip_vertically_on_load(true);
  }

  std::unique_ptr<Context> createContext(unsigned int width, unsigned int height, unsigned int num_channels) override {
    return std::unique_ptr<Context>(new ContextAndroid(cache.get(), width, height, num_channels, getDisplayScale()));
  }
  std::unique_ptr<Surface> createSurface(unsigned int width, unsigned int height, unsigned int num_channels) override {
    unsigned int aw = width * getDisplayScale(), ah = height * getDisplayScale();
    return std::unique_ptr<Surface>(new AndroidSurface(cache.get(), width, height, aw, ah, num_channels));
  }

  std::unique_ptr<Image> loadImage(const std::string & filename) override;
  std::unique_ptr<Image> createImage() override;
  std::unique_ptr<Image> createImage(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels) override;

  static void initialize(JNIEnv * env, jobject & manager);

private:
  static std::shared_ptr<AndroidCache> cache;
  AAssetManager * asset_manager = 0;
};
}
;

#endif
