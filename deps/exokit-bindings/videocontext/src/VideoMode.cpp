#include <VideoMode.h>

#include <cassert>
#include <iomanip> // setprecision

extern "C" {
#include <libavutil/avstring.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
}

namespace ffmpeg {

static AVInputFormat*
getInputFormat()
{
  AVInputFormat* iformat = NULL;
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
  return NULL;
}

class VideoDeviceImpl : public IVideoDevice
{
public:
  AVFormatContext* pFormatCtx;
  int videoStream;
  AVCodec* pCodec;
  AVFrame* pFrame;
  AVFrame* pFrameRGB;
  AVPacket packet;
  mutable bool dataDirty;


  VideoDeviceImpl(AVFormatContext* formatContext, AVCodec* codec, int videoStream)
  : pFormatCtx(formatContext)
  , videoStream(videoStream)
  , pCodec(codec)
  , pFrame(av_frame_alloc())
  , pFrameRGB(av_frame_alloc())
  , dataDirty(false)
  {
    assert(videoStream >= 0);
  }

  ~VideoDeviceImpl()
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
    if (videoStream < 0 || !pFormatCtx) {
      return NULL;
    }
    return pFormatCtx->streams[videoStream]->codec;
  }

  AVPixelFormat getFormat() const {
    return getCodecContext()->pix_fmt;
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
          img_convert_ctx = sws_getCachedContext(NULL,pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,   pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL,NULL);
          sws_scale(img_convert_ctx, ((AVPicture*)pFrame)->data, ((AVPicture*)pFrame)->linesize, 0, pCodecCtx->height, ((AVPicture *)pFrameRGB)->data, ((AVPicture *)pFrameRGB)->linesize);
          dataDirty = true;

          av_free_packet(&packet);
          sws_freeContext(img_convert_ctx);
          return true;
        }

      }

    }
    return false;
  }

  static VideoDeviceImpl* open(const char* deviceName, AVDictionary* options) {
    AVInputFormat* format = getInputFormat();
    if (format) {
      AVFormatContext *pFormatCtx = avformat_alloc_context();
      if(avformat_open_input(&pFormatCtx, deviceName, format, &options) >= 0) {
        if(avformat_find_stream_info(pFormatCtx, NULL) >= 0) {
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
              if(avcodec_open2(pCodecCtx,pCodec,NULL) >= 0) {
                VideoDeviceImpl* device = new VideoDeviceImpl(pFormatCtx, pCodec, videoStream);
                return device;
              }
            }
          }
        }
        avformat_close_input(&pFormatCtx);
      }
    }
    return NULL;
  }
};

template<typename Out>
static void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

IVideoDevice*
VideoMode::open(const DeviceString& name, const DeviceString& opts)
{
  AVDictionary *options = NULL;
  for (DeviceString opt : split(opts, '&')) {
    auto xs(split(opt, '='));
    if (xs.size() >= 2) {
      av_dict_set(&options, xs[0].c_str(), xs[1].c_str(), 0);
    }
  }
  return VideoDeviceImpl::open(name.c_str(), options);
}

void
VideoMode::close(IVideoDevice*& pDevice)
{
  VideoDeviceImpl* device(static_cast<VideoDeviceImpl*>(pDevice));
  if (device) {
    delete device;
  }
  pDevice = NULL;
}

}
