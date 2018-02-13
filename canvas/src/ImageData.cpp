#include <ImageData.h>

#include <vector>
#include <algorithm>
#include <cassert>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

using namespace std;
using namespace canvas;

bool ImageData::flip = false;

std::unique_ptr<ImageData>
ImageData::crop(int x, int y, unsigned short w, unsigned short h, bool flipY) const {
  const unsigned short fullWidth = getWidth();
  const unsigned short fullHeight = getHeight();

  const int minX = std::min<int>(std::max<int>(x, 0), fullWidth);
  const int minY = std::min<int>(std::max<int>(y, 0), fullHeight);
  const int maxX = std::min<int>(std::max<int>(x + w, 0), fullWidth);
  const int maxY = std::min<int>(std::max<int>(y + h, 0), fullHeight);

  const size_t size = calculateSize(w, h, num_channels);
  std::unique_ptr<unsigned char[]> outputData(new unsigned char[size]);
  memset(outputData.get(), 0, size);

  const int copyWidth = maxX - minX;
  if (!flipY) {
    for (int i = minY; i < maxY; i++) {
      memcpy(
        outputData.get() + ((i - y) * copyWidth * num_channels) + ((minX - x) * num_channels),
        data.get() + (i * fullWidth * num_channels) + (minX * num_channels),
        copyWidth * num_channels
      );
    }
  } else {
    for (int i = minY; i < maxY; i++) {
      memcpy(
        outputData.get() + ((i - y) * copyWidth * num_channels) + ((minX - x) * num_channels),
        data.get() + ((y + h - 1 - i) * fullWidth * num_channels) + (minX * num_channels),
        copyWidth * num_channels
      );
    }
  }

  return unique_ptr<ImageData>(new ImageData(outputData.get(), w, h, num_channels));
}

std::unique_ptr<ImageData>
ImageData::scale(unsigned short target_width, unsigned short target_height) const {
  size_t size = calculateSize(target_width, target_height, num_channels);
  std::unique_ptr<unsigned char[]> outputData(new unsigned char[size]);

  stbir_resize_uint8(data.get(), getWidth(), getHeight(), 0, outputData.get(), target_width, target_height, 0, num_channels);

  return unique_ptr<ImageData>(new ImageData(outputData.get(), target_width, target_height, num_channels));
}

std::unique_ptr<ImageData>
ImageData::colorize(const Color & color) const {
  assert(num_channels == 1);

  int red = int(255 * color.red * color.alpha);
  int green = int(255 * color.green * color.alpha);
  int blue = int(255 * color.blue * color.alpha);
  int alpha = int(255 * color.alpha);

  unique_ptr<ImageData> r(new ImageData(width, height, 4));

  unsigned char * target_buffer = r->getData();
  for (unsigned int i = 0; i < width * height; i++) {
    unsigned char v = data[i];
    target_buffer[4 * i + 0] = (unsigned char)(red * v / 255);
    target_buffer[4 * i + 1] = (unsigned char)(green * v / 255);
    target_buffer[4 * i + 2] = (unsigned char)(blue * v / 255);
    target_buffer[4 * i + 3] = (unsigned char)(alpha * v / 255);
  }

  return r;
}

static vector<int> make_kernel(float radius) {
  int r = (int)ceil(radius);
  int rows = 2 * r + 1;
  float sigma = radius / 3;
  float sigma22 = 2.0f * sigma * sigma;
  float sigmaPi2 = 2.0f * float(M_PI) * sigma;
  float sqrtSigmaPi2 = sqrt(sigmaPi2);
  // float radius2 = radius*radius;
  vector<int> kernel;
  kernel.reserve(rows);

  int row = -r;
  float first_value = exp(-row*row/sigma22) / sqrtSigmaPi2;
  kernel.push_back(1);

  for (unsigned int i = 1; i < rows; i++, row++) {
    // for (row++; row <= r; row++) {
    kernel.push_back(int(exp(-row * row / sigma22) / sqrtSigmaPi2 / first_value));
  }
  return kernel;
}

