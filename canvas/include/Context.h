#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <GraphicsState.h>

#include <Color.h>
#include <Surface.h>
#include <Image.h>
#include <HitRegion.h>

#include <string>
#include <memory>

namespace canvas {
  class Context : public GraphicsState {
  public:
    Context(float _display_scale = 1.0f)
      : display_scale(_display_scale),
      current_linear_gradient(this)
      { }
    Context(const Context & other) = delete;
    Context & operator=(const Context & other) = delete;
    Context & operator=(const GraphicsState & other) {
      GraphicsState::operator=(other);
      return *this;
    }
    virtual ~Context() = default;

    virtual std::unique_ptr<Surface> createSurface(const ImageData & image) = 0;
    virtual std::unique_ptr<Surface> createSurface(unsigned int _width, unsigned int _height, unsigned int _num_channels) = 0;
    virtual Surface & getDefaultSurface() = 0;
    virtual const Surface & getDefaultSurface() const = 0;

    virtual bool hasNativeShadows() const { return false; }
    virtual bool hasNativeEmoticons() const { return false; }

    virtual void resize(unsigned int _width, unsigned int _height) {
      getDefaultSurface().resize(_width, _height, (unsigned int)(_width * getDisplayScale()), (unsigned int)(_height * getDisplayScale()), getDefaultSurface().getNumChannels());
      hit_regions.clear();
    }
        
    Context & stroke() { return renderPath(STROKE, currentPath, strokeStyle); }
    Context & stroke(const Path2D & path) { return renderPath(STROKE, path, strokeStyle); }
    Context & fill() { return renderPath(FILL, currentPath, fillStyle); }
    Context & fill(const Path2D & path) { return renderPath(FILL, path, fillStyle); }
    Context & save() {
      restore_stack.push_back(*this);
      return *this;
    }
    Context & restore() {
      if (!restore_stack.empty()) {
	*this = restore_stack.back();
	restore_stack.pop_back();    
      }
      return *this;
    }
    
    bool isPointInPath(const Path2D & path, double x, double y) { return false; }
    
    TextMetrics measureText(const std::string & text) {
      return getDefaultSurface().measureText(font, text, textBaseline.get(), getDisplayScale());
    }
    
    Context & fillRect(double x, double y, double w, double h) {
      beginPath().rect(x, y, w, h);
      return fill();
    }
    
    Context & strokeRect(double x, double y, double w, double h) {
      beginPath().rect(x, y, w, h);
      return stroke();
    }
    
    Context & clearRect(double x, double y, double w, double h) {
      Path2D path;
      path.moveTo(currentTransform.multiply(x, y));
      path.lineTo(currentTransform.multiply(x + w, y));
      path.lineTo(currentTransform.multiply(x + w, y + h));
      path.lineTo(currentTransform.multiply(x, y + h));
      path.closePath();
      Style style(this);
      style = Color(0.0f, 0.0f, 0.0f, 0.0f);
      return renderPath(FILL, path, style, COPY);
    }
    
    Context & fillText(const std::string & text, double x, double y) { return renderText(FILL, fillStyle, text, currentTransform.multiply(x, y)); }
    Context & strokeText(const std::string & text, double x, double y) { return renderText(STROKE, strokeStyle, text, currentTransform.multiply(x, y)); }
    
    unsigned int getWidth() const { return getDefaultSurface().getLogicalWidth(); }
    unsigned int getHeight() const { return getDefaultSurface().getLogicalHeight(); }
    unsigned int getActualWidth() const { return getDefaultSurface().getActualWidth(); }
    unsigned int getActualHeight() const { return getDefaultSurface().getActualHeight(); }

    Context & drawImage(Context & other, double x, double y, double w, double h) {
      return drawImage(other.getDefaultSurface(), x, y, w, h);
    }
    
    Context & drawImage(Image & img, double x, double y, double w, double h) {
      return drawImage(img.getData(), x, y, w, h);
    }
    
    virtual Context & drawImage(const ImageData & img, double x, double y, double w, double h) {
      Point p = currentTransform.multiply(x, y);
      if (hasNativeShadows()) {
	getDefaultSurface().drawImage(img, p, w, h, getDisplayScale(), globalAlpha.get(), shadowBlur.get(), shadowOffsetX.get(), shadowOffsetY.get(), shadowColor.get(), clipPath, imageSmoothingEnabled.get());
      } else {
	if (hasShadow()) {
	  float b = shadowBlur.get(), bs = shadowBlur.get() * getDisplayScale();
	  float bi = int(ceil(b));
	  auto shadow = createSurface(getDefaultSurface().getLogicalWidth() + 2 * bi, getDefaultSurface().getLogicalHeight() + 2 * bi, R8);
	  shadow->drawImage(img, Point(x + b + shadowOffsetX.get(), y + b + shadowOffsetY.get()), w, h, getDisplayScale(), globalAlpha.get(), 0.0f, 0.0f, 0.0f, shadowColor.get(), clipPath, imageSmoothingEnabled.get());
	  // shadow->colorFill(shadowColor.get());
	  auto shadow1 = shadow->blur(bs, bs);
	  auto shadow2 = shadow1->colorize(shadowColor.get());
	  getDefaultSurface().drawImage(*shadow2, Point(-b, -b), shadow->getLogicalWidth(), shadow->getLogicalHeight(), getDisplayScale(), 1.0f, 0.0f, 0.0f, 0.0f, shadowColor.get(), Path2D(), false);
	}
	getDefaultSurface().drawImage(img, p, w, h, getDisplayScale(), globalAlpha.get(), 0.0f, 0.0f, 0.0f, shadowColor.get(), clipPath, imageSmoothingEnabled.get());
      }
      return *this;
    }
    
