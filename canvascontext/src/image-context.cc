#include <canvascontext/include/image-context.h>

using namespace v8;

Handle<Object> Image::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Image"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();

  Nan::SetMethod(proto,"load", LoadMethod);

  return scope.Escape(ctor->GetFunction());
}

unsigned int Image::GetWidth() {
  if (image) {
    return image->width();
  } else {
    return 0;
  }
}

unsigned int Image::GetHeight() {
  if (image) {
    return image->height();
  } else {
    return 0;
  }
}

unsigned int Image::GetNumChannels() {
  return 4;
}

/* unsigned char *Image::GetData() {
  if (image) {
    return image->getData().getData();
  } else {
    return nullptr;
  }
} */

bool Image::Load(const unsigned char *buffer, size_t size) {
  sk_sp<SkData> data = SkData::MakeWithoutCopy(buffer, size);
  SkBitmap bitmap;
  bool ok = DecodeDataToBitmap(data, &bitmap);

  if (ok) {
    bitmap.setImmutable();
    image = SkImage::MakeFromBitmap(bitmap);

    return true;
  } else {
    return false;
  }
}

/* void Image::Set(canvas::Image *image) {
  this->image = image->image;
} */

NAN_METHOD(Image::New) {
  Nan::HandleScope scope;

  Local<Object> imageObj = info.This();

  Image *image = new Image();
  image->Wrap(imageObj);

  Nan::SetAccessor(imageObj, JS_STR("width"), WidthGetter);
  Nan::SetAccessor(imageObj, JS_STR("height"), HeightGetter);
  Nan::SetAccessor(imageObj, JS_STR("data"), DataGetter);

  info.GetReturnValue().Set(imageObj);
}

NAN_GETTER(Image::WidthGetter) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(info.This());
  info.GetReturnValue().Set(JS_INT(image->GetWidth()));
}

NAN_GETTER(Image::HeightGetter) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(info.This());
  info.GetReturnValue().Set(JS_INT(image->GetHeight()));
}

NAN_GETTER(Image::DataGetter) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(info.This());
  if (image->dataArray.IsEmpty()) {
    SkPixmap pixmap;
    bool ok = image->image->peekPixels(&pixmap);

    if (ok) {
      unsigned int width = image->GetWidth();
      unsigned int height = image->GetHeight();
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), (void *)pixmap.addr(), width * height * 4); // XXX link lifetime

      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      image->dataArray.Reset(uint8ClampedArray);
    } else {
      return info.GetReturnValue().Set(Nan::Null());
    }
  }

  info.GetReturnValue().Set(Nan::New(image->dataArray));
}

NAN_METHOD(Image::LoadMethod) {
  Nan::HandleScope scope;

  Image *image = ObjectWrap::Unwrap<Image>(info.This());
  Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[0]);
  v8::ArrayBuffer::Contents contents = arrayBuffer->GetContents();
  unsigned char *data = (unsigned char *)contents.Data();
  size_t size = contents.ByteLength();

  bool result = image->Load(data, size);
  info.GetReturnValue().Set(JS_BOOL(result));
}

Image::Image () {}
Image::~Image () {}
