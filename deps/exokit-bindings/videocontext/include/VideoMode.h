#ifndef _HTML_VIDEO_MODE_ELEMENT_H_
#define _HTML_VIDEO_MODE_ELEMENT_H_

#include "VideoCommon.h"
#include <vector>

namespace ffmpeg {

struct VideoMode {
  int width;
  int height;
  float FPS;

  VideoMode(int width = 0, int height = 0, float FPS = 0)
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

  static size_t getDeviceList(DeviceList& devices);
};

}

#endif
