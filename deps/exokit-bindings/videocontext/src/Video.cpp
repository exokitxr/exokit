#include <Video.h>
#include "VideoMode.h"

using namespace v8;

extern "C" {
#include <libavdevice/avdevice.h>
}

namespace ffmpeg {

const int kBufferSize = 4 * 1024;
const AVPixelFormat kPixelFormat = AV_PIX_FMT_RGB24;

AppData::AppData() :
  dataPos(0),
  fmt_ctx(nullptr), io_ctx(nullptr), stream_idx(-1), video_stream(nullptr), codec_ctx(nullptr), decoder(nullptr), packet(nullptr), av_frame(nullptr), gl_frame(nullptr), conv_ctx(nullptr), lastTimestamp(0) {}
AppData::~AppData() {
  resetState();
}

void AppData::resetState() {
  if (av_frame) {
    av_free(av_frame);
    av_frame = nullptr;
  }
  if (gl_frame) {
    av_free(gl_frame);
    gl_frame = nullptr;
  }
  if (packet) {
    av_free_packet(packet);
    packet = nullptr;
  }
  if (codec_ctx) {
    avcodec_close(codec_ctx);
    codec_ctx = nullptr;
  }
  if (fmt_ctx) {
    avformat_free_context(fmt_ctx);
    fmt_ctx = nullptr;
  }
  if (io_ctx) {
    av_free(io_ctx->buffer);
    av_free(io_ctx);
    io_ctx = nullptr;
  }
}

bool AppData::set(vector<unsigned char> &memory, string *error) {
  data = std::move(memory);
  resetState();

  // open video
  fmt_ctx = avformat_alloc_context();
  io_ctx = avio_alloc_context((unsigned char *)av_malloc(kBufferSize), kBufferSize, 0, this, bufferRead, NULL, bufferSeek);
  fmt_ctx->pb = io_ctx;
  if (avformat_open_input(&fmt_ctx, "memory input", NULL, NULL) < 0) {
    if (error) {
      *error = "failed to open input";
    }
    return false;
  }

  // find stream info
  if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
    if (error) {
      *error = "failed to get stream info";
    }
    return false;
  }

  // dump debug info
  // av_dump_format(fmt_ctx, 0, argv[1], 0);

   // find the video stream
  for (unsigned int i = 0; i < fmt_ctx->nb_streams; ++i)
  {
      if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
      {
          stream_idx = i;
          break;
      }
  }

  if (stream_idx == -1) {
    if (error) {
      *error = "failed to find video stream";
    }
    return false;
  }

  video_stream = fmt_ctx->streams[stream_idx];
  codec_ctx = video_stream->codec;

  // find the decoder
  decoder = avcodec_find_decoder(codec_ctx->codec_id);
  if (decoder == NULL) {
    if (error) {
      *error = "failed to find decoder";
    }
    return false;
  }

  // open the decoder
  if (avcodec_open2(codec_ctx, decoder, NULL) < 0) {
    if (error) {
      *error = "failed to open codec";
    }
    return false;
  }

  // allocate the video frames
  av_frame = av_frame_alloc();
  gl_frame = av_frame_alloc();
  int size = avpicture_get_size(kPixelFormat, codec_ctx->width, codec_ctx->height);
  uint8_t *internal_buffer = (uint8_t *)av_malloc(size * sizeof(uint8_t));
  avpicture_fill((AVPicture *)gl_frame, internal_buffer, kPixelFormat, codec_ctx->width, codec_ctx->height);
  packet = (AVPacket *)av_malloc(sizeof(AVPacket));

  // allocate the converter
  conv_ctx = sws_getContext(
    codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
    codec_ctx->width, codec_ctx->height, kPixelFormat,
    SWS_BICUBIC, NULL, NULL, NULL
  );

  return true;
}

int AppData::bufferRead(void *opaque, unsigned char *buf, int buf_size) {
  AppData *appData = (AppData *)opaque;
  int64_t readLength = std::min<int64_t>(buf_size, appData->data.size() - appData->dataPos);
  if (readLength > 0) {
    memcpy(buf, appData->data.data() + appData->dataPos, readLength);
    appData->dataPos += readLength;
    return readLength;
  } else {
    return AVERROR_EOF;
  }
}
int64_t AppData::bufferSeek(void *opaque, int64_t offset, int whence) {
  AppData *appData = (AppData *)opaque;
  if (whence == AVSEEK_SIZE) {
    return appData->data.size();
  } else {
    int64_t newPos;
    if (whence == SEEK_SET) {
      newPos = offset;
    } else if (whence == SEEK_CUR) {
      newPos = appData->dataPos + offset;
    } else if (whence == SEEK_END) {
      newPos = appData->data.size() + offset;
    } else {
      newPos = offset;
    }
    newPos = std::min<int64_t>(std::max<int64_t>(newPos, 0), appData->data.size() - appData->dataPos);
    appData->dataPos = newPos;
    return newPos;
  }
}

