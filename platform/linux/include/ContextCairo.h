#include "Context.h"

#include <cairo/cairo.h>

namespace canvas {
  class ContextCairo;

  class CairoSurface : public Surface {
  public:
    friend class ContextCairo;

    CairoSurface(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels);
    CairoSurface(const ImageData & image);
    CairoSurface(const CairoSurface & other) = delete;
    CairoSurface(const unsigned char * buffer, size_t size);
    ~CairoSurface();
    
    void resize(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels) override;

    void renderPath(RenderMode mode, const Path2D & path, const Style & style, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath);
    void renderText(RenderMode mode, const Font & font, const Style & style, TextBaseline textBaseline, TextAlign textAlign, const std::string & text, const Point & p, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath);
    TextMetrics measureText(const Font & font, const std::string & text, TextBaseline textBaseline, float displayScale);
    void drawImage(Surface & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true);
    void drawImage(const ImageData & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true);

    std::unique_ptr<Image> createImage(float display_scale) override;

    cairo_surface_t * getCairoSurface() { return surface; }
    
  protected:
    void flush();
    void markDirty();

    void * lockMemory(bool write_access = false) override {
      flush();
      locked_for_write = write_access;
      return cairo_image_surface_get_data(surface);
    }
    void releaseMemory() override {
      if (locked_for_write) {
	locked_for_write = false;
	markDirty();
      }
    }

    void initializeContext() {
      if (!cr) {
	if (!surface) {
	  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 4, 4);
	}
	cr = cairo_create(surface);	
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
      }
    }

    void drawNativeSurface(CairoSurface & img, const Point & p, double w, double h, float displayScale, float globalAlpha, const Path2D & clipPath, bool imageSmoothingEnabled);

    void sendPath(const Path2D & path);

  private:
    cairo_t * cr = 0;
    cairo_surface_t * surface;
    unsigned int * storage = 0;
    bool locked_for_write = false;
  };

  class ContextCairo : public Context {
  public:
  ContextCairo(unsigned int _width, unsigned int _height, unsigned int _num_channels = 4, float _displayScale = 1.0f)
      : Context(_displayScale),
      default_surface(_width, _height, (unsigned int)(_displayScale * _width), (unsigned int)(_displayScale * _height), _num_channels)
	{ 
	}
    
    std::unique_ptr<Surface> createSurface(const ImageData & image) {
      return std::unique_ptr<Surface>(new CairoSurface(image));
    }
    std::unique_ptr<Surface> createSurface(unsigned int _width, unsigned int _height, unsigned int _num_channels = 4) {
      return std::unique_ptr<Surface>(new CairoSurface(_width, _height, (unsigned int)(_width * getDisplayScale()), (unsigned int)(_height * getDisplayScale()), _num_channels));
    }

    CairoSurface & getDefaultSurface() { return default_surface; }
    const CairoSurface & getDefaultSurface() const { return default_surface; }
    
  protected:
    CairoSurface default_surface;
  };

  class CairoContextFactory : public ContextFactory {
  public:
   CairoContextFactory() : ContextFactory(1.0f) { }
    std::unique_ptr<Context> createContext(unsigned int width, unsigned int height, unsigned int num_channels) override {
      return std::unique_ptr<Context>(new ContextCairo(width, height, num_channels));
    }
    std::unique_ptr<Surface> createSurface(unsigned int width, unsigned int height, unsigned int num_channels) override {
      unsigned int aw = width * getDisplayScale(), ah = height * getDisplayScale();
      return std::unique_ptr<Surface>(new CairoSurface(width, height, aw, ah, num_channels));
    }
    std::unique_ptr<Image> loadImage(const std::string & filename) override;
    std::unique_ptr<Image> createImage() override;
    std::unique_ptr<Image> createImage(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels) override;
  };
};
