#include <canvascontext/include/imageBitmap-context.h>

using namespace v8;
using namespace node;

Handle<Object> ImageBitmap::Initialize(Isolate *isolate, Local<Value> imageCons) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("ImageBitmap"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Local<Function> ctorFn = ctor->GetFunction();

  Local<Function> createImageBitmapFn = Nan::New<Function>(CreateImageBitmap);
  createImageBitmapFn->Set(JS_STR("ImageBitmap"), ctorFn);
  createImageBitmapFn->Set(JS_STR("Image"), imageCons);

  ctorFn->Set(JS_STR("createImageBitmap"), createImageBitmapFn);

  return scope.Escape(ctorFn);
}

unsigned int ImageBitmap::GetWidth() {
  if (imageData != nullptr) {
    return imageData->getWidth();
  } else {
    return 0;
  }
}

unsigned int ImageBitmap::GetHeight() {
  if (imageData != nullptr) {
    return imageData->getHeight();
  } else {
    return 0;
  }
}

unsigned int ImageBitmap::GetNumChannels() {
  if (imageData != nullptr) {
    return imageData->getNumChannels();
  } else {
    return 0;
  }
}

unsigned char *ImageBitmap::GetData() {
  if (imageData != nullptr) {
    return imageData->getData();
  } else {
    return nullptr;
  }
}

void ImageBitmap::Set(canvas::ImageData *imageData) {
  if (this->imageData != nullptr) {
    delete this->imageData;
  }
  this->imageData = imageData;
}

NAN_METHOD(ImageBitmap::New) {
  Nan::HandleScope scope;

  Local<Object> imageBitmapObj = info.This();

  if (info[0]->IsObject() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber() && info[4]->IsNumber() && info[5]->IsBoolean()) {
    Image *image = ObjectWrap::Unwrap<Image>(info[0]->ToObject());
    int x = info[1]->Int32Value();
    int y = info[2]->Int32Value();
    unsigned int width = info[3]->Uint32Value();
    unsigned int height = info[4]->Uint32Value();
    bool flipY = info[5]->BooleanValue();
    ImageBitmap *imageBitmap = new ImageBitmap(image, x, y, width, height, flipY);
    imageBitmap->Wrap(imageBitmapObj);
  } else {
    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsArrayBufferView()) {
      unsigned int width = info[0]->Uint32Value();
      unsigned int height = info[1]->Uint32Value();
      Local<ArrayBufferView> dataValue = Local<ArrayBufferView>::Cast(info[2]);
      unsigned char *data = (unsigned char *)dataValue->Buffer()->GetContents().Data() + dataValue->ByteOffset();
      ImageBitmap *imageBitmap = new ImageBitmap(width, height, data);
      imageBitmap->Wrap(imageBitmapObj);
    } else {
      ImageBitmap *imageBitmap = new ImageBitmap();
      imageBitmap->Wrap(imageBitmapObj);
    }
  }

  Nan::SetAccessor(imageBitmapObj, JS_STR("width"), WidthGetter);
  Nan::SetAccessor(imageBitmapObj, JS_STR("height"), HeightGetter);
  Nan::SetAccessor(imageBitmapObj, JS_STR("data"), DataGetter);

  info.GetReturnValue().Set(imageBitmapObj);
}

NAN_GETTER(ImageBitmap::WidthGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());
  if (imageBitmap->imageData != nullptr) {
    info.GetReturnValue().Set(JS_INT(imageBitmap->GetWidth()));
  } else {
    info.GetReturnValue().Set(JS_INT(0));
  }
}

NAN_GETTER(ImageBitmap::HeightGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());
  if (imageBitmap->imageData != nullptr) {
    info.GetReturnValue().Set(JS_INT(imageBitmap->GetHeight()));
  } else {
    info.GetReturnValue().Set(JS_INT(0));
  }
}

