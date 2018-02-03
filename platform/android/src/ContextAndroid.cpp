#include "ContextAndroid.h"

#include <errno.h>
#include <cassert>

using namespace std;
using namespace canvas;

AndroidCache::AndroidCache(JNIEnv * myEnv, jobject _assetManager) {
  myEnv->GetJavaVM(&javaVM);

  assetManager = myEnv->NewGlobalRef(_assetManager);

  // frameClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("com/sometrik/framework/FrameWork"));
  typefaceClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Typeface"));
  canvasClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Canvas"));
  assetManagerClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/content/res/AssetManager"));
  factoryClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/BitmapFactory"));
  bitmapClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Bitmap"));
  paintClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Paint"));
  pathClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Path"));
  paintStyleClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Paint$Style"));
  alignClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Paint$Align"));
  bitmapConfigClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Bitmap$Config"));

  field_argb_8888 = myEnv->GetStaticFieldID(bitmapConfigClass, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
  // field_rgb_565 = myEnv->GetStaticFieldID(bitmapConfigClass, "RGB_565", "Landroid/graphics/Bitmap$Config;");
  field_alpha_8 = myEnv->GetStaticFieldID(bitmapConfigClass, "ALPHA_8", "Landroid/graphics/Bitmap$Config;");
  rectFClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/RectF"));
  rectClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Rect"));
  bitmapOptionsClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/BitmapFactory$Options"));
  fileClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("java/io/File"));
  fileInputStreamClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("java/io/FileInputStream"));
  stringClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("java/lang/String"));
  charsetString = (jstring) myEnv->NewGlobalRef(myEnv->NewStringUTF("UTF-8"));
  linearGradientClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/LinearGradient"));
  shaderTileModeClass = (jclass) myEnv->NewGlobalRef(myEnv->FindClass("android/graphics/Shader$TileMode"));
  shaderTileModeMirrorField = myEnv->GetStaticFieldID(shaderTileModeClass, "MIRROR", "Landroid/graphics/Shader$TileMode;");

  measureAscentMethod = myEnv->GetMethodID(paintClass, "ascent", "()F");
  measureDescentMethod = myEnv->GetMethodID(paintClass, "descent", "()F");
  measureTextMethod = myEnv->GetMethodID(paintClass, "measureText", "(Ljava/lang/String;)F");
  setAlphaMethod = myEnv->GetMethodID(paintClass, "setAlpha", "(I)V");
  setTypefaceMethod = myEnv->GetMethodID(paintClass, "setTypeface", "(Landroid/graphics/Typeface;)Landroid/graphics/Typeface;");
  textAlignMethod = myEnv->GetMethodID(paintClass, "setTextAlign", "(Landroid/graphics/Paint$Align;)V");
  paintSetColorMethod = myEnv->GetMethodID(paintClass, "setColor", "(I)V");
  paintSetStyleMethod = myEnv->GetMethodID(paintClass, "setStyle", "(Landroid/graphics/Paint$Style;)V");
  paintSetStrokeWidthMethod = myEnv->GetMethodID(paintClass, "setStrokeWidth", "(F)V");
  paintSetStrokeJoinMethod = myEnv->GetMethodID(paintClass, "setStrokeJoin", "(Landroid/graphics/Paint$Join;)V");
  paintConstructor = myEnv->GetMethodID(paintClass, "<init>", "()V");
  paintSetAntiAliasMethod = myEnv->GetMethodID(paintClass, "setAntiAlias", "(Z)V");
  textAlignMethod = myEnv->GetMethodID(paintClass, "setTextAlign", "(Landroid/graphics/Paint$Align;)V");
  paintSetShaderMethod = myEnv->GetMethodID(paintClass, "setShader", "(Landroid/graphics/Shader;)Landroid/graphics/Shader;");

  typefaceCreator = myEnv->GetStaticMethodID(typefaceClass, "create", "(Ljava/lang/String;I)Landroid/graphics/Typeface;");
  managerOpenMethod = myEnv->GetMethodID(assetManagerClass, "open", "(Ljava/lang/String;)Ljava/io/InputStream;");
  bitmapCreateMethod = myEnv->GetStaticMethodID(bitmapClass, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
  bitmapCreateMethod2 = myEnv->GetStaticMethodID(bitmapClass, "createBitmap", "([IIILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
  bitmapCreateScaledMethod = myEnv->GetStaticMethodID(bitmapClass, "createScaledBitmap", "(Landroid/graphics/Bitmap;IIZ)Landroid/graphics/Bitmap;");
  canvasConstructor = myEnv->GetMethodID(canvasClass, "<init>", "(Landroid/graphics/Bitmap;)V");
  factoryDecodeMethod = myEnv->GetStaticMethodID(factoryClass, "decodeStream", "(Ljava/io/InputStream;)Landroid/graphics/Bitmap;");
  factoryDecodeMethod2 = myEnv->GetStaticMethodID(factoryClass, "decodeStream", "(Ljava/io/InputStream;Landroid/graphics/Rect;Landroid/graphics/BitmapFactory$Options;)Landroid/graphics/Bitmap;");
  bitmapCopyMethod = myEnv->GetMethodID(bitmapClass, "copy", "(Landroid/graphics/Bitmap$Config;Z)Landroid/graphics/Bitmap;");
  pathMoveToMethod = myEnv->GetMethodID(pathClass, "moveTo", "(FF)V");
  pathConstructor = myEnv->GetMethodID(pathClass, "<init>", "()V");
  canvasTextDrawMethod = myEnv->GetMethodID(canvasClass, "drawText", "(Ljava/lang/String;FFLandroid/graphics/Paint;)V");
  pathLineToMethod = myEnv->GetMethodID(pathClass, "lineTo", "(FF)V");
  pathCloseMethod = myEnv->GetMethodID(pathClass, "close", "()V");
  pathArcToMethod = myEnv->GetMethodID(pathClass, "arcTo", "(Landroid/graphics/RectF;FF)V");
  canvasPathDrawMethod = myEnv->GetMethodID(canvasClass, "drawPath", "(Landroid/graphics/Path;Landroid/graphics/Paint;)V");
  rectFConstructor = myEnv->GetMethodID(rectFClass, "<init>", "(FFFF)V");
  rectConstructor = myEnv->GetMethodID(rectClass, "<init>", "(IIII)V");
  paintSetShadowMethod = myEnv->GetMethodID(paintClass, "setShadowLayer", "(FFFI)V");
  paintSetTextSizeMethod = myEnv->GetMethodID(paintClass, "setTextSize", "(F)V");
  canvasBitmapDrawMethod = myEnv->GetMethodID(canvasClass, "drawBitmap", "(Landroid/graphics/Bitmap;FFLandroid/graphics/Paint;)V");
  canvasBitmapDrawMethod2 = myEnv->GetMethodID(canvasClass, "drawBitmap", "(Landroid/graphics/Bitmap;Landroid/graphics/Rect;Landroid/graphics/RectF;Landroid/graphics/Paint;)V");
  factoryDecodeByteMethod = myEnv->GetStaticMethodID(factoryClass, "decodeByteArray", "([BII)Landroid/graphics/Bitmap;");
  bitmapGetWidthMethod = myEnv->GetMethodID(bitmapClass, "getWidth", "()I");
  bitmapGetHeightMethod = myEnv->GetMethodID(bitmapClass, "getHeight", "()I");
  bitmapOptionsConstructor = myEnv->GetMethodID(bitmapOptionsClass, "<init>", "()V");
  fileConstructor = myEnv->GetMethodID(fileClass, "<init>", "(Ljava/lang/String;)V");
  fileInputStreamConstructor = myEnv->GetMethodID(fileInputStreamClass, "<init>", "(Ljava/io/File;)V");
  stringConstructor = myEnv->GetMethodID(stringClass, "<init>", "([BLjava/lang/String;)V");
  stringGetBytesMethod = myEnv->GetMethodID(stringClass, "getBytes", "()[B");
  stringConstructor2 = myEnv->GetMethodID(stringClass, "<init>", "()V");
  stringByteConstructor = myEnv->GetMethodID(stringClass, "<init>", "([BLjava/lang/String;)V");
  linearGradientConstructor = myEnv->GetMethodID(linearGradientClass, "<init>", "(FFFFIILandroid/graphics/Shader$TileMode;)V");

  optionsMutableField = myEnv->GetFieldID(bitmapOptionsClass, "inMutable", "Z");
  alignEnumRight = myEnv->GetStaticFieldID(alignClass, "RIGHT", "Landroid/graphics/Paint$Align;");
  alignEnumLeft = myEnv->GetStaticFieldID(alignClass, "LEFT", "Landroid/graphics/Paint$Align;");
  alignEnumCenter = myEnv->GetStaticFieldID(alignClass, "CENTER", "Landroid/graphics/Paint$Align;");
  factoryOptions = myEnv->NewGlobalRef(myEnv->NewObject(bitmapOptionsClass, bitmapOptionsConstructor));
  myEnv->SetBooleanField(factoryOptions, optionsMutableField, JNI_TRUE);

  paintStyleEnumStroke = myEnv->GetStaticFieldID(myEnv->FindClass("android/graphics/Paint$Style"), "STROKE", "Landroid/graphics/Paint$Style;");
  paintStyleEnumFill = myEnv->GetStaticFieldID(myEnv->FindClass("android/graphics/Paint$Style"), "FILL", "Landroid/graphics/Paint$Style;");

  auto joinClass = (jclass) myEnv->FindClass("android/graphics/Paint$Join");
  joinField_ROUND = myEnv->NewGlobalRef(myEnv->GetStaticObjectField(joinClass, myEnv->GetStaticFieldID(joinClass, "ROUND", "Landroid/graphics/Paint$Join;")));
  myEnv->DeleteLocalRef(joinClass);
}

AndroidCache::~AndroidCache() {
  auto myEnv = getEnv();
  myEnv->DeleteGlobalRef(factoryOptions);
  myEnv->DeleteGlobalRef(assetManager);
  myEnv->DeleteGlobalRef(typefaceClass);
  myEnv->DeleteGlobalRef(rectFClass);
  myEnv->DeleteGlobalRef(rectClass);
  myEnv->DeleteGlobalRef(canvasClass);
  myEnv->DeleteGlobalRef(paintClass);
  myEnv->DeleteGlobalRef(pathClass);
  myEnv->DeleteGlobalRef(bitmapClass);
  myEnv->DeleteGlobalRef(assetManagerClass);
  myEnv->DeleteGlobalRef(factoryClass);
  myEnv->DeleteGlobalRef(paintStyleClass);
  myEnv->DeleteGlobalRef(alignClass);
  myEnv->DeleteGlobalRef(bitmapConfigClass);
  myEnv->DeleteGlobalRef(bitmapOptionsClass);
  myEnv->DeleteGlobalRef(fileClass);
  myEnv->DeleteGlobalRef(fileInputStreamClass);
  myEnv->DeleteGlobalRef(stringClass);
  myEnv->DeleteGlobalRef(charsetString);
  myEnv->DeleteGlobalRef(linearGradientClass);

  myEnv->DeleteGlobalRef(joinField_ROUND);

}

AndroidSurface::AndroidSurface(AndroidCache * _cache, unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels)
  : Surface(_logical_width, _logical_height, _actual_width, _actual_height, _num_channels), cache(_cache), paint(_cache) {
  // creates an empty canvas

  if (_actual_width && _actual_height) {
    __android_log_print(ANDROID_LOG_INFO, "Sometrik", "AndroidSurface widthheight constructor called with width : height %u : %u", _actual_width, _actual_height);
    JNIEnv * env = cache->getEnv();

    // set bitmap config according to internalformat
    jobject argbObject = createBitmapConfig(_num_channels);
    
    jobject localBitmap = env->CallStaticObjectMethod(cache->bitmapClass, cache->bitmapCreateMethod, _actual_width, _actual_height, argbObject);
    if (localBitmap) {
      bitmap = (jobject) env->NewGlobalRef(localBitmap);
      env->DeleteLocalRef(localBitmap);
    } else {
      bitmap = 0;
    }
    env->DeleteLocalRef(argbObject);
  }
}

AndroidSurface::AndroidSurface(AndroidCache * _cache, const ImageData & image)
  : Surface(image.getWidth(), image.getHeight(), image.getWidth(), image.getHeight(), image.getNumChannels() == 1 ? 1 : 4), cache(_cache), paint(_cache) {
  JNIEnv * env = cache->getEnv();

  // creates a surface with width, height and contents from image
  jobject localBitmap = (jobject) env->NewGlobalRef(imageToBitmap(image));
  if (localBitmap) {
    bitmap = (jobject) env->NewGlobalRef(localBitmap);
    env->DeleteLocalRef(localBitmap);
  }
}

AndroidSurface::AndroidSurface(AndroidCache * _cache, const std::string & filename)
  : Surface(0, 0, 0, 0, 0), cache(_cache), paint(_cache) {
  __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "Surface filename constructor");

  JNIEnv * env = cache->getEnv();

  // Get inputStream from the picture(filename)
  jstring jfilename = env->NewStringUTF(filename.c_str());
  jobject inputStream = env->CallObjectMethod(cache->getAssetManager(), cache->managerOpenMethod, jfilename);
  env->DeleteLocalRef(jfilename);
  
  // Create a bitmap from the inputStream
  jobject localBitmap = env->CallStaticObjectMethod(cache->factoryClass, cache->factoryDecodeMethod2, inputStream, NULL, cache->getFactoryOptions());
  bitmap = (jobject) env->NewGlobalRef(localBitmap);
  env->DeleteLocalRef(localBitmap);
  
  int bitmapWidth = env->CallIntMethod(bitmap, cache->bitmapGetWidthMethod);
  int bitmapHeigth = env->CallIntMethod(bitmap, cache->bitmapGetHeightMethod);
  Surface::resize(bitmapWidth, bitmapHeigth, bitmapWidth, bitmapHeigth, 4);

  env->DeleteLocalRef(inputStream);
}

