#include <VideoCommon.h>
#include <VideoMode.h>
#include <AVFoundation/AVFoundation.h>

namespace ffmpeg {

static DeviceString NSStringToDeviceString(const NSString* str) {
  return DeviceString([str UTF8String]);
}

static NSString* DeviceStringToNSString(const DeviceString& str) {
  return [NSString stringWithCString:str.c_str() encoding:NSUTF8StringEncoding];
}

size_t
VideoMode::getDevices(DeviceList& devices) {
  size_t count = 0;

  NSArray* pDevices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
  for (AVCaptureDevice* pDevice in pDevices) {
    devices.push_back({
        NSStringToDeviceString([pDevice uniqueID]),
        NSStringToDeviceString([pDevice localizedName])
        });
    count++;
  }
  return count;
}

size_t
VideoMode::getDeviceModes(VideoModeList& modes, const DeviceString& deviceName) {
  size_t count = 0;

  NSString* name = DeviceStringToNSString(deviceName);
  AVCaptureDevice* device = [AVCaptureDevice deviceWithUniqueID:name];
  if (device) {
    for (AVCaptureDeviceFormat* pFormat in [device formats]) {
      CMFormatDescriptionRef desc = (CMFormatDescriptionRef)[pFormat performSelector:@selector(formatDescription)];
      CMVideoDimensions dims = CMVideoFormatDescriptionGetDimensions(desc);

      for (AVFrameRateRange* pRange in pFormat.videoSupportedFrameRateRanges) {
        VideoMode mode;
        mode.FPS = pRange.maxFrameRate;
        mode.width = dims.width;
        mode.height = dims.height;
        modes.push_back(mode);
        count++;
      }

    }
  }


  return count;
}

}