NAN_GETTER(ImageBitmap::DataGetter) {
  Nan::HandleScope scope;

  ImageBitmap *imageBitmap = ObjectWrap::Unwrap<ImageBitmap>(info.This());
  if (imageBitmap->imageData != nullptr) {
    if (imageBitmap->dataArray.IsEmpty()) {
      unsigned int width = imageBitmap->GetWidth();
      unsigned int height = imageBitmap->GetHeight();
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), imageBitmap->GetData(), width * height * 4);

      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      imageBitmap->dataArray.Reset(uint8ClampedArray);
    }

    info.GetReturnValue().Set(Nan::New(imageBitmap->dataArray));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(ImageBitmap::CreateImageBitmap) {
  Nan::HandleScope scope;

  if (info[0]->IsObject()) {
    Local<Object> imageObj;
    if (info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("HTMLImageElement"))) {
      Local<Value> imageValue = info[0]->ToObject()->Get(JS_STR("image"));
      if (imageValue->IsObject()) {
        imageObj = imageValue->ToObject();
      } else {
        return Nan::ThrowError("Invalid arguments");
      }
    } else if (info[0]->ToObject()->Get(JS_STR("constructor"))->ToObject()->Get(JS_STR("name"))->StrictEquals(JS_STR("Blob"))) {
      Local<Object> buffer = info[0]->ToObject()->Get(JS_STR("buffer"))->ToObject();
      unsigned int byteOffset = buffer->Get(JS_STR("byteOffset"))->Uint32Value();
      unsigned int byteLength = buffer->Get(JS_STR("byteLength"))->Uint32Value();
      Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(buffer->Get(JS_STR("buffer")));

      Local<Function> imageConstructor = Local<Function>::Cast(info.Callee()->Get(JS_STR("Image")));
      Local<Value> argv[] = {};
      imageObj = imageConstructor->NewInstance(sizeof(argv) / sizeof(argv[0]), argv);

      Image *image = ObjectWrap::Unwrap<Image>(imageObj);
      if (!image->Load((unsigned char *)arrayBuffer->GetContents().Data() + byteOffset, byteLength)) {
        return Nan::ThrowError("Failed to load image");
      }
    } else {
      return Nan::ThrowError("Invalid arguments");
    }

    int x = info[1]->IsNumber() ? info[1]->Int32Value() : 0;
    int y = info[2]->IsNumber() ? info[2]->Int32Value() : 0;
    unsigned int width = info[3]->IsNumber() ? info[3]->Uint32Value() : ObjectWrap::Unwrap<Image>(imageObj)->GetWidth();
    unsigned int height = info[4]->IsNumber() ? info[4]->Uint32Value() : ObjectWrap::Unwrap<Image>(imageObj)->GetHeight();

    Local<Value> imageOrientationValue;
    if (info[1]->IsObject()) {
      imageOrientationValue = info[1]->ToObject()->Get(JS_STR("imageOrientation"));
    } else if (info[5]->IsObject()) {
      imageOrientationValue = info[5]->ToObject()->Get(JS_STR("imageOrientation"));
    }
    bool flipY = (!imageOrientationValue.IsEmpty() && imageOrientationValue->IsString()) ?
      (strcmp(*String::Utf8Value(imageOrientationValue), "flipY") == 0)
    :
      false;

    Local<Function> imageBitmapConstructor = Local<Function>::Cast(info.Callee()->Get(JS_STR("ImageBitmap")));
    Local<Value> argv[] = {
      imageObj,
      Nan::New<Integer>(x),
      Nan::New<Integer>(y),
      Nan::New<Integer>(width),
      Nan::New<Integer>(height),
      Nan::New<Boolean>(flipY),
    };
    Local<Object> imageBitmapObj = imageBitmapConstructor->NewInstance(sizeof(argv) / sizeof(argv[0]), argv);

    info.GetReturnValue().Set(imageBitmapObj);
  } else {
    Nan::ThrowError("Invalid arguments");
  }
}

ImageBitmap::ImageBitmap() : imageData(nullptr) {}
ImageBitmap::ImageBitmap(Image *image, int x, int y, unsigned int width, unsigned int height, bool flipY) :
  imageData(image->image->getData().crop(x, y, width, height, flipY).release()) {}
ImageBitmap::ImageBitmap(unsigned int width, unsigned int height, unsigned char *data) : imageData(new canvas::ImageData(data, width, height, 4)) {}
ImageBitmap::~ImageBitmap () {
  if (imageData != nullptr) {
    delete imageData;
  }
}
