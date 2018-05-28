#include "VideoMode.h"

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
wcharToUtf8(wchar_t* str)
{
    int num = WideCharToMultiByte(CP_UTF8, NULL, str, -1, NULL, NULL, NULL, NULL);
    char* out = new char[num];
    if (out) {
        WideCharToMultiByte(CP_UTF8, NULL, str, -1, out, num, NULL, NULL);
    }
    return out;
}

size_t
VideoMode::getDeviceList(DeviceList& devices) {
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
                char* id = wcharToUtf8(str);

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
                    char* name = wcharToUtf8(variant.bstrVal);

                    devices.push_back({DeviceString("video=") + id, name});
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

}