FrameStatus AppData::advanceToFrameAt(double timestamp) {
  double timeBase = getTimeBase();

  for (;;) {
    if (lastTimestamp >= timestamp) {
      return FRAME_STATUS_OK;
    }

    bool packetValid = false;
    for (;;) {
      if (packetValid) {
        av_free_packet(packet);
        packetValid = false;
      }

      int ret = av_read_frame(fmt_ctx, packet);
      packetValid = true;
      if (ret == AVERROR_EOF) {
        av_free_packet(packet);
        return FRAME_STATUS_EOF;
      } else if (ret < 0) {
        // std::cout << "Unknown error " << ret << "\n";
        av_free_packet(packet);
        return FRAME_STATUS_ERROR;
      } else {
        if (packet->stream_index == stream_idx) {
          break;
        }
      }
    }
    // we have a valid packet at this point
    int frame_finished = 0;
    if (avcodec_decode_video2(codec_ctx, av_frame, &frame_finished, packet) < 0) {
      av_free_packet(packet);
      return FRAME_STATUS_ERROR;
    }

    sws_scale(conv_ctx, av_frame->data, av_frame->linesize, 0, codec_ctx->height, gl_frame->data, gl_frame->linesize);

    if (frame_finished) {
      lastTimestamp = (double)packet->pts * timeBase;
    }

    av_free_packet(packet);
  }
}

double AppData::getTimeBase() {
  if (video_stream) {
    return (double)video_stream->time_base.num / (double)video_stream->time_base.den;
  } else {
    return 1;
  }
}

Video::Video() : loaded(false), playing(false), loop(false), startTime(0), startFrameTime(0), dataDirty(true) {
  videos.push_back(this);
}

Video::~Video() {
  videos.erase(std::find(videos.begin(), videos.end(), this));
}

Handle<Object> Video::Initialize(Isolate *isolate) {
  // initialize libav
  av_register_all();
  avcodec_register_all();
  avdevice_register_all();
  avformat_network_init();

  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Video"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "load", Load);
  Nan::SetMethod(proto, "update", Update);
  Nan::SetMethod(proto, "play", Play);
  Nan::SetMethod(proto, "pause", Pause);
  Nan::SetAccessor(proto, JS_STR("width"), WidthGetter);
  Nan::SetAccessor(proto, JS_STR("height"), HeightGetter);
  Nan::SetAccessor(proto, JS_STR("loop"), LoopGetter, LoopSetter);
  Nan::SetAccessor(proto, JS_STR("data"), DataGetter);
  Nan::SetAccessor(proto, JS_STR("currentTime"), CurrentTimeGetter, CurrentTimeSetter);
  Nan::SetAccessor(proto, JS_STR("duration"), DurationGetter);

  Local<Function> ctorFn = ctor->GetFunction();

  ctorFn->Set(JS_STR("updateAll"), Nan::New<Function>(UpdateAll));
  ctorFn->Set(JS_STR("getDevices"), Nan::New<Function>(GetDevices));

  return scope.Escape(ctorFn);
}

NAN_METHOD(Video::New) {
  Nan::HandleScope scope;

  Video *video = new Video();
  Local<Object> videoObj = info.This();
  video->Wrap(videoObj);

  info.GetReturnValue().Set(videoObj);
}