    virtual Context & drawImage(Surface & img, double x, double y, double w, double h) {
      Point p = currentTransform.multiply(x, y);
      if (hasNativeShadows()) {
	getDefaultSurface().drawImage(img, p, w, h, getDisplayScale(), globalAlpha.get(), shadowBlur.get(), shadowOffsetX.get(), shadowOffsetY.get(), shadowColor.get(), clipPath, imageSmoothingEnabled.get());
      } else {
	if (hasShadow()) {
	  float b = shadowBlur.get(), bs = shadowBlur.get() * getDisplayScale();
	  float bi = int(ceil(b));
	  auto shadow = createSurface(getDefaultSurface().getLogicalWidth() + 2 * bi, getDefaultSurface().getLogicalHeight() + 2 * bi, R8);
	  
	  shadow->drawImage(img, Point(p.x + b + shadowOffsetX.get(), p.y + b + shadowOffsetY.get()), w, h, getDisplayScale(), globalAlpha.get(), 0.0f, 0.0f, 0.0f, shadowColor.get(), clipPath, imageSmoothingEnabled.get());
	  // shadow->colorFill(shadowColor.get());
	  auto shadow1 = shadow->blur(bs, bs);
	  auto shadow2 = shadow1->colorize(shadowColor.get());
	  getDefaultSurface().drawImage(*shadow2, Point(-b, -b), shadow->getLogicalWidth(), shadow->getLogicalHeight(), getDisplayScale(), 1.0f, 0.0f, 0.0f, 0.0f, shadowColor.get(), Path2D(), false);
	}
	getDefaultSurface().drawImage(img, p, w, h, getDisplayScale(), globalAlpha.get(), 0.0f, 0.0f, 0.0f, shadowColor.get(), clipPath, imageSmoothingEnabled.get());
      }
      return *this;
    }
        
    Style & createLinearGradient(double x0, double y0, double x1, double y1) {
      current_linear_gradient.setType(Style::LINEAR_GRADIENT);
      current_linear_gradient.setVector(x0, y0, x1, y1);
      return current_linear_gradient;
    }

    float getDisplayScale() const { return display_scale; }
    Context & addHitRegion(const std::string & id, const std::string & cursor) {
      if (!currentPath.empty()) {
	HitRegion hr(id, currentPath, cursor);
	hit_regions.push_back(hr);
      }
      return *this;
    }
#if 0
    const HitRegion & getHitRegion(float x, float y) const {
      for (auto & r : hit_regions) {
	if (r.isInside(x, y)) return r;
      }
      return null_region;
    }
#endif
    const std::vector<HitRegion> & getHitRegions() const { return hit_regions; }
    
#if 0
    Style & createPattern(const ImageData & image, const char * repeat) {
      
    }
    Style & createRadialGradient(double x0, double y0, double r0, double x1, double y1, double r1) {
     }
#endif
    
  protected:
    Context & renderPath(RenderMode mode, const Path2D & path, const Style & style, Operator op = SOURCE_OVER) {
      if (hasNativeShadows()) {
	getDefaultSurface().renderPath(mode, path, style, lineWidth.get(), op, getDisplayScale(), globalAlpha.get(), shadowBlur.get(), shadowOffsetX.get(), shadowOffsetY.get(), shadowColor.get(), clipPath);
      } else {
	if (hasShadow()) {
	  float b = shadowBlur.get(), bs = shadowBlur.get() * getDisplayScale();
	  float bi = int(ceil(b));
	  auto shadow = createSurface(getDefaultSurface().getLogicalWidth() + 2 * bi, getDefaultSurface().getLogicalHeight() + 2 * bi, R8);
	  Style shadow_style(this);
	  shadow_style = shadowColor.get();
	  Path2D tmp_path = path, tmp_clipPath = clipPath;
	  tmp_path.offset(shadowOffsetX.get() + bi, shadowOffsetY.get() + bi);
	  tmp_clipPath.offset(shadowOffsetX.get() + bi, shadowOffsetY.get() + bi);
	  
	  shadow->renderPath(mode, tmp_path, shadow_style, lineWidth.get(), op, getDisplayScale(), globalAlpha.get(), 0, 0, 0, shadowColor.get(), tmp_clipPath);
	  auto shadow1 = shadow->blur(bs, bs);
	  auto shadow2 = shadow1->colorize(shadowColor.get());
	  getDefaultSurface().drawImage(*shadow2, Point(-b, -b), shadow->getLogicalWidth(), shadow->getLogicalHeight(), getDisplayScale(), 1.0f, 0.0f, 0.0f, 0.0f, shadowColor.get(), Path2D(), false);
	}
	getDefaultSurface().renderPath(mode, path, style, lineWidth.get(), op, getDisplayScale(), globalAlpha.get(), 0, 0, 0, shadowColor.get(), clipPath);
      }
      return *this;
    }
    
