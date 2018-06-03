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
  AVDictionary *options = NULL;
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
  pDevice = NULL;
}

}
