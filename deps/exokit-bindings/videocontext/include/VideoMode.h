#ifndef _EXO_VIDEO_MODE_H_
#define _EXO_VIDEO_MODE_H_

#include "VideoCommon.h"

#include <cassert>


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
  AVFormatContext* pFormatCtx;
  int videoStream;
  AVCodec* pCodec;
  AVFrame* pFrame;
  AVFrame* pFrameRGB;
  AVPacket packet;


  VideoCamera(AVFormatContext* formatContext, AVCodec* codec, int videoStream)
  : pFormatCtx(formatContext)
  , videoStream(videoStream)
  , pCodec(codec)
  , pFrame(av_frame_alloc())
  , pFrameRGB(av_frame_alloc())
  {
    assert(videoStream >= 0);
  }

  ~VideoCamera()
  {
    av_free_packet(&packet);
    if (pFrameRGB) {
      av_free(pFrameRGB);
    }
    if (pFrame) {
      av_free(pFrame);
    }
    if (pFormatCtx) {
      avformat_close_input(&pFormatCtx);
    }
  }

  AVCodecContext* getCodecContext() const {
    assert(videoStream >= 0 && pFormatCtx);
    return pFormatCtx->streams[videoStream]->codec;
  }

  AVPixelFormat getFormat() const {
    // https://stackoverflow.com/a/23216860
    AVPixelFormat pixFormat = getCodecContext()->pix_fmt;
    switch (pixFormat) {
      case AV_PIX_FMT_YUVJ420P :
        pixFormat = AV_PIX_FMT_YUV420P;
        break;
      case AV_PIX_FMT_YUVJ422P  :
        pixFormat = AV_PIX_FMT_YUV422P;
        break;
      case AV_PIX_FMT_YUVJ444P   :
        pixFormat = AV_PIX_FMT_YUV444P;
        break;
      case AV_PIX_FMT_YUVJ440P :
        pixFormat = AV_PIX_FMT_YUV440P;
        break;
    }
    return pixFormat;
  }

  size_t getWidth() const {
    return getCodecContext()->width;
  }

  size_t getHeight() const {
    return getCodecContext()->height;
  }

  size_t getSize() const {
    return avpicture_get_size(AV_PIX_FMT_RGB24, getWidth(), getHeight());
  }

  void copy(uint8_t* buffer) const {
    avpicture_fill((AVPicture*)pFrameRGB, buffer, AV_PIX_FMT_RGB24, getWidth(), getHeight());
  }

  bool update()
  {
    int res = 0;
    int frameFinished = 0;
    if(res = av_read_frame(pFormatCtx,&packet)>=0)
    {
      if(packet.stream_index == videoStream){
        AVCodecContext* pCodecCtx(getCodecContext());
        avcodec_decode_video2(pCodecCtx,pFrame,&frameFinished,&packet);

        if(frameFinished){
          struct SwsContext * img_convert_ctx;
          img_convert_ctx = sws_getCachedContext(nullptr,getWidth(), getHeight(), getFormat(), getWidth(), getHeight(), AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr,nullptr);
          sws_scale(img_convert_ctx, ((AVPicture*)pFrame)->data, ((AVPicture*)pFrame)->linesize, 0, getHeight(), ((AVPicture *)pFrameRGB)->data, ((AVPicture *)pFrameRGB)->linesize);

          av_free_packet(&packet);
          sws_freeContext(img_convert_ctx);
          return true;
        }

      }

    }
    return false;
  }

  static AVInputFormat*
  getInputFormat()
  {
    AVInputFormat* iformat = nullptr;
#if USING_V4L
    if ((iformat = av_find_input_format("v4l2")))
        return iformat;
#endif

#ifdef EXO_OS_WIN
    if ((iformat = av_find_input_format("dshow")))
      return iformat;
#endif

#ifdef EXO_OS_OSX
    if ((iformat = av_find_input_format("avfoundation")))
        return iformat;
    if ((iformat = av_find_input_format("qtkit")))
        return iformat;
#endif

    fprintf(stderr, "No valid input format found\n");
    return nullptr;
  }

  static VideoCamera* open(const char* deviceName, AVDictionary* options)
  {
    AVInputFormat* format = getInputFormat();
    if (format) {
      AVFormatContext *pFormatCtx = avformat_alloc_context();
      if(avformat_open_input(&pFormatCtx, deviceName, format, &options) >= 0) {
        if(avformat_find_stream_info(pFormatCtx, nullptr) >= 0) {
          av_dump_format(pFormatCtx, 0, deviceName, 0);

          int videoStream = -1;
          for(unsigned int i=0; i < pFormatCtx->nb_streams; i++)
          {
            if(pFormatCtx->streams[i]->codec->coder_type==AVMEDIA_TYPE_VIDEO)
            {
              videoStream = i;
              break;
            }
          }
          if(videoStream != -1) {
            AVCodecContext* pCodecCtx = pFormatCtx->streams[videoStream]->codec;
            AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
            if(pCodec) {
              if(avcodec_open2(pCodecCtx,pCodec,nullptr) >= 0) {
                VideoCamera* device = new VideoCamera(pFormatCtx, pCodec, videoStream);
                return device;
              }
            }
          }
        }
        avformat_close_input(&pFormatCtx);
      }
    }
    return nullptr;
  }
};

struct VideoMode {
  int width;
  int height;
  double FPS;

  VideoMode(int width = 0, int height = 0, double FPS = 0)
    : width(width)
    , height(height)
    , FPS(FPS)
  {
  }

  operator bool() const {
    return width || height || static_cast<int>(FPS);
  }

  bool operator==(const VideoMode& r) const {
    if (width != r.width) return false;
    if (height != r.height) return false;
    if (!FuzzyCompare(FPS, r.FPS)) return false;
    return true;
  }

  const DeviceString& toString() const {
    DeviceStringStream stringStream;
    stringStream << width << "x" << height << "@" << FPS << "fps";
    return stringStream.str();
  }

  static size_t getDevices(DeviceList& devices);
  static size_t getDeviceModes(VideoModeList& modes, const DeviceString& deviceName);
  static VideoCamera* open(const DeviceString& name, const DeviceString& opts);
  static void close(VideoCamera*& device);
};

}

#endif
