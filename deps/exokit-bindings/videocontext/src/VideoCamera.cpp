#include "VideoCamera.h"

extern "C" {
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
}

namespace ffmpeg {

// https://stackoverflow.com/a/23216860
static AVPixelFormat normalizeFormat(AVPixelFormat pixFormat) {
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

VideoCamera::VideoCamera(AVFormatContext *pFormatCtx, int videoStream)
: pFormatCtx(pFormatCtx)
, videoStream(videoStream)
{
  AVFrame *pFrame = av_frame_alloc();
  AVFrame *pFrameRGB = av_frame_alloc();
  uint8_t *internal_buffer = (uint8_t *)av_malloc(getSize() * sizeof(uint8_t));
  avpicture_fill((AVPicture *)pFrameRGB, internal_buffer, AV_PIX_FMT_RGBA, getWidth(), getHeight());
  
  this->pFrameRGB = pFrameRGB;
  bool *pLive = new bool(true);
  this->pLive = pLive;
  std::mutex *pMutex = new std::mutex();
  this->pMutex = pMutex;
  bool *pFrameReady = new bool(false);
  this->pFrameReady = pFrameReady;

  std::thread([pFormatCtx, videoStream, pFrame, pFrameRGB, pLive, pMutex, pFrameReady]() mutable -> void {
    AVPacket packet;

    while (*pLive) {
      int frameFinished = 0;
      if (av_read_frame(pFormatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStream) {
          AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;
          avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

          if (frameFinished) {
            struct SwsContext *img_convert_ctx;
            img_convert_ctx = sws_getCachedContext(nullptr, pCodecCtx->width, pCodecCtx->height, normalizeFormat(pCodecCtx->pix_fmt), pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGBA, SWS_BICUBIC, nullptr, nullptr, nullptr);

            {
              std::lock_guard<std::mutex> lock(*pMutex);
              sws_scale(img_convert_ctx, ((AVPicture*)pFrame)->data, ((AVPicture*)pFrame)->linesize, 0, pCodecCtx->height, ((AVPicture *)pFrameRGB)->data, ((AVPicture *)pFrameRGB)->linesize);
              *pFrameReady = true;
            }

            sws_freeContext(img_convert_ctx);
          }
        }
      }
    }

    av_free_packet(&packet);
    av_free(pFrameRGB);
    av_free(pFrame);
    avformat_close_input(&pFormatCtx);
    delete pLive;
    delete pMutex;
    delete pFrameReady;
  }).detach();
}

VideoCamera::~VideoCamera() {
  *pLive = false;
}

AVCodecContext*
VideoCamera::getCodecContext() const
{
  return pFormatCtx->streams[videoStream]->codec;
}

AVPixelFormat
VideoCamera::getFormat() const
{
  return normalizeFormat(getCodecContext()->pix_fmt);
}

size_t
VideoCamera::getWidth() const
{
  return getCodecContext()->width;
}

size_t
VideoCamera::getHeight() const
{
  return getCodecContext()->height;
}

size_t
VideoCamera::getSize() const
{
  return avpicture_get_size(AV_PIX_FMT_RGBA, getWidth(), getHeight());
}

bool VideoCamera::isFrameReady() const {
  std::lock_guard<std::mutex> lock(*pMutex);
  return *pFrameReady;
}

void VideoCamera::pullUpdate(uint8_t *buffer) const {
  std::lock_guard<std::mutex> lock(*pMutex);
  avpicture_fill((AVPicture*)pFrameRGB, buffer, AV_PIX_FMT_RGBA, getWidth(), getHeight());
  *pFrameReady = false;
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

VideoCamera*
VideoCamera::open(const char* deviceName, AVDictionary* options)
{
  AVInputFormat* format = getInputFormat();
  if (format) {
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, deviceName, format, &options) >= 0) {
      if (avformat_find_stream_info(pFormatCtx, nullptr) >= 0) {
        av_dump_format(pFormatCtx, 0, deviceName, 0);

        int videoStream = -1;
        for (unsigned int i=0; i < pFormatCtx->nb_streams; i++)
        {
          if (pFormatCtx->streams[i]->codec->coder_type==AVMEDIA_TYPE_VIDEO)
          {
            videoStream = i;
            break;
          }
        }
        if (videoStream != -1) {
          AVCodecContext *pCodecCtx = pFormatCtx->streams[videoStream]->codec;
          AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
          if (pCodec) {
            if (avcodec_open2(pCodecCtx, pCodec, nullptr) >= 0) {
              VideoCamera* device = new VideoCamera(pFormatCtx, videoStream);
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

}
