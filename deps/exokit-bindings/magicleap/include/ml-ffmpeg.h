#if defined(LUMIN)

#include <magicleap.h>
// #include <ml-math.h>
#include <uv.h>

#include <unistd.h>
#include <iostream>
#include <vector>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}

using namespace v8;

namespace ml {

class FramebufferRequest {
public:
  GLuint pbo;
  uint8_t *framebufferDataRgb;
};

class FramebufferResponse {
public:
  GLuint pbo;
};

class MLStream {
public:
  MLStream(int cameraID = 0, int fps = 20, int width = 1280, int height = 720, int bitrate = 3500 * 1024, const std::string &codec_profile = std::string("main"), const std::string &outputServer = std::string("rtmp://127.0.0.1:1935/live/lol"));

  static void Init();
  void start();
  // void stop();
  void pushFramebuffer(GLuint pbo, uint8_t *framebufferDataRgb);
  void tick();
  static void RunResponseInMainThread(uv_async_t *handle);

protected:
  int cameraID;
  int fps;
  int width;
  int height;
  int bitrate;
  std::string codec_profile;
  std::string outputServer;

  int64_t startTime;
  int vts;
  int ats;

  std::vector<uint8_t> imgbuf;
  float **samplebuf;

  // format
  AVCodec *audio_codec;
  AVCodec *video_codec;
  AVStream *audio_st;
  AVStream *video_st;
  AVCodecContext *audio_cc;
  AVCodecContext *video_cc;
  AVFormatContext *oc;

  SwsContext *swsctx;
  AVFrame *video_frame;
  AVFrame *audio_frame;

  std::thread thread;
  std::mutex requestMutex;
  std::vector<FramebufferRequest> framebufferRequestQueue;

  static std::mutex responseMutex;
  static uv_async_t responseAsync;
  static std::vector<FramebufferResponse> framebufferResponseQueue;
};

}

#endif
