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
  if (image != nullptr) {
    return image->getData().getWidth();
  } else {
    return 0;
  }
}

unsigned int Image::GetHeight() {
  if (image != nullptr) {
    return image->getData().getHeight();
  } else {
    return 0;
  }
}

unsigned int Image::GetNumChannels() {
  if (image != nullptr) {
    return image->getData().getNumChannels();
  } else {
    return 0;
  }
}

unsigned char *Image::GetData() {
  if (image != nullptr) {
    return image->getData().getData();
  } else {
    return nullptr;
  }
}

bool Image::Load(const unsigned char *buffer, size_t size) {
  if (image == nullptr) {
    image = CanvasRenderingContext2D::canvasContextFactory->createImage().release();
  }
  return image->decode(buffer, size);
}

void Image::Set(canvas::Image *image) {
  if (this->image != nullptr) {
    delete this->image;
  }
  this->image = image;
}

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
  if (image->image != nullptr) {
    if (image->dataArray.IsEmpty()) {
      unsigned int width = image->GetWidth();
      unsigned int height = image->GetHeight();
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), image->GetData(), width * height * 4);

      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      image->dataArray.Reset(uint8ClampedArray);
    }

    info.GetReturnValue().Set(Nan::New(image->dataArray));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
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

Image::Image () : image(nullptr) {}
Image::~Image () {
  if (image != nullptr) {
    delete image;
  }
}
