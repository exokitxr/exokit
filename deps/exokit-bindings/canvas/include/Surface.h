#ifndef _SURFACE_H_
#define _SURFACE_H_

#include <Color.h>
#include <Path2D.h>
#include <Style.h>
#include <Font.h>
#include <TextBaseline.h>
#include <TextAlign.h>
#include <TextMetrics.h>
#include <Operator.h>
#include <InternalFormat.h>
#include <ImageData.h>
#include <PackedImageData.h>

#include <memory>
#include <cassert>

namespace canvas {
  class Context;
  class ImageData;
  class Image;

  enum RenderMode {
    FILL = 1,
    STROKE
  };

  class Surface {
  public:
    friend class Context;

  Surface(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels) :
    logical_width(_logical_width),
      logical_height(_logical_height),
      actual_width(_actual_width),
      actual_height(_actual_height),
      num_channels(_num_channels)
	{ }
    
    Surface(const Surface & other) = delete;
    Surface & operator=(const Surface & other) = delete;
    virtual ~Surface() = default;

    virtual void resize(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels) {
      logical_width = _logical_width;
      logical_height = _logical_height;
      actual_width = _actual_width;
      actual_height = _actual_height;
      num_channels = _num_channels;
    }

    virtual void renderPath(RenderMode mode, const Path2D & path, const Style & style, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) = 0;
    virtual void renderText(RenderMode mode, const Font & font, const Style & style, TextBaseline textBaseline, TextAlign textAlign, const std::string & text, const Point & p, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) = 0;
    virtual TextMetrics measureText(const Font & font, const std::string & text, TextBaseline textBaseline, float displayScale) = 0;
	  
    virtual void drawImage(Surface & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true) = 0;
    virtual void drawImage(const ImageData & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true) = 0;
    virtual std::unique_ptr<Image> createImage(float display_scale) = 0;

    std::unique_ptr<PackedImageData> createPackedImage() {
      unsigned char * buffer = (unsigned char *)lockMemory(false);
      assert(buffer);
      if (buffer) {
	std::unique_ptr<PackedImageData> image(new PackedImageData(RGBA8, getActualWidth(), getActualHeight(), 1, buffer));
	releaseMemory();
	return image;
      } else {
	return std::unique_ptr<PackedImageData>(new PackedImageData(RGBA8, getActualWidth(), getActualHeight(), 1));
      }
    }

    std::unique_ptr<ImageData> blur(float hradius, float vradius) {
      ImageData tmp((unsigned char *)lockMemory(false), getActualWidth(), getActualHeight(), getNumChannels());
      auto r = tmp.blur(hradius, vradius);
      releaseMemory();
      return r;
    }
    
    std::unique_ptr<ImageData> colorize(const Color & color) {
      ImageData tmp((unsigned char *)lockMemory(false), getActualWidth(), getActualHeight(), getNumChannels());
      auto r = tmp.colorize(color);
      releaseMemory();
      return r;
    }

    unsigned int getLogicalWidth() const { return logical_width; }
    unsigned int getLogicalHeight() const { return logical_height; }
    unsigned int getActualWidth() const { return actual_width; }
    unsigned int getActualHeight() const { return actual_height; }
    unsigned int getNumChannels() const { return num_channels; }
    
  // protected:
    virtual void * lockMemory(bool write_access = false) = 0;
    virtual void releaseMemory() = 0;

  private:
    unsigned int logical_width, logical_height, actual_width, actual_height, num_channels;
  };
};

#endif
