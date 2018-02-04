#include "Context.h"

#define NOMINMAX
#include <algorithm>
namespace Gdiplus
{
  using std::min;
  using std::max;
};

#undef WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gdiplus.h>

#include <iostream>
#include <vector>

#pragma comment (lib, "gdiplus.lib")

namespace canvas {
  class GDIPlusSurface : public Surface {
  public:
    friend class ContextGDIPlus;

  GDIPlusSurface(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int numChannels)
    : Surface(_actual_width, _actual_height, _logical_width, _logical_height, numChannels) {
      if (_actual_width && _actual_height) {
	bitmap = std::unique_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap(_actual_width, _actual_height, numChannels >= 4 ? PixelFormat32bppPARGB : PixelFormat32bppRGB));
      }
    }
    GDIPlusSurface(const std::string & filename);
	GDIPlusSurface(const unsigned char * buffer, size_t size);
  GDIPlusSurface(const ImageData & imageData) : Surface(imageData.getWidth(), imageData.getHeight(), imageData.getWidth(), imageData.getHeight(), imageData.getNumChannels())
    {
      // stride must be a multiple of four
      size_t numPixels = imageData.getWidth() * imageData.getHeight();
      size_t numChannels = imageData.getNumChannels();
      storage = new BYTE[numPixels * numChannels];
      memcpy(storage, imageData.getData(), numPixels * numChannels);
      
      bitmap = std::unique_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap(imageData.getWidth(), imageData.getHeight(), imageData.getWidth() * numChannels, numChannels >= 4 ? PixelFormat32bppPARGB : PixelFormat32bppRGB, storage));
      // can the storage be freed here?
    }
    ~GDIPlusSurface() {
      delete[] storage;
    }
    void resize(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int numChannels) {
      Surface::resize(_logical_width, _logical_height, _actual_width, _actual_height, numChannels);
      bitmap = std::unique_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap(_actual_width, _actual_height, numChannels >= 4 ? PixelFormat32bppPARGB : PixelFormat32bppRGB ));
      g = std::unique_ptr<Gdiplus::Graphics>();
    }

    void renderText(RenderMode mode, const Font & font, const Style & style, TextBaseline textBaseline, TextAlign textAlign, const std::string & text, const Point & p, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) override;
    TextMetrics measureText(const Font & font, const std::string & text, TextBaseline textBaseline, float displayScale);

    void renderPath(RenderMode mode, const Path2D & path, const Style & style, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) override;
    
    void drawImage(Surface & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true);
    void drawImage(const ImageData & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true) {
      GDIPlusSurface cs(_img);
      drawNativeSurface(cs, p, w, h, displayScale, globalAlpha, imageSmoothingEnabled);
    }
    
    std::unique_ptr<Image> createImage(float display_scale) override;
    
    void clip(const Path2D & path, float display_scale);
    void save() {
      initializeContext();
      save_stack.push_back(g->Save());
    }
    void restore() {
      initializeContext();
      if (!save_stack.empty()) {
	g->Restore(save_stack.back());
	save_stack.pop_back();
      }
    }

    void * lockMemory(bool write_access = false) {
      if (bitmap.get()) {
	Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
	bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead | (write_access ? Gdiplus::ImageLockModeWrite : 0), getNumChannels() >= 4 ? PixelFormat32bppPARGB : PixelFormat32bppRGB, &data);
	return data.Scan0;
      } else {
	return 0;
      }
    }
      
    void releaseMemory() {
      bitmap->UnlockBits(&data);
    }

  protected:
    void initializeContext() {
      if (!g.get()) {
	if (!bitmap.get()) {
	  bitmap = std::unique_ptr<Gdiplus::Bitmap>(new Gdiplus::Bitmap(4, 4, PixelFormat32bppPARGB));
	}
	g = std::unique_ptr<Gdiplus::Graphics>(new Gdiplus::Graphics(bitmap.get()));
#if 0
	g->SetPixelOffsetMode( PixelOffsetModeNone );
#endif
	g->SetCompositingQuality( Gdiplus::CompositingQualityHighQuality );
	g->SetSmoothingMode( Gdiplus::SmoothingModeAntiAlias );
      }
    }
    void drawNativeSurface(GDIPlusSurface & img, const Point & p, double w, double h, float displayScale, float globalAlpha, bool imageSmoothingEnabled);

  private:
    std::unique_ptr<Gdiplus::Bitmap> bitmap;
    std::unique_ptr<Gdiplus::Graphics> g;
    Gdiplus::BitmapData data;     
    std::vector<Gdiplus::GraphicsState> save_stack;
    BYTE * storage = 0;
  };
  
  class ContextGDIPlus : public Context {
  public:
    ContextGDIPlus(unsigned int _width, unsigned int _height, unsigned int numChannels, float _display_scale = 1.0f)
      : Context(_display_scale),
	default_surface(_width, _height, (unsigned int)(_display_scale * _width), (unsigned int)(_display_scale * _height), numChannels)
    {
    
    }

    static void initialize() {
      if (!is_initialized) {
	// Initialize GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
	is_initialized = true;
      }
    }
    
    virtual std::unique_ptr<Surface> createSurface(const ImageData & image) {
      return std::unique_ptr<Surface>(new GDIPlusSurface(image));
    }

    std::unique_ptr<Surface> createSurface(unsigned int _width, unsigned int _height, unsigned int numChannels) {
      return std::unique_ptr<Surface>(new GDIPlusSurface(_width, _height, (unsigned int)(_width * getDisplayScale()), (unsigned int)(_height * getDisplayScale()), numChannels));
    }

    GDIPlusSurface & getDefaultSurface() { return default_surface; }
    const GDIPlusSurface & getDefaultSurface() const { return default_surface; }

  private:   
    GDIPlusSurface default_surface;
    static bool is_initialized;
    static ULONG_PTR m_gdiplusToken;
  };

  class GDIPlusContextFactory : public ContextFactory  {
  public:
   GDIPlusContextFactory() : ContextFactory(1.0f) { }
    std::unique_ptr<Context> createContext(unsigned int width, unsigned int height, unsigned int numChannels) override {
      return std::make_unique<ContextGDIPlus>(width, height, numChannels, getDisplayScale());
    }
    std::unique_ptr<Surface> createSurface(unsigned int width, unsigned int height, unsigned int numChannels) override {
      unsigned int aw = width * getDisplayScale(), ah = height * getDisplayScale();
      return std::make_unique<GDIPlusSurface>(width, height, aw, ah, numChannels);
    }
    std::unique_ptr<Image> createImage() {
      return std::unique_ptr<Image>(new Image(1));
    }
    std::unique_ptr<Image> createImage(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels) {
      return std::unique_ptr<Image>(new Image(_data, _width, _height, _num_channels, 1));
    }
  };
};
