#ifndef _HTML_VIDEO_COMMON_ELEMENT_H_
#define _HTML_VIDEO_COMMON_ELEMENT_H_

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

namespace ffmpeg {

  static int Abs(int a) {
    return (a < 0) ? -a : a;
  }
  static float Abs(float a) {
    return (a < 0) ? -a : a;
  }
  static double Abs(double a) {
    return (a < 0) ? -a : a;
  }
  static float Min(float a, float b) {
    return (a < b) ? a : b;
  }
  static double Min(double a, double b) {
    return (a < b) ? a : b;
  }
  static float Max(float a, float b) {
    return (a > b) ? a : b;
  }
  static double Max(double a, double b) {
    return (a > b) ? a : b;
  }
  static bool FuzzyCompare(float a, float b) {
    return Abs(a - b) * 100000.f <= Min(Abs(a), Abs(b));
  }
  static bool FuzzyCompare(double a, double b) {
    return Abs(a - b) * 1000000000000. <= Min(Abs(a), Abs(b));
  }

  typedef std::string DeviceString;
  typedef std::pair<DeviceString, DeviceString> DevicePair;
  typedef std::vector<DevicePair> DeviceList;
  struct VideoMode;
  typedef std::vector<VideoMode> VideoModeList;
}

#endif


