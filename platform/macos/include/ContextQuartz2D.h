#ifndef _CANVAS_CONTEXTQUARTZ2D_H_
#define _CANVAS_CONTEXTQUARTZ2D_H_

#include "Context.h"

#include <QuartzCore/QuartzCore.h>
#include <CoreText/CoreText.h>
// #include <FilenameConverter.h>

#include <unordered_map>
#include <sstream>
#include <iostream>

namespace canvas {
  class Quartz2DCache {
  public:
    Quartz2DCache() { }
    Quartz2DCache(const Quartz2DCache & other) = delete;
    ~Quartz2DCache() {
#ifdef MEMDEBUG
      if (colorspace && CFGetRetainCount(colorspace) != 1) std::cerr << "leaking memory A!\n";
#endif
      CGColorSpaceRelease(colorspace);
      for (auto & fd : fonts) {
#ifdef MEMDEBUG
	if (CFGetRetainCount(fd.second) != 1) std::cerr << "leaking memory B!\n";
#endif
        CFRelease(fd.second);
      }
    }
      
    Quartz2DCache & operator=(const Quartz2DCache & other) = delete;
    
    CTFontRef getFont(const Font & font, float display_scale) {
      float size = font.size * display_scale;
      bool is_bold = font.weight.isBold();
      bool is_italic = font.style == Font::ITALIC;
      std::ostringstream key;
      key << font.family << "/" << size << "/" << (is_bold ? "bold" : "") << "/" << (is_italic ? "italic" : "");
      auto it = fonts.find(key.str());
      if (it != fonts.end()) {
        return it->second;
      } else {
        CTFontRef font2;
        if (is_italic && is_bold) {
          font2 = CTFontCreateWithName(CFSTR("Arial-BoldItalicMT"), size, NULL);
        } else if (is_bold) {
          font2 = CTFontCreateWithName(CFSTR("Arial-BoldMT"), size, NULL);
        } else if (is_italic) {
          font2 = CTFontCreateWithName(CFSTR("Arial-ItalicMT"), size, NULL);
        } else {
          font2 = CTFontCreateWithName(CFSTR("ArialMT"), size, NULL);
        }
        fonts[key.str()] = font2;
        return font2;
      }
    }
    
    CGColorSpaceRef & getColorSpace() {
      if (!colorspace) colorspace = CGColorSpaceCreateDeviceRGB();
      return colorspace;
    }
    
  private:
    CGColorSpaceRef colorspace = 0;
    std::unordered_map<std::string, CTFontRef> fonts;
  };
  
  class Quartz2DSurface : public Surface {
  public:
    friend class ContextQuartz2D;
        
  Quartz2DSurface(const std::shared_ptr<Quartz2DCache> & _cache, unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels)
    : Surface(_logical_width, _logical_height, _actual_width, _actual_height, _num_channels), cache(_cache) {
      if (_actual_width && _actual_height) {
        unsigned int bitmapBytesPerRow = _actual_width * 4;
        unsigned int bitmapByteCount = bitmapBytesPerRow * _actual_height;
        bitmapData = new unsigned char[bitmapByteCount];
        memset(bitmapData, 0, bitmapByteCount);
      }
  }
  
    Quartz2DSurface(const std::shared_ptr<Quartz2DCache> & _cache, const ImageData & image);  
    Quartz2DSurface(const std::shared_ptr<Quartz2DCache> & _cache, const std::string & filename);
    
    ~Quartz2DSurface() {
      if (active_shadow_color) {
#ifdef MEMDEBUG
	if (CFGetRetainCount(active_shadow_color) != 1) std::cerr << "leaking memory C!\n";
#endif
	CGColorRelease(active_shadow_color);
      }
      if (gc) {
#ifdef MEMDEBUG
	if (CFGetRetainCount(gc) != 1) std::cerr << "leaking memory D!\n";
#endif
	CGContextRelease(gc);
      }
      delete[] bitmapData;
    }

    void * lockMemory(bool write_access = false) override { return bitmapData; }
    
    void releaseMemory() override { }

    void renderPath(RenderMode mode, const Path2D & path, const Style & style, float lineWidth, Operator op, float display_scale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) override;

    void resize(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels) override;
    