std::unique_ptr<ImageData>
ImageData::blur(float hradius, float vradius) const {
  unique_ptr<ImageData> r(new ImageData(width, height, num_channels));

  if (num_channels == 4) {
    unsigned char * tmp = new unsigned char[width * height * 4];
    if (hradius > 0.0f) {
      vector<int> hkernel = make_kernel(hradius);
      unsigned short hsize = hkernel.size();

      int htotal = 0;
      for (auto & a : hkernel) htotal += a;

      memset(tmp, 0, width * height * 4);
      for (unsigned int row = 0; row < height; row++) {
        for (unsigned int col = 0; col + hsize <= width; col++) {
          int c0 = 0, c1 = 0, c2 = 0, c3 = 0;
          for (unsigned int i = 0; i < hsize; i++) {
            const unsigned char * ptr = getData() + (row * width + col + i) * 4;
            c0 += *ptr++ * hkernel[i];
            c1 += *ptr++ * hkernel[i];
            c2 += *ptr++ * hkernel[i];
            c3 += *ptr++ * hkernel[i];
          }
          unsigned char * ptr = tmp + (row * width + col + hsize / 2) * 4;
          *ptr++ = (unsigned char)(c0 / htotal);
          *ptr++ = (unsigned char)(c1 / htotal);
          *ptr++ = (unsigned char)(c2 / htotal);
          *ptr++ = (unsigned char)(c3 / htotal);
        }
      }
    } else {
      memcpy(tmp, getData(), width * height * 4);
    }
    if (vradius > 0) {
      vector<int> vkernel = make_kernel(vradius);
      unsigned short vsize = vkernel.size();

      int vtotal = 0;
      for (auto & a : vkernel) vtotal += a;

      memset(r->getData(), 0, width * height * 4);
      for (unsigned int col = 0; col < width; col++) {
        for (unsigned int row = 0; row + vsize <= height; row++) {
          int c0 = 0, c1 = 0, c2 = 0, c3 = 0;
          for (unsigned int i = 0; i < vsize; i++) {
            const unsigned char * ptr = tmp + ((row + i) * width + col) * 4;
            c0 += *ptr++ * vkernel[i];
            c1 += *ptr++ * vkernel[i];
            c2 += *ptr++ * vkernel[i];
            c3 += *ptr++ * vkernel[i];
          }
          unsigned char * ptr = r->getData() + ((row + vsize / 2) * width + col) * 4;
          *ptr++ = (unsigned char)(c0 / vtotal);
          *ptr++ = (unsigned char)(c1 / vtotal);
          *ptr++ = (unsigned char)(c2 / vtotal);
          *ptr++ = (unsigned char)(c3 / vtotal);
        }
      }
    } else {
      memcpy(r->getData(), tmp, width * height * 4);
    }
    delete[] tmp;
  } else if (num_channels == 1) {
    unsigned char * tmp = new unsigned char[width * height];
    if (hradius > 0.0f) {
      vector<int> hkernel = make_kernel(hradius);
      unsigned short hsize = hkernel.size();

      int htotal = 0;
      for (auto & a : hkernel) htotal += a;

      memset(tmp, 0, width * height);
      for (unsigned int row = 0; row < height; row++) {
        for (unsigned int col = 0; col + hsize <= width; col++) {
          int c0 = 0;
          for (unsigned int i = 0; i < hsize; i++) {
            const unsigned char * ptr = getData() + (row * width + col + i);
            c0 += *ptr * hkernel[i];
          }
          unsigned char * ptr = tmp + (row * width + col + hsize / 2);
          *ptr = (unsigned char)(c0 / htotal);
        }
      }
    } else {
      memcpy(tmp, getData(), width * height);
    }
    if (vradius > 0) {
      vector<int> vkernel = make_kernel(vradius);
      unsigned short vsize = vkernel.size();
      int vtotal = 0;
      for (auto & a : vkernel) vtotal += a;

      memset(r->getData(), 0, width * height);
      for (unsigned int col = 0; col < width; col++) {
        for (unsigned int row = 0; row + vsize <= height; row++) {
          int c0 = 0;
          for (unsigned int i = 0; i < vsize; i++) {
            const unsigned char * ptr = tmp + ((row + i) * width + col);
            c0 += *ptr * vkernel[i];
          }
          unsigned char * ptr = r->getData() + ((row + vsize / 2) * width + col);
          *ptr = (unsigned char)(c0 / vtotal);
        }
      }
    } else {
      memcpy(r->getData(), tmp, width * height);
    }
    delete[] tmp;
  }

  return r;
}