AndroidSurface::AndroidSurface(AndroidCache * _cache, const unsigned char * buffer, size_t size)
  : Surface(0, 0, 0, 0, 0), cache(_cache), paint(_cache) {

  JNIEnv * env = cache->getEnv();
  int arraySize = size;
  
  jbyteArray array = env->NewByteArray(arraySize);
  env->SetByteArrayRegion(array, 0, arraySize, (const jbyte*) buffer);
  jobject firstBitmap = env->CallStaticObjectMethod(cache->factoryClass, cache->factoryDecodeByteMethod, array, 0, arraySize);
  
  // make this with factory options instead
  jobject argbObject = createBitmapConfig(4);
  jobject localBitmap = env->CallObjectMethod(firstBitmap, cache->bitmapCopyMethod, argbObject, JNI_TRUE);
  bitmap = (jobject) env->NewGlobalRef(localBitmap);
  env->DeleteLocalRef(localBitmap);
  
  int bitmapWidth = env->CallIntMethod(bitmap, cache->bitmapGetWidthMethod);
  int bitmapHeigth = env->CallIntMethod(bitmap, cache->bitmapGetHeightMethod);
  Surface::resize(bitmapWidth, bitmapHeigth, bitmapWidth, bitmapHeigth, 4);
  
  env->DeleteLocalRef(argbObject);
  env->DeleteLocalRef(firstBitmap);
  env->DeleteLocalRef(array);
}

