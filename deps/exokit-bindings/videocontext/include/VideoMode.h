#ifndef _EXO_VIDEO_MODE_H_
#define _EXO_VIDEO_MODE_H_

#include "VideoCommon.h"

namespace ffmpeg {
class IVideoDevice {
public:
  virtual size_t getWidth() const = 0;
  virtual size_t getHeight() const = 0;
  virtual size_t getSize() const = 0;
  virtual bool update() = 0;
  virtual void copy(uint8_t* buffer) const = 0;
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
  static IVideoDevice* open(const DeviceString& name, const DeviceString& opts);
  static void close(IVideoDevice*& device);
};

}

#endif
