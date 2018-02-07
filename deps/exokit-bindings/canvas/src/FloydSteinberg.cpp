#include "FloydSteinberg.h"

#include <ImageData.h>
#include <ImageFormat.h>

#include <cassert>
#include <vector>

using namespace std;
using namespace canvas;

struct rgba_s {
  rgba_s() : red(0), green(0), blue(0), alpha(0) { }
  rgba_s(unsigned int _red, unsigned int _green, unsigned int _blue, unsigned int _alpha = 0) : red(_red), green(_green), blue(_blue), alpha(_alpha) { }
  unsigned int red;
  unsigned int green;
  unsigned int blue;
  unsigned int alpha;

  inline void setError(unsigned int weight, unsigned int error) {
    red = weight * (RGBA_TO_RED(error) >> 4);
    green = weight * (RGBA_TO_GREEN(error) >> 4);
    blue = weight * (RGBA_TO_BLUE(error) >> 4);
  }

  inline void addError(unsigned int weight, unsigned int error) {
    red += weight * (RGBA_TO_RED(error) >> 4);
    green += weight * (RGBA_TO_GREEN(error) >> 4);
    blue += weight * (RGBA_TO_BLUE(error) >> 4);
  }

  inline void setErrorAlpha(unsigned int weight, unsigned int error) {
    red = weight * (RGBA_TO_RED(error) >> 4);
    green = weight * (RGBA_TO_GREEN(error) >> 4);
    blue = weight * (RGBA_TO_BLUE(error) >> 4);
    alpha = weight * (RGBA_TO_ALPHA(error) >> 4);
  }

  inline void addErrorAlpha(unsigned int weight, unsigned int error) {
    red += weight * (RGBA_TO_RED(error) >> 4);
    green += weight * (RGBA_TO_GREEN(error) >> 4);
    blue += weight * (RGBA_TO_BLUE(error) >> 4);
    alpha += weight * (RGBA_TO_ALPHA(error) >> 4);
  }
};

static unsigned int apply2(unsigned int * input_data, unsigned int width, unsigned int height, InternalFormat target_format, unsigned short * output_data) {
  unsigned int * input = input_data;

  if (target_format == RGBA4) {
    vector<rgba_s> old_errors(width + 2);
    for (unsigned int y = 0; y < height; y++) {
      vector<rgba_s> new_errors(width + 2);
      rgba_s next_error;
      for (unsigned int x = 0; x < width; x++, input++) {
        unsigned int v0 = *input;
        unsigned int red = RGBA_TO_RED(v0) + next_error.red + old_errors[x + 1].red;
        unsigned int green = RGBA_TO_GREEN(v0) + next_error.green + old_errors[x + 1].green;
        unsigned int blue = RGBA_TO_BLUE(v0) + next_error.blue + old_errors[x + 1].blue;
        unsigned int alpha = RGBA_TO_ALPHA(v0) + next_error.alpha + old_errors[x + 1].alpha;
        if (red > 255) red = 255;
        if (green > 255) green = 255;
        if (blue > 255) blue = 255;
        if (alpha > 255) alpha = 255;
        unsigned int v = PACK_RGBA32(red, green, blue, alpha);
        unsigned int error = v & 0x0f0f0f0f;
#if defined __APPLE__ || defined __ANDROID__
        *output_data++ = ((RGBA_TO_RED(v) >> 4) << 12) | ((RGBA_TO_GREEN(v) >> 4) << 8) | ((RGBA_TO_BLUE(v) >> 4) << 4) | (RGBA_TO_ALPHA(v) >> 4);
#else
        *output_data++ = ((RGBA_TO_BLUE(v) >> 4) << 12) | ((RGBA_TO_GREEN(v) >> 4) << 8) | ((RGBA_TO_RED(v) >> 4) << 4) | (RGBA_TO_ALPHA(v) >> 4);
#endif
        next_error.setErrorAlpha(7, error);
        new_errors[x].addErrorAlpha(3, error);
        new_errors[x + 1].addErrorAlpha(5, error);
        new_errors[x + 2].addErrorAlpha(1, error);
      }
      old_errors.swap(new_errors);
    }
  } else {
    vector<rgba_s> old_errors(width + 2);
    for (unsigned int y = 0; y < height; y++) {
      vector<rgba_s> new_errors(width + 2);
      rgba_s next_error;
      for (unsigned int x = 0; x < width; x++, input++) {
        unsigned int v0 = *input;
        unsigned int red = RGBA_TO_RED(v0) + next_error.red + old_errors[x + 1].red;
        unsigned int green = RGBA_TO_GREEN(v0) + next_error.green + old_errors[x + 1].green;
        unsigned int blue = RGBA_TO_BLUE(v0) + next_error.blue + old_errors[x + 1].blue;
        if (red > 255) red = 255;
        if (green > 255) green = 255;
        if (blue > 255) blue = 255;
        unsigned int v = PACK_RGB24(red, green, blue);
        unsigned int error = v & 0x00070307;
#if defined __APPLE__ || defined __ANDROID__
        *output_data++ = PACK_RGB565(RGBA_TO_BLUE(v) >> 3, RGBA_TO_GREEN(v) >> 2, RGBA_TO_RED(v) >> 3);
#else
        *output_data++ = PACK_RGB565(RGBA_TO_RED(v) >> 3, RGBA_TO_GREEN(v) >> 2, RGBA_TO_BLUE(v) >> 3);
#endif
        next_error.setError(7, error);
        new_errors[x].addError(3, error);
        new_errors[x + 1].addError(5, error);
        new_errors[x + 2].addError(1, error);
      }
      old_errors.swap(new_errors);
    }
  }

  return width * height * 2;
}

unsigned int
FloydSteinberg::apply(const ImageData & input_image, unsigned char * output) const {
  unsigned int width = input_image.getWidth();
  unsigned int height = input_image.getHeight();

  auto data = input_image.getData();

  auto input_data = std::unique_ptr<unsigned int[]>(new unsigned int[width * height]);

  if (input_image.getNumChannels() == 4) {
    memcpy(input_data.get(), data, 4 * width * height);
  } else if (input_image.getNumChannels() == 3) {
    auto tmp = input_data.get();
    for (unsigned int offset = 0; offset < 3 * width * height; offset += 3) {
      *tmp++ = (0xff << 24) | (data[offset + 2] << 16) | (data[offset + 1] << 8) | (data[offset + 0]);
    }
  } else if (input_image.getNumChannels() == 1) {
    auto tmp = input_data.get();
    for (unsigned int offset = 0; offset < width * height; ) {
      unsigned char v = data[offset++];
      *tmp++ = (0xff << 24) | (v << 16) | (v << 8) | (v);
    }
  }

  return apply2(input_data.get(), width, height, target_format, (unsigned short *)output);
}