bool Video::Load(unsigned char *bufferValue, size_t bufferLength, string *error) {
  // reset state
  loaded = false;
  dataArray.Reset();
  dataDirty = true;

  // initialize custom data structure
  std::vector<unsigned char> bufferData(bufferLength);
  memcpy(bufferData.data(), bufferValue, bufferLength);

  if (data.set(bufferData, error)) { // takes ownership of bufferData
    // scan to the first frame
    FrameStatus status = advanceToFrameAt(0);
    if (status == FRAME_STATUS_OK) {
      loaded = true;

      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void Video::Update() {
  if (loaded && playing) {
    FrameStatus status = advanceToFrameAt(getRequiredCurrentTimeS());
    if (status == FRAME_STATUS_EOF) {
      if (loop) {
        SeekTo(0);
      } else {
        Pause();
      }
    }
  }
}

void Video::Play() {
  if (!playing) {
    playing = true;
    startTime = av_gettime();
    startFrameTime = getFrameCurrentTimeS();
  }
}

void Video::Pause() {
  playing = false;
}

void Video::SeekTo(double timestamp) {
  if (loaded) {
    startTime = av_gettime() - (int64_t)(timestamp * 1e6);

    data.dataPos = 0;
    if (av_seek_frame(data.fmt_ctx, data.stream_idx, (int64_t)(timestamp / data.video_stream->time_base.num * data.video_stream->time_base.den), AVSEEK_FLAG_BACKWARD) >= 0) {
      avcodec_flush_buffers(data.codec_ctx);
      av_free(data.av_frame);
      data.av_frame = av_frame_alloc();
      data.lastTimestamp = 0;

      advanceToFrameAt(getRequiredCurrentTimeS());
    } else {
      Nan::ThrowError("currentTime: failed to seek");
    }
  }
}

uint32_t Video::GetWidth() {
  if (loaded) {
    return data.codec_ctx->width;
  } else {
    return 0;
  }
}

uint32_t Video::GetHeight() {
  if (loaded) {
    return data.codec_ctx->height;
  } else {
    return 0;
  }
}

NAN_METHOD(Video::Load) {
  if (info[0]->IsArrayBuffer()) {
    Video *video = ObjectWrap::Unwrap<Video>(info.This());

    Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[0]);

    string error;
    if (video->Load((uint8_t *)arrayBuffer->GetContents().Data(), arrayBuffer->ByteLength(), &error)) {
      // nothing
    } else {
      Nan::ThrowError(error.c_str());
    }
  } else if (info[0]->IsTypedArray()) {
    Video *video = ObjectWrap::Unwrap<Video>(info.This());

    Local<ArrayBufferView> arrayBufferView = Local<ArrayBufferView>::Cast(info[0]);
    Local<ArrayBuffer> arrayBuffer = arrayBufferView->Buffer();

    string error;
    if (video->Load((unsigned char *)arrayBuffer->GetContents().Data() + arrayBufferView->ByteOffset(), arrayBufferView->ByteLength())) {
      // nothing
    } else {
      Nan::ThrowError(error.c_str());
    }
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

NAN_METHOD(Video::Update) {
  Video *video = ObjectWrap::Unwrap<Video>(info.This());
  video->Update();
}

NAN_METHOD(Video::Play) {
  Video *video = ObjectWrap::Unwrap<Video>(info.This());
  video->Play();
}

NAN_METHOD(Video::Pause) {
  Video *video = ObjectWrap::Unwrap<Video>(info.This());
  video->Pause();
}

NAN_GETTER(Video::WidthGetter) {
  Nan::HandleScope scope;

  Video *video = ObjectWrap::Unwrap<Video>(info.This());
  info.GetReturnValue().Set(JS_INT(video->GetWidth()));
}

NAN_GETTER(Video::HeightGetter) {
  Nan::HandleScope scope;

  Video *video = ObjectWrap::Unwrap<Video>(info.This());
  info.GetReturnValue().Set(JS_INT(video->GetHeight()));
}

NAN_GETTER(Video::LoopGetter) {
  Nan::HandleScope scope;

  Video *video = ObjectWrap::Unwrap<Video>(info.This());
  info.GetReturnValue().Set(JS_BOOL(video->loop));
}

NAN_SETTER(Video::LoopSetter) {
  Nan::HandleScope scope;

  if (value->IsBoolean()) {
    Video *video = ObjectWrap::Unwrap<Video>(info.This());
    video->loop = value->BooleanValue();
  } else {
    Nan::ThrowError("loop: invalid arguments`");
  }
}

NAN_GETTER(Video::DataGetter) {
  Nan::HandleScope scope;

  Video *video = ObjectWrap::Unwrap<Video>(info.This());

  unsigned int width = video->GetWidth();
  unsigned int height = video->GetHeight();
  unsigned int dataSize = width * height * 3;
  if (video->dataArray.IsEmpty()) {
    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), dataSize);
    Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
    video->dataArray.Reset(uint8ClampedArray);
  }

  Local<Uint8ClampedArray> uint8ClampedArray = Nan::New(video->dataArray);
  if (video->loaded && video->dataDirty) {
    Local<ArrayBuffer> arrayBuffer = uint8ClampedArray->Buffer();
    memcpy((unsigned char *)arrayBuffer->GetContents().Data() + uint8ClampedArray->ByteOffset(), video->data.gl_frame->data[0], dataSize);
    video->dataDirty = false;
  }

  info.GetReturnValue().Set(uint8ClampedArray);
}

NAN_GETTER(Video::CurrentTimeGetter) {
  Nan::HandleScope scope;

  Video *video = ObjectWrap::Unwrap<Video>(info.This());

  double currentTime = video->getFrameCurrentTimeS();
  info.GetReturnValue().Set(JS_NUM(currentTime));
}

NAN_SETTER(Video::CurrentTimeSetter) {
  Nan::HandleScope scope;

  if (value->IsNumber()) {
    Video *video = ObjectWrap::Unwrap<Video>(info.This());

    double timestamp = value->NumberValue();
    video->SeekTo(timestamp);
  } else {
    Nan::ThrowError("currentTime: invalid arguments");
  }
}

NAN_GETTER(Video::DurationGetter) {
  Nan::HandleScope scope;

  Video *video = ObjectWrap::Unwrap<Video>(info.This());

  double duration = video->loaded ? ((double)video->data.fmt_ctx->duration / (double)AV_TIME_BASE) : 1;
  info.GetReturnValue().Set(JS_NUM(duration));
}

NAN_METHOD(Video::UpdateAll) {
  for (auto i : videos) {
    i->Update();
  }
}
NAN_METHOD(Video::GetDevices) {
  Nan::HandleScope scope;

  DeviceList devices;
  VideoMode::getDevices(devices);

  Local<Object> lst = Array::New(Isolate::GetCurrent());
  size_t i = 0;
  for (auto device : devices) {
    const DeviceString& id(device.first);
    const DeviceString& name(device.second);
    Local<Object> obj = Object::New(Isolate::GetCurrent());
    lst->Set(i++, obj);
    obj->Set(JS_STR("id"), JS_STR(id.c_str()));
    obj->Set(JS_STR("name"), JS_STR(name.c_str()));

    VideoModeList modes;
    VideoMode::getDeviceModes(modes, id);

    Local<Object> lst = Array::New(Isolate::GetCurrent());
    size_t j = 0;
    for (auto mode : modes) {
      Local<Object> obj = Object::New(Isolate::GetCurrent());
      lst->Set(j++, obj);
      obj->Set(JS_STR("width"), JS_NUM(mode.width));
      obj->Set(JS_STR("height"), JS_NUM(mode.height));
      obj->Set(JS_STR("fps"), JS_NUM(mode.FPS));
    }
    obj->Set(JS_STR("modes"), lst);
  }
  info.GetReturnValue().Set(lst);
}

double Video::getRequiredCurrentTimeS() {
  if (playing) {
    int64_t now = av_gettime();
    int64_t startTimeDiff = now - startTime;
    double startTimeDiffS = std::max<double>((double)startTimeDiff / 1e6, 0);
    return startFrameTime + startTimeDiffS;
  } else {
    return getFrameCurrentTimeS();
  }
}

double Video::getFrameCurrentTimeS() {
  if (loaded) {
    double pts = data.av_frame ? (double)std::max<int64_t>(data.av_frame->pts, 0) : 0;
    double timeBase = data.getTimeBase();
    return pts * timeBase;
  } else {
    return 0;
  }
}

FrameStatus Video::advanceToFrameAt(double timestamp) {
  FrameStatus status = data.advanceToFrameAt(timestamp);
  if (status == FRAME_STATUS_OK) {
    dataDirty = true;
  }
  return status;
}


VideoDevice::VideoDevice() : dataDirty(true), dev(NULL) {
  videoDevices.push_back(this);
}

VideoDevice::~VideoDevice() {
  if (dev) {
    VideoMode::close(dev);
  }
  videoDevices.erase(std::find(videoDevices.begin(), videoDevices.end(), this));
}

Handle<Object> VideoDevice::Initialize(Isolate *isolate) {
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("VideoDevice"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "open", Open);
  Nan::SetMethod(proto, "close", Close);
  Nan::SetMethod(proto, "update", Update);
  Nan::SetAccessor(proto, JS_STR("width"), WidthGetter);
  Nan::SetAccessor(proto, JS_STR("height"), HeightGetter);
  Nan::SetAccessor(proto, JS_STR("size"), SizeGetter);
  Nan::SetAccessor(proto, JS_STR("data"), DataGetter);

  Local<Function> ctorFn = ctor->GetFunction();

  ctorFn->Set(JS_STR("updateAll"), Nan::New<Function>(UpdateAll));

  return scope.Escape(ctorFn);
}

NAN_METHOD(VideoDevice::New) {
  Nan::HandleScope scope;

  VideoDevice *video = new VideoDevice();
  Local<Object> videoObj = info.This();
  video->Wrap(videoObj);

  info.GetReturnValue().Set(videoObj);
}

NAN_METHOD(VideoDevice::Open) {
  VideoDevice *video = ObjectWrap::Unwrap<VideoDevice>(info.This());
  if (video->dev) {
    VideoMode::close(video->dev);
    video->dev = NULL;
  }
  if (info.Length() < 1) {
    Nan::ThrowError("VideoDevice.Open: pass in a device name");
  } else {
    Nan::Utf8String nameStr(info[0]);
    std::string name(*nameStr, nameStr.length());
    std::string opts;
    if (info.Length() >= 2)
    {
      Nan::Utf8String optsStr(info[1]);
      opts = std::string(*optsStr, optsStr.length());
    }
    video->dev = VideoMode::open(name, opts);
    info.GetReturnValue().Set(JS_BOOL(video->dev != NULL));
    if (video->dev) {
      unsigned int dataSize = video->dev->getSize();
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), dataSize);
      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      video->dataArray.Reset(uint8ClampedArray);
      uint8_t* buffer = (uint8_t *)arrayBuffer->GetContents().Data() + uint8ClampedArray->ByteOffset();
      video->dev->copy(buffer);
      video->dataDirty = false;
    }
  }
}