    void renderText(RenderMode mode, const Font & font, const Style & style, TextBaseline textBaseline, TextAlign textAlign, const std::string & text, const Point & p, float lineWidth, Operator op, float display_scale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) override {
      initializeContext();
      bool has_shadow = shadowBlur > 0.0f || shadowOffsetX != 0.0f || shadowOffsetY != 0.0f;
      if (has_shadow || !clipPath.empty()) {
        CGContextSaveGState(gc);
      }
      if (!clipPath.empty()) {
        sendPath(clipPath, display_scale);
        CGContextClip(gc);
      }
      if (has_shadow) {
	setShadow(shadowOffsetX, shadowOffsetY, shadowBlur, shadowColor, display_scale);
      }

      CFStringRef text2 = CFStringCreateWithCString(NULL, text.c_str(), kCFStringEncodingUTF8);
      if (!text2) {
#ifdef MEMDEBUG
        std::cerr << "failed to create CString from '" << text << "'" << std::endl;
#endif
        return;
      }
      
      CTFontRef font2 = cache->getFont(font, display_scale);
#ifdef MEMDEBUG
      int font_retain = CFGetRetainCount(font2);
      if (font_retain != 1) std::cerr << "too many retains for font (" << font_retain << ")" << std::endl;
#endif
      CGColorRef color = createCGColor(style.color, globalAlpha);
      
#if 0
      int traits = 0;
      if (font.weight.isBold()) traits |= kCTFontBoldTrait;
      if (font.slant == Font::ITALIC) traits |= kCTFontItalicTrait;
      CFNumberRef traits2 = CFNumberCreate(NULL, kCFNumberSInt32Type, &traits);
#endif
      
      CFStringRef keys[] = { kCTFontAttributeName, kCTForegroundColorAttributeName }; // kCTFontSymbolicTrait };
      CFTypeRef values[] = { font2, color }; // traits2
      
      CFDictionaryRef attr = CFDictionaryCreate(NULL, (const void **)&keys, (const void **)&values, sizeof(keys) / sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
      
      CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, text2, attr);
      CTLineRef line = CTLineCreateWithAttributedString(attrString);
      
      double x = p.x * display_scale;
      double y = p.y * display_scale;

      if (textAlign != ALIGN_LEFT || textBaseline == MIDDLE || textBaseline == TOP) {
	CGFloat ascent, descent, leading;
	double width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
	
	switch (textBaseline) {
	case MIDDLE: y += -descent + (ascent + descent) / 2.0; break;
	case TOP: y += ascent; break;
	default: break;
	}
	
	switch (textAlign) {
        case ALIGN_LEFT: break;
        case ALIGN_CENTER: x -= width / 2; break;
        case ALIGN_RIGHT: x -= width; break;
        default: break;
	}
      }
      
      // CGContextSetTextMatrix(gc, CGAffineTransformIdentity);
      CGContextSetTextMatrix(gc, CGAffineTransformMakeScale(1.0, -1.0)); // Use this one if the view's coordinates are flipped
      CGContextSetTextPosition(gc, x, y);
      CTLineDraw(line, gc);

#ifdef MEMDEBUG
      if (CFGetRetainCount(line) != 1) std::cerr << "leaking memory F!\n";
#endif
      CFRelease(line);
#ifdef MEMDEBUG
      if (CFGetRetainCount(attrString) != 1) std::cerr << "leaking memory G!\n";
#endif
      CFRelease(attrString);
#ifdef MEMDEBUG
      if (CFGetRetainCount(attr) != 1) std::cerr << "leaking memory H!\n";
#endif
      CFRelease(attr);
#ifdef MEMDEBUG
      if (CFGetRetainCount(text2) != 1) std::cerr << "leaking memory I!\n";
#endif
      CFRelease(text2);
      // CFRelease(traits2);
#ifdef MEMDEBUG
      int color_retain = CFGetRetainCount(color);
      if (color_retain != 1) std::cerr << "leaking CGColor (" << color_retain << ")!\n";
#endif
      CGColorRelease(color);

      if (has_shadow || !clipPath.empty()) {
        CGContextRestoreGState(gc);
      }
    }

