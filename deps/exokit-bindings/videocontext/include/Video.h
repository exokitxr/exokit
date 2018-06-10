#ifndef _HTML_VIDEO_ELEMENT_H_
#define _HTML_VIDEO_ELEMENT_H_

#include <v8.h>
#include <node.h>
#include <nan/nan.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
}

#include <defines.h>

using namespace std;
using namespace v8;
using namespace node;

namespace ffmpeg {

enum FrameStatus {
  FRAME_STATUS_OK,
  FRAME_STATUS_ERROR,
  FRAME_STATUS_EOF,
};

class AppData {
public:
  AppData();
  ~AppData();

  void resetState();
  bool set(vector<unsigned char> &memory, string *error = nullptr);
  static int bufferRead(void *opaque, unsigned char *buf, int buf_size);
  static int64_t bufferSeek(void *opaque, int64_t offset, int whence);
  double getTimeBase();
  FrameStatus advanceToFrameAt(double timestamp);

public:
  std::vector<unsigned char> data;
  int64_t dataPos;

	AVFormatContext *fmt_ctx;
	AVIOContext *io_ctx;
	int stream_idx;
	AVStream *video_stream;
	AVCodecContext *codec_ctx;
	AVCodec *decoder;
	AVPacket *packet;
	AVFrame *av_frame;
	AVFrame *gl_frame;
	struct SwsContext *conv_ctx;
  double lastTimestamp;
};

class Video : public ObjectWrap {
public:
  static Handle<Object> Initialize(Isolate *isolate);
  bool Load(uint8_t *bufferValue, size_t bufferLength, string *error = nullptr);
  void Update();
  void Play();
  void Pause();
  void SeekTo(double timestamp);
  uint32_t GetWidth();
  uint32_t GetHeight();

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(Load);
  static NAN_METHOD(Update);
  static NAN_METHOD(Play);
  static NAN_METHOD(Pause);
  static NAN_GETTER(WidthGetter);
  static NAN_GETTER(HeightGetter);
  static NAN_GETTER(LoopGetter);
  static NAN_SETTER(LoopSetter);
  static NAN_GETTER(DataGetter);
  static NAN_GETTER(CurrentTimeGetter);
  static NAN_SETTER(CurrentTimeSetter);
  static NAN_GETTER(DurationGetter);
  static NAN_METHOD(UpdateAll);
  static NAN_METHOD(GetDevices);
  double getRequiredCurrentTime();
  double getRequiredCurrentTimeS();
  double getFrameCurrentTimeS();
  FrameStatus advanceToFrameAt(double timestamp);

  Video();
  ~Video();

private:
  AppData data;
  bool loaded;
  bool playing;
  bool loop;
  int64_t startTime;
  double startFrameTime;
  Nan::Persistent<Uint8ClampedArray> dataArray;
  bool dataDirty;
};

class VideoCamera;

class VideoDevice : public ObjectWrap {
  public:
    static Handle<Object> Initialize(Isolate *isolate, Local<Value> imageDataCons);

  protected:
    static NAN_METHOD(New);
    static NAN_METHOD(Open);
    static NAN_METHOD(Close);
    static NAN_GETTER(WidthGetter);
    static NAN_GETTER(HeightGetter);
    static NAN_GETTER(SizeGetter);
    static NAN_GETTER(DataGetter);
    static NAN_GETTER(ImageDataGetter);

    VideoDevice();
    ~VideoDevice();

  private:
    VideoCamera *dev;
    Nan::Persistent<Object> imageData;
};

extern std::vector<Video *> videos;
extern std::vector<VideoDevice *> videoDevices;

}

#endif
