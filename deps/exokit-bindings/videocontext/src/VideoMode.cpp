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

VideoCamera::VideoCamera(AVFormatContext* formatContext, AVCodec* codec, int videoStream)
: pFormatCtx(formatContext)
, videoStream(videoStream)
, pCodec(codec)
, pFrame(av_frame_alloc())
, pFrameRGB(av_frame_alloc())
{
  assert(videoStream >= 0);
}

VideoCamera::~VideoCamera()
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

AVCodecContext*
VideoCamera::getCodecContext() const
{
  assert(videoStream >= 0 && pFormatCtx);
  return pFormatCtx->streams[videoStream]->codec;
}

AVPixelFormat
VideoCamera::getFormat() const
{
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
  return avpicture_get_size(AV_PIX_FMT_RGB24, getWidth(), getHeight());
}

void
VideoCamera::copy(uint8_t* buffer) const
{
  avpicture_fill((AVPicture*)pFrameRGB, buffer, AV_PIX_FMT_RGB24, getWidth(), getHeight());
}

bool VideoCamera::update()
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

VideoCamera*
VideoCamera::open(const char* deviceName, AVDictionary* options)
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

VideoCamera*
VideoMode::open(const DeviceString& name, const DeviceString& opts)
{
  AVDictionary *options = nullptr;
  for (DeviceString opt : split(opts, '&')) {
    auto xs(split(opt, '='));
    if (xs.size() >= 2) {
      av_dict_set(&options, xs[0].c_str(), xs[1].c_str(), 0);
    }
  }
  return VideoCamera::open(name.c_str(), options);
}

void
VideoMode::close(VideoCamera*& pDevice)
{
  VideoCamera* device(static_cast<VideoCamera*>(pDevice));
  if (device) {
    delete device;
  }
  pDevice = nullptr;
}

}
