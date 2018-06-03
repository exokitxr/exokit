#ifndef _EXO_VIDEO_COMMON_H_
#define _EXO_VIDEO_COMMON_H_

#if defined(_WIN32)
#define EXO_OS_WIN 1
#elif defined(__APPLE__)
#define EXO_OS_OSX 1
#else
#define EXO_OS_LINUX 1
#define EXO_USING_V4L 1
#endif

#include <string>
#include <vector>
#include <sstream>

namespace ffmpeg {

  template<typename T>
  static T Abs(T a) {
    return (a < 0) ? -a : a;
  }
  template<typename T>
  static T Min(T a, T b) {
    return (a < b) ? a : b;
  }
  template<typename T>
  static T Max(T a, T b) {
    return (a > b) ? a : b;
  }
  static bool FuzzyCompare(float a, float b) {
    return Abs(a - b) * 100000.f <= Min(Abs(a), Abs(b));
  }
  static bool FuzzyCompare(double a, double b) {
    return Abs(a - b) * 1000000000000. <= Min(Abs(a), Abs(b));
  }

  typedef std::string DeviceString;
  typedef std::ostringstream DeviceStringStream;
  typedef std::pair<DeviceString, DeviceString> DevicePair;
  typedef std::vector<DevicePair> DeviceList;
  struct VideoMode;
  typedef std::vector<VideoMode> VideoModeList;
}

#endif