    TextMetrics measureText(const Font & font, const std::string & text, TextBaseline textBaseline, float display_scale) override {
      CTFontRef font2 = cache->getFont(font, display_scale);
      CFStringRef text2 = CFStringCreateWithCString(NULL, text.c_str(), kCFStringEncodingUTF8);
      CFStringRef keys[] = { kCTFontAttributeName };
      CFTypeRef values[] = { font2 };
      CFDictionaryRef attr = CFDictionaryCreate(NULL, (const void **)&keys, (const void **)&values, sizeof(keys) / sizeof(keys[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
      
      CFAttributedStringRef attrString = CFAttributedStringCreate(NULL, text2, attr);
      CTLineRef line = CTLineCreateWithAttributedString(attrString);
      
      CGFloat ascent, descent, leading;
      double width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
      
#ifdef MEMDEBUG
      if (CFGetRetainCount(line) != 1) std::cerr << "leaking memory K!\n";
#endif
      CFRelease(line);
#ifdef MEMDEBUG
      if (CFGetRetainCount(attrString) != 1) std::cerr << "leaking memory L!\n";
#endif
      CFRelease(attrString);
#ifdef MEMDEBUG
      if (CFGetRetainCount(attr) != 1) std::cerr << "leaking memory M!\n";
#endif
      CFRelease(attr);
#ifdef MEMDEBUG
      if (CFGetRetainCount(text2) != 1) std::cerr << "leaking memory N!\n";
#endif
      CFRelease(text2);
      
      return TextMetrics(width / display_scale);
    }

    void drawImage(Surface & surface, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true) override {
      initializeContext();
#if 1
      auto img = surface.createImage(displayScale);
      drawImage(img->getData(), p, w, h, displayScale, globalAlpha, shadowBlur, shadowOffsetX, shadowOffsetY, shadowColor, clipPath, imageSmoothingEnabled);
#else
      _img.initializeContext();
      Quartz2DSurface & img = dynamic_cast<Quartz2DSurface &>(_img);
      CGImageRef myImage = CGBitmapContextCreateImage(img.gc);
      CGContextDrawImage(gc, CGRectMake(x, y, w, h), myImage);
      CGImageRelease(myImage);
#endif
    }
    void drawImage(const ImageData & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled = true) override;
   
  protected:
    void sendPath(const Path2D & path, float display_scale);
    CGColorRef createCGColor(const Color & color, float globalAlpha = 1.0f) {
      CGFloat cv[] = { color.red, color.green, color.blue, color.alpha * globalAlpha};
      return CGColorCreate(cache->getColorSpace(), cv);
    }
    
    void setShadow(float shadowOffsetX, float shadowOffsetY, float shadowBlur, const Color & shadowColor, float displayScale) {
      initializeContext();
      CGSize offset = CGSizeMake(displayScale * shadowOffsetX, displayScale * shadowOffsetY);
      if (active_shadow_color) {
#ifdef MEMDEBUG
	if (CFGetRetainCount(active_shadow_color) != 1) std::cerr << "leaking memory Q!\n";
#endif
	CGColorRelease(active_shadow_color);
      }
      active_shadow_color = createCGColor(shadowColor);
      CGContextSetShadowWithColor(gc, offset, displayScale * shadowBlur, active_shadow_color);
    }
    
    void initializeContext() {
      if (!gc && bitmapData) {
        unsigned int bitmapBytesPerRow = getActualWidth() * 4;
        gc = CGBitmapContextCreate(bitmapData, getActualWidth(), getActualHeight(), 8, bitmapBytesPerRow, cache->getColorSpace(),
                                   (getNumChannels() == 4 ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNoneSkipLast)); // | kCGBitmapByteOrder32Big);
        CGContextSetInterpolationQuality(gc, kCGInterpolationHigh);
        CGContextSetShouldAntialias(gc, true);
        flipY();
      }
    }

    void flipY() {
      CGContextTranslateCTM(gc, 0, getActualHeight());
      CGContextScaleCTM(gc, 1.0, -1.0);
    }

    std::unique_ptr<Image> createImage(float display_scale) override;
    
  private:
    std::shared_ptr<Quartz2DCache> cache;
    CGContextRef gc = 0;
    unsigned char * bitmapData = 0;
    CGColorRef active_shadow_color = 0;
  };

  class ContextQuartz2D : public Context {
  public:
  ContextQuartz2D(const std::shared_ptr<Quartz2DCache> & _cache, unsigned int _width, unsigned int _height, unsigned int _num_channels, float _display_scale)
    : Context(_display_scale),
      cache(_cache),
      default_surface(_cache, _width, _height, (unsigned int)(_width * _display_scale), (unsigned int)(_height * _display_scale), _num_channels)
      {
      }

    std::unique_ptr<Surface> createSurface(const ImageData & image) override {
        return std::unique_ptr<Surface>(new Quartz2DSurface(cache, image));
    }
    std::unique_ptr<Surface> createSurface(unsigned int _width, unsigned int _height, unsigned int _num_channels) override {
      return std::unique_ptr<Surface>(new Quartz2DSurface(cache, _width, _height, (unsigned int)(_width * getDisplayScale()), (unsigned int)(_height * getDisplayScale()), _num_channels));
    }
        
    Surface & getDefaultSurface() override { return default_surface; }
    const Surface & getDefaultSurface() const override { return default_surface; }
    
  protected:
    bool hasNativeShadows() const override { return true; }
    bool hasNativeEmoticons() const override { return true; }
    
  private:
    std::shared_ptr<Quartz2DCache> cache;
    Quartz2DSurface default_surface;
  };

  class Quartz2DContextFactory : public ContextFactory {
  public:
    Quartz2DContextFactory(float _display_scale) : ContextFactory(_display_scale) {
      cache = std::make_shared<Quartz2DCache>();
  }
    std::unique_ptr<Context> createContext(unsigned int width, unsigned int height, unsigned int num_channels) override {
      return std::unique_ptr<Context>(new ContextQuartz2D(cache, width, height, num_channels, getDisplayScale()));
    }
    std::unique_ptr<Surface> createSurface(unsigned int width, unsigned int height, unsigned int num_channels) override {
      unsigned int aw = width * getDisplayScale();
      unsigned int ah = height * getDisplayScale();
        return std::unique_ptr<Surface>(new Quartz2DSurface(cache, width, height, aw, ah, num_channels));
    }

    std::unique_ptr<Image> createImage() override;
    std::unique_ptr<Image> createImage(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels) override;

  private:
    std::shared_ptr<Quartz2DCache> cache;
  };
};

#endif
