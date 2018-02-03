#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <ImageData.h>
#include <PackedImageData.h>

#include <string>

namespace canvas {
  class Image {
  public:
    Image(float _display_scale) : data(new ImageData()), display_scale(_display_scale) { }
    Image(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels, float _display_scale)
      : data(new ImageData(_data, _width, _height, _num_channels)),
        display_scale(_display_scale) { }

    virtual ~Image() = default;

    bool decode(const unsigned char * buffer, size_t size);
    void scale(unsigned int target_width, unsigned int target_height) {
      if (data.get()) {
	unsigned int width = (unsigned int)(target_width * display_scale);
	unsigned int height = (unsigned int)(target_height * display_scale);
	data = data->scale(width, height);
      }
    }

    ImageData & getData() const {
      return *data;
    }

    std::string getFilename() const { return filename; }

    void setDisplayScale(float f) { display_scale = f; }
    float getDisplayScale() const { return display_scale; }

    std::unique_ptr<PackedImageData> pack(InternalFormat format, int num_levels) const {
      return std::unique_ptr<PackedImageData>(new PackedImageData(format, num_levels, *data));
    }
    
    static bool isPNG(const unsigned char * buffer, size_t size);
    static bool isJPEG(const unsigned char * buffer, size_t size);
    static bool isGIF(const unsigned char * buffer, size_t size);
    static bool isBMP(const unsigned char * buffer, size_t size);
    static bool isXML(const unsigned char * buffer, size_t size);

  protected:
    static std::unique_ptr<ImageData> loadFromMemory(const unsigned char * buffer, size_t size);
    
    std::string filename;
    std::unique_ptr<ImageData> data;

  private:
    float display_scale;
  };
};
#endif