NAN_METHOD(VideoDevice::Close) {
  VideoDevice *video = ObjectWrap::Unwrap<VideoDevice>(info.This());
  if (video->dev) {
    VideoMode::close(video->dev);
    video->dev = NULL;
  }
}

NAN_METHOD(VideoDevice::Update) {
  VideoDevice *video = ObjectWrap::Unwrap<VideoDevice>(info.This());
  info.GetReturnValue().Set(JS_BOOL(video->Update()));
}

bool VideoDevice::Update() {
  if (dev) {
    if (dev->update()) {
      dataDirty = true;
      return true;
    }
  }
  return false;
}

NAN_GETTER(VideoDevice::WidthGetter) {
  Nan::HandleScope scope;

  VideoDevice *video = ObjectWrap::Unwrap<VideoDevice>(info.This());
  if (video->dev) {
    info.GetReturnValue().Set(JS_INT((unsigned int)video->dev->getWidth()));
  } else {
    info.GetReturnValue().Set(JS_INT(0));
  }
}

NAN_GETTER(VideoDevice::HeightGetter) {
  Nan::HandleScope scope;

  VideoDevice *video = ObjectWrap::Unwrap<VideoDevice>(info.This());
  if (video->dev) {
    info.GetReturnValue().Set(JS_INT((unsigned int)video->dev->getHeight()));
  } else {
    info.GetReturnValue().Set(JS_INT(0));
  }
}

