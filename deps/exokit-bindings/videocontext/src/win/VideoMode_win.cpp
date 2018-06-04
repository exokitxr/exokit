#include <VideoMode.h>

#ifndef EXO_OS_WIN
#error "This file is meant to be compiled on Windows"
#endif

#pragma comment(lib, "Strmiids.lib")
#include <objbase.h>
#include <strmif.h>
#include <amvideo.h>
#include <dvdmedia.h>
#include <uuids.h>
#include <cassert>
#include <cstdint>

namespace ffmpeg {

static char*
ToUtf8(wchar_t* str)
{
    int num = WideCharToMultiByte(CP_UTF8, NULL, str, -1, NULL, NULL, NULL, NULL);
    char* out = new char[num];
    if (out) {
        WideCharToMultiByte(CP_UTF8, NULL, str, -1, out, num, NULL, NULL);
    }
    return out;
}

size_t
VideoMode::getDevices(DeviceList& devices) {
  size_t count = 0;
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (hr == S_OK) {
    IMoniker* moniker = NULL;

    ICreateDevEnum* enumerate = NULL;
    if (CoCreateInstance(
          CLSID_SystemDeviceEnum,
          NULL,
          CLSCTX_INPROC_SERVER,
          IID_ICreateDevEnum,
          (void**)&enumerate) == S_OK) {

      IEnumMoniker* classEnumerator = NULL;
      if (enumerate->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, (IEnumMoniker**)&classEnumerator, 0) == S_OK) {

        while (classEnumerator->Next(1, &moniker, NULL) == S_OK) {
          LPMALLOC comalloc = NULL;
          if (CoGetMalloc(1, &comalloc) == S_OK) {
            IBindCtx* bind = NULL;
            if (CreateBindCtx(0, &bind) == S_OK) {
              LPOLESTR str = NULL;
              if (moniker->GetDisplayName(bind, NULL, &str) == S_OK) {
                char* id = ToUtf8(str);
                size_t len = strlen(id);
                for (size_t i = 0; i < len; ++i) {
                  if (id[i] == ':') {
                    id[i] = '_';
                  }
                }
                IPropertyBag* propertyBag = NULL;
                if (moniker->BindToStorage(
                      NULL,
                      NULL,
                      IID_IPropertyBag,
                      (void**)&propertyBag) == S_OK) {
                  VARIANT variant;
                  variant.vt = VT_BSTR;
                  if (propertyBag->Read(L"FriendlyName", &variant, NULL) == S_OK) {
                    char* name = ToUtf8(variant.bstrVal);

                    devices.push_back({id, name});
                    count++;

                    delete[] name;
                  }
                  propertyBag->Release();
                }
                delete[] id;
                comalloc->Free(str);
              }
              bind->Release();
            }
          }
          moniker->Release();
        }
        classEnumerator->Release();

      }
      CoUninitialize();
    }
  }
  return count;
}

static IBaseFilter*
getDeviceFilter(const DeviceString& deviceName) {
  IBaseFilter* deviceFilter = NULL;
  ICreateDevEnum* enumerate = NULL;
  if (CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&enumerate) == S_OK) {
    IEnumMoniker* classenum = NULL;
    if (enumerate->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, (IEnumMoniker**)&classenum, 0) == S_OK) {
      IMoniker* moniker = NULL;
      while (classenum->Next(1, &moniker, NULL) == S_OK) {
        LPMALLOC comalloc = NULL;
        if (CoGetMalloc(1, &comalloc) == S_OK) {
          IBindCtx* bindCtx = NULL;
          if (CreateBindCtx(0, &bindCtx) == S_OK) {
            LPOLESTR str = NULL;
            if (moniker->GetDisplayName(bindCtx, NULL, &str) == S_OK) {
              char* name = ToUtf8(str);
              size_t len = strlen(name);
              for (size_t i = 0; i < len; ++i) {
                if (name[i] == ':') {
                  name[i] = '_';
                }
              }
              if (deviceName == name) {
                if (moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&deviceFilter) == S_OK) {
                }
              }
              delete[] name;
              comalloc->Free(str);
            }
            bindCtx->Release();
          }
        }
        moniker->Release();
      }
      classenum->Release();
      if (!deviceFilter) {
        fprintf(stderr, "Could't find the device %s\n", deviceName.c_str());
      }
    }
  }
  return deviceFilter;
}

size_t
VideoMode::getDeviceModes(VideoModeList& modes, const DeviceString& deviceName) {
  size_t count = 0;
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (hr == S_OK) {
    IBaseFilter* deviceFilter = getDeviceFilter(deviceName);
    if (deviceFilter) {
      IEnumPins* pins = NULL;
      IPin* pPin;
      if (deviceFilter->EnumPins(&pins) == S_OK) {
        while (pins->Next(1, &pPin, NULL) == S_OK) {
          PIN_INFO pinInfo;
          pPin->QueryPinInfo(&pinInfo);
          pinInfo.pFilter->Release();
          if (pinInfo.dir == PINDIR_OUTPUT) {
            IKsPropertySet* pPropertySet = NULL;
            if (pPin->QueryInterface(IID_IKsPropertySet, (void**)&pPropertySet) == S_OK) {
              GUID guid;
              DWORD r;
              if (pPropertySet->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &guid, sizeof(GUID), &r) == S_OK) {
                if (IsEqualGUID(guid, PIN_CATEGORY_CAPTURE)) {
                  IAMStreamConfig* streamConfiguration = NULL;
                  if (pPin->QueryInterface(IID_IAMStreamConfig, (void**)&streamConfiguration) == S_OK) {
                    int size;
                    int n;
                    if (streamConfiguration->GetNumberOfCapabilities(&n, &size) == S_OK) {

                      assert(size == sizeof(VIDEO_STREAM_CONFIG_CAPS));
                      VIDEO_STREAM_CONFIG_CAPS* videoCapabilities = new VIDEO_STREAM_CONFIG_CAPS;

                      for (int i = 0; i < n; ++i) {
                        AM_MEDIA_TYPE* mediaType = NULL;
                        VideoMode mode;
                        if (streamConfiguration->GetStreamCaps(i, &mediaType, (BYTE*)videoCapabilities) == S_OK) {
                          if (IsEqualGUID(mediaType->formattype, FORMAT_VideoInfo) || IsEqualGUID(mediaType->formattype, FORMAT_VideoInfo2)) {
                            mode.width = videoCapabilities->MaxOutputSize.cx;
                            mode.height = videoCapabilities->MaxOutputSize.cy;
                            mode.FPS = 10000000. / videoCapabilities->MinFrameInterval;
                            if (std::find(modes.begin(), modes.end(), mode) == modes.end()) {
                              modes.push_back(std::move(mode));
                              count++;
                            }
                          }
                          if (mediaType->pbFormat) {
                            CoTaskMemFree(mediaType->pbFormat);
                          }
                          CoTaskMemFree(mediaType);
                        }
                      }
                      delete videoCapabilities;
                      streamConfiguration->Release();
                    }
                  }
                }
              }
              pPropertySet->Release();
            }
          }
          pPin->Release();
        }
      }
    }
    CoUninitialize();
  }
  return count;
}

}