jobject
AndroidSurface::createBitmapConfig(unsigned int num_channels) {
  JNIEnv * env = cache->getEnv();
  if (num_channels == 1) {
    return env->GetStaticObjectField(cache->bitmapConfigClass, cache->field_alpha_8);
  } else {
    return env->GetStaticObjectField(cache->bitmapConfigClass, cache->field_argb_8888);
  }
}

jobject
AndroidSurface::imageToBitmap(const ImageData & img) {
  JNIEnv * env = cache->getEnv();

  unsigned int n = img.getWidth() * img.getHeight();
  
  jintArray jarray = env->NewIntArray(n);
  if (img.getNumChannels() == 4) {
    env->SetIntArrayRegion(jarray, 0, n, (const jint*)img.getData());
  } else {
    const unsigned char * input_data = img.getData();
    unique_ptr<unsigned int[]> tmp(new unsigned int[n]);
    for (unsigned int i = 0; i < n; i++) {
      unsigned char r = *input_data++;
      unsigned char g = img.getNumChannels() >= 2 ? *input_data++ : r;
      unsigned char b = img.getNumChannels() >= 3 ? *input_data++ : g;
      unsigned char a = img.getNumChannels() >= 4 ? *input_data++ : 0xff;
      tmp[i] = (r) | (g << 8) | (b << 16) | (a << 24);
    }
    env->SetIntArrayRegion(jarray, 0, n, (const jint*)tmp.get());
  }
  jobject argbObject = createBitmapConfig(4);
  jobject drawableBitmap = env->CallStaticObjectMethod(cache->bitmapClass, cache->bitmapCreateMethod2, jarray, img.getWidth(), img.getHeight(), argbObject);

  if (env->ExceptionCheck()) {
    __android_log_print(ANDROID_LOG_VERBOSE, "Sometrik", "exception on imageToBitmap");
    env->ExceptionClear();
  }
  
  env->DeleteLocalRef(argbObject);
  env->DeleteLocalRef(jarray);

  return drawableBitmap;
}

