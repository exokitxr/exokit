#ifndef _IMAGEDATA_H_
#define _IMAGEDATA_H_

#include <Color.h>

#include <cstring>
#include <memory>

namespace canvas {
  class ImageData {
  public:

  ImageData() : width(0), height(0), num_channels(0) { }
  ImageData(const unsigned char * _data, unsigned short _width, unsigned short _height, unsigned short _num_channels)
    : width(_width), height(_height), num_channels(_num_channels)
    {
      size_t s = calculateSize();
      data = std::unique_ptr<unsigned char[]>(new unsigned char[s]);
      if (!_data) {
	memset(data.get(), 0, s);
      } else {
	memcpy(data.get(), _data, s);
      }
    }
  ImageData(unsigned short _width, unsigned short _height, unsigned short _num_channels)
    : width(_width), height(_height), num_channels(_num_channels) {
      size_t s = calculateSize();
      data = std::unique_ptr<unsigned char[]>(new unsigned char[s]);
      memset(data.get(), 0, s);  
    }

    ImageData(const ImageData & other)
      : width(other.getWidth()), height(other.getHeight()), num_channels(other.num_channels)
    {
      size_t s = calculateSize();
      data = std::unique_ptr<unsigned char[]>(new unsigned char[s]);
      if (other.getData()) {
	memcpy(data.get(), other.getData(), s);
      } else {
	memset(data.get(), 0, s);
      }
    }

    ImageData & operator=(const ImageData & other) = delete;
    
    std::unique_ptr<ImageData> crop(int x, int y, unsigned short w, unsigned short h, bool flipY = false) const;
    std::unique_ptr<ImageData> scale(unsigned short target_width, unsigned short target_height) const;
    std::unique_ptr<ImageData> colorize(const Color & color) const;
    std::unique_ptr<ImageData> blur(float hradius, float vradius) const;

    bool isValid() const { return width != 0 && height != 0 && num_channels != 0; }
    unsigned short getWidth() const { return width; }
    unsigned short getHeight() const { return height; }
    unsigned short getNumChannels() const { return num_channels; }

    unsigned char * getData() const { return data.get(); }
    
    static size_t calculateSize(unsigned short width, unsigned short height, unsigned short num_channels) { return width * height * num_channels; }
    size_t calculateSize() const { return calculateSize(width, height, num_channels); }

    static bool getFlip() { return ImageData::flip; }
    static void setFlip(bool newFlip) { ImageData::flip = newFlip; }
  private:
    static bool flip;

    unsigned short width, height, num_channels;
    std::unique_ptr<unsigned char[]> data;
  };
};
#endif