    Context & renderText(RenderMode mode, const Style & style, const std::string & text, const Point & p, Operator op = SOURCE_OVER) {
      if (hasNativeShadows()) {
	getDefaultSurface().renderText(mode, font, style, textBaseline.get(), textAlign.get(), text, p, lineWidth.get(), op, getDisplayScale(), globalAlpha.get(), shadowBlur.get(), shadowOffsetX.get(), shadowOffsetY.get(), shadowColor.get(), clipPath);
      } else {
	if (hasShadow()) {
	  float b = shadowBlur.get(), bs = shadowBlur.get() * getDisplayScale();
	  int bi = int(ceil(b));
	  auto shadow = createSurface(getDefaultSurface().getLogicalWidth() + 2 * bi, getDefaultSurface().getLogicalHeight() + 2 * bi, R8);
	  
	  Style shadow_style(this);
	  shadow_style = shadowColor.get();
	  shadow_style.color.alpha = 1.0f;
	  shadow->renderText(mode, font, shadow_style, textBaseline.get(), textAlign.get(), text, Point(p.x + shadowOffsetX.get() + b, p.y + shadowOffsetY.get() + b), lineWidth.get(), op, getDisplayScale(), globalAlpha.get(), 0.0f, 0.0f, 0.0f, shadowColor.get(), clipPath);
	  auto shadow1 = shadow->blur(bs, bs);
	  auto shadow2 = shadow1->colorize(shadowColor.get());
	  getDefaultSurface().drawImage(*shadow2, Point(-b, -b), shadow->getLogicalWidth(), shadow->getLogicalHeight(), getDisplayScale(), 1.0f, 0.0f, 0.0f, 0.0f, shadowColor.get(), Path2D(), false);
	}
	getDefaultSurface().renderText(mode, font, style, textBaseline.get(), textAlign.get(), text, p, lineWidth.get(), op, getDisplayScale(), globalAlpha.get(), 0.0f, 0.0f, 0.0f, shadowColor.get(), clipPath);
      }
      return *this;
    }

    bool hasShadow() const { return shadowBlur.get() > 0.0f || shadowOffsetX.get() != 0 || shadowOffsetY.get() != 0; }
    
  private:
    float display_scale;
    Style current_linear_gradient;
    std::vector<GraphicsState> restore_stack;
    std::vector<HitRegion> hit_regions;
    HitRegion null_region;
  };
    
  class ContextFactory {
  public:
    ContextFactory(float _display_scale) : display_scale(_display_scale) { }
    virtual ~ContextFactory() { }
    virtual std::unique_ptr<Context> createContext(unsigned int width, unsigned int height, unsigned int num_channels = 4) = 0;
    virtual std::unique_ptr<Surface> createSurface(unsigned int width, unsigned int height, unsigned int num_channels = 4) = 0;
    virtual std::unique_ptr<Image> loadImage(const std::string & filename) = 0;
    virtual std::unique_ptr<Image> createImage() = 0;
    virtual std::unique_ptr<Image> createImage(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels) = 0;
    
    float getDisplayScale() const { return display_scale; }
    
  private:
    float display_scale;
  };

  class NullContext : public Context {
  public:
    NullContext() { }
  };

#if 0
  class NullContextFactory : public ContextFactory {
  public:
    NullContextFactory() : ContextFactory(1.0f) { }
    std::unique_ptr<Context> createContext(unsigned int width, unsigned int height, InternalFormat format = RGBA8) {
      return std::unique_ptr<Context>(new NullContext);
    }
    std::unique_ptr<Surface> createSurface(unsigned int width, unsigned int height, InternalFormat format = RGBA8) {
      
    }
    virtual std::unique_ptr<Image> loadImage(const std::string & filename) = 0;
    virtual std::unique_ptr<Image> createImage() = 0;
    virtual std::unique_ptr<Image> createImage(const unsigned char * _data, InternalFormat _format, unsigned int _width, unsigned int _height) = 0;
  };
#endif  
};

#endif
