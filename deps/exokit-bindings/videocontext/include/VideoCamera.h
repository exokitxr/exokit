#ifndef _EXO_VIDEO_CAMERA_H_
#define _EXO_VIDEO_CAMERA_H_

#include "VideoCommon.h"

// #include <cassert>
// #include <iomanip> // setprecision
#include <mutex>
#include <thread>

extern "C" {
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
}

namespace ffmpeg {

class VideoCamera
{
public:
  AVFormatContext *pFormatCtx;
  AVFrame *pFrameRGB;
  int videoStream;
  bool *pLive;
  std::mutex *pMutex;
  bool *pFrameReady;

  VideoCamera(AVFormatContext *formatContext, int videoStream);
  ~VideoCamera();

  AVCodecContext *getCodecContext() const;
  AVPixelFormat getFormat() const;
  size_t getWidth() const;
  size_t getHeight() const;
  size_t getSize() const;
  bool isFrameReady() const;
  void pullUpdate(uint8_t *buffer) const;
  static VideoCamera* open(const char *deviceName, AVDictionary *options);
};

}

#endif