static int android_read(void* cookie, char* buf, int size) {
  return AAsset_read((AAsset*)cookie, buf, size);
}

static int android_write(void* cookie, const char* buf, int size) {
  return EACCES; // can't provide write access to the apk
}

static fpos_t android_seek(void* cookie, fpos_t offset, int whence) {
  return AAsset_seek((AAsset*)cookie, offset, whence);
}

static int android_close(void* cookie) {
  AAsset_close((AAsset*)cookie);
  return 0;
}

class AndroidImage : public Image {
public:
  AndroidImage(AAssetManager * _asset_manager, float _display_scale)
    : Image(_display_scale), asset_manager(_asset_manager) { }

  AndroidImage(AAssetManager * _asset_manager, const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels, float _display_scale) : Image(_data, _width, _height, _num_channels, _display_scale), asset_manager(_asset_manager) { }

private:
  AAssetManager * asset_manager;
};

std::unique_ptr<Image>
AndroidSurface::createImage(float display_scale) {
  std::unique_ptr<Image> image;
  unsigned char * buffer = (unsigned char *)lockMemory(false);
  assert(buffer);
  if (buffer) {
    image = std::unique_ptr<Image>(new AndroidImage(0, buffer, getActualWidth(), getActualHeight(), getNumChannels(), display_scale));
    releaseMemory();
  }
  return image;
}

std::shared_ptr<AndroidCache> AndroidContextFactory::cache;

std::unique_ptr<Image>
AndroidContextFactory::createImage() {
  return std::unique_ptr<Image>(new AndroidImage(asset_manager, getDisplayScale()));
}

std::unique_ptr<Image>
AndroidContextFactory::createImage(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels) {
  return std::unique_ptr<Image>(new AndroidImage(0, _data, _width, _height, _num_channels, getDisplayScale()));
}

void
AndroidContextFactory::initialize(JNIEnv * env, jobject & manager) {
  cache = std::make_shared<AndroidCache>(env, manager);
}