NAN_GETTER(VideoDevice::SizeGetter) {
  Nan::HandleScope scope;

  VideoDevice *video = ObjectWrap::Unwrap<VideoDevice>(info.This());
  if (video->dev) {
    info.GetReturnValue().Set(JS_INT((unsigned int)video->dev->getSize()));
  } else {
    info.GetReturnValue().Set(JS_INT(0));
  }
}

NAN_GETTER(VideoDevice::DataGetter) {
  Nan::HandleScope scope;

  VideoDevice *video = ObjectWrap::Unwrap<VideoDevice>(info.This());

  Local<Uint8ClampedArray> uint8ClampedArray = Nan::New(video->dataArray);
  if (video->dev && video->dataDirty) {
    if (video->dataArray.IsEmpty()) {
      unsigned int dataSize = video->dev->getSize();
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), dataSize);
      Local<Uint8ClampedArray> uint8ClampedArray = Uint8ClampedArray::New(arrayBuffer, 0, arrayBuffer->ByteLength());
      video->dataArray.Reset(uint8ClampedArray);
    }
    uint8_t* buffer = (uint8_t *)uint8ClampedArray->Buffer()->GetContents().Data() + uint8ClampedArray->ByteOffset();
    video->dev->copy(buffer);
    video->dataDirty = false;
  }

  info.GetReturnValue().Set(uint8ClampedArray);
}

NAN_METHOD(VideoDevice::UpdateAll) {
  for (auto i : videoDevices) {
    i->Update();
  }
}


std::vector<Video *> videos;
std::vector<VideoDevice *> videoDevices;

}
