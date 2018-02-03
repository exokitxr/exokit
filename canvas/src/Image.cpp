#include <Image.h>

#include <ImageLoadingException.h>

#include <nanosvg.h>
#include <nanosvgrast.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cassert>
// #include <iostream>

using namespace std;
using namespace canvas;

static NSVGrasterizer *svgRasterizer = nsvgCreateRasterizer();

bool
Image::decode(const unsigned char * buffer, size_t size) {
  data = loadFromMemory(buffer, size);
  return (bool)data;
}

std::unique_ptr<ImageData>
Image::loadFromMemory(const unsigned char * buffer, size_t size) {
  int w, h, channels;
  unsigned char *imgDataBuffer = stbi_load_from_memory(buffer, size, &w, &h, &channels, 4);
  if (imgDataBuffer) {
    // cerr << "Image.cpp: loaded image, size = " << size << ", b = " << (void*)imgDataBuffer << ", w = " << w << ", h = " << h << ", ch = " << channels << endl;
    assert(w && h && channels);

    std::unique_ptr<ImageData> data = std::unique_ptr<ImageData>(new ImageData(imgDataBuffer, w, h, 4));

    stbi_image_free(imgDataBuffer);

    return data;
  } else {
    unique_ptr<char[]> svgString(new char[size + 1]);
    memcpy(svgString.get(), buffer, size);
    svgString[size] = 0;

    NSVGimage *svgImage = nsvgParse(svgString.get(), "px", 96);
    if (svgImage != nullptr) {
      if (svgImage->width > 0 && svgImage->height > 0 && svgImage->shapes != nullptr) {
        int w = svgImage->width;
        int h = svgImage->height;
        unsigned char *imgDataBuffer = (unsigned char *)malloc(w * h * 4);
        nsvgRasterize(svgRasterizer, svgImage, 0, 0, 1, imgDataBuffer, w, h, w * 4);

        std::unique_ptr<ImageData> data = std::unique_ptr<ImageData>(new ImageData(imgDataBuffer, w, h, 4));

        free(svgImage);

        // std::cout << "image decode ok" << "\n";

        return data;
      } else {
        free(svgImage);

        // std::cout << "image decode fail 1" << "\n";

        return nullptr;
      }
    } else {
      // std::cout << "image decode fail 2" << "\n";
      // throw ImageLoadingException(stbi_failure_reason());
      return nullptr;
    }
  }
}

std::unique_ptr<ImageData>
Image::loadFromFile(const std::string & filename) {
  assert(!filename.empty());
  int w, h, channels;
  auto img_buffer = stbi_load(filename.c_str(), &w, &h, &channels, 0);
  if (!img_buffer) {
    throw ImageLoadingException(stbi_failure_reason());
  }
  assert(w && h && channels);    

  std::unique_ptr<ImageData> data = std::unique_ptr<ImageData>(new ImageData((unsigned char *)img_buffer, w, h, channels));
  
  stbi_image_free(img_buffer);

  return data;
}

bool
Image::isPNG(const unsigned char * buffer, size_t size) {
  return size >= 4 && buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4e && buffer[3] == 0x47;
}

bool
Image::isJPEG(const unsigned char * buffer, size_t size) {
  return size >= 3 && buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff;
}

bool
Image::isGIF(const unsigned char * buffer, size_t size) {
  return size >= 6 && buffer[0] == 'G' && buffer[1] == 'I' && buffer[2] == 'F' && buffer[3] == '8' && (buffer[4] == '7' || buffer[4] == '9') && buffer[5] == 'a';
}

bool
Image::isBMP(const unsigned char * buffer, size_t size) {
  return size > 2 && buffer[0] == 0x42 && buffer[1] == 0x4d;
}

bool
Image::isXML(const unsigned char * buffer, size_t size) {
  return size >= 6 && buffer[0] == '<' && buffer[1] == '!' && buffer[2] == 'D' && buffer[3] == 'O' && buffer[4] == 'C' && buffer[5] == 'T';
}
