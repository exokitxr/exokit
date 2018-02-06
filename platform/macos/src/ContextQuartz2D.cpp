#include <ContextQuartz2D.h>

#include <iostream>
#include <cassert>

#include <ImageIO/ImageIO.h>

using namespace canvas;
using namespace std;

Quartz2DSurface::Quartz2DSurface(const std::shared_ptr<Quartz2DCache> & _cache, const ImageData & image)
  : Surface(image.getWidth(), image.getHeight(), image.getWidth(), image.getHeight(), image.getNumChannels()), cache(_cache) {
  assert(getActualWidth() && getActualHeight());
  size_t bitmapByteCount = 4 * getActualWidth() * getActualHeight();
  bitmapData = new unsigned char[bitmapByteCount];
  if (image.getNumChannels() == 4) {
    memcpy(bitmapData, image.getData(), bitmapByteCount);
  } else {
    for (unsigned int i = 0; i < getActualWidth() * getActualHeight(); i++) {
      bitmapData[4 * i + 0] = image.getData()[3 * i + 2];
      bitmapData[4 * i + 1] = image.getData()[3 * i + 1];
      bitmapData[4 * i + 2] = image.getData()[3 * i + 0];
      bitmapData[4 * i + 3] = 255;
    }
  }
}

Quartz2DSurface::Quartz2DSurface(const std::shared_ptr<Quartz2DCache> & _cache, const std::string & filename)
  : Surface(0, 0, 0, 0, RGBA8), cache(_cache) {
  CGDataProviderRef provider = CGDataProviderCreateWithFilename(filename.c_str());
  CGImageRef img;
  if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".png") == 0) {
    img = CGImageCreateWithPNGDataProvider(provider, 0, false, kCGRenderingIntentDefault);
  } else if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".jpg") == 0) {
    img = CGImageCreateWithJPEGDataProvider(provider, 0, false, kCGRenderingIntentDefault);
  } else {
    cerr << "could not open file " << filename << endl;
    assert(0);
    img = 0;
  }
  if (img) {
    bool has_alpha = CGImageGetAlphaInfo(img) != kCGImageAlphaNone;
    Surface::resize(CGImageGetWidth(img), CGImageGetHeight(img), CGImageGetWidth(img), CGImageGetHeight(img), has_alpha ? RGBA8 : RGB8);
    unsigned int bitmapByteCount = 4 * getActualWidth() * getActualHeight();
    bitmapData = new unsigned char[bitmapByteCount];
    memset(bitmapData, 0, bitmapByteCount);
  
    initializeContext();
    flipY();
    CGContextDrawImage(gc, CGRectMake(0, 0, getActualWidth(), getActualHeight()), img);
#ifdef MEMDEBUG
    if (CFGetRetainCount(img) != 1) cerr << "leaking memory 1!\n";
#endif
    CGImageRelease(img);
    flipY();
  } else {
    Surface::resize(16, 16, 16, 16, RGBA8);
    unsigned int bitmapByteCount = 4 * getActualWidth() * getActualHeight();
    bitmapData = new unsigned char[bitmapByteCount];
    memset(bitmapData, 0, bitmapByteCount);
  }
#ifdef MEMDEBUG
  if (CFGetRetainCount(provider) != 1) cerr << "leaking memory 2!\n";
#endif
  CGDataProviderRelease(provider);
}

void
Quartz2DSurface::sendPath(const Path2D & path, float scale) {
  initializeContext();
  CGContextBeginPath(gc);
  for (auto pc : path.getData()) {
    switch (pc.type) {
    case PathComponent::MOVE_TO: CGContextMoveToPoint(gc, pc.x0 * scale + 0.5, pc.y0 * scale + 0.5); break;
    case PathComponent::LINE_TO: CGContextAddLineToPoint(gc, pc.x0 * scale + 0.5, pc.y0 * scale + 0.5); break;
    case PathComponent::ARC: CGContextAddArc(gc, pc.x0 * scale + 0.5, pc.y0 * scale + 0.5, pc.radius * scale, pc.sa, pc.ea, pc.anticlockwise); break;
    case PathComponent::CLOSE: CGContextClosePath(gc); break;
    }
  }
}

void
Quartz2DSurface::renderPath(RenderMode mode, const Path2D & path, const Style & style, float lineWidth, Operator op, float display_scale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) {
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
  
  switch (op) {
  case SOURCE_OVER:
    CGContextSetBlendMode(gc, kCGBlendModeNormal);
    break;
  case COPY:
    CGContextSetBlendMode(gc, kCGBlendModeCopy);
    break;
  }
  switch (mode) {
  case STROKE:
    sendPath(path, display_scale);
    CGContextSetRGBStrokeColor(gc, style.color.red,
			       style.color.green,
			       style.color.blue,
			       style.color.alpha * globalAlpha);
    CGContextSetLineWidth(gc, lineWidth * display_scale);
    CGContextStrokePath(gc);  
    break;
  case FILL:
    if (style.getType() == Style::LINEAR_GRADIENT) {
      const std::map<float, Color> & colors = style.getColors();
      if (!colors.empty()) {
	std::map<float, Color>::const_iterator it0 = colors.begin(), it1 = colors.end();
	it1--;
	const Color & c0 = it0->second, c1 = it1->second;
	
	size_t num_locations = 2;
	CGFloat locations[2] = { 0.0, 1.0 };
	CGFloat components[8] = {
	  c0.red, c0.green, c0.blue, c0.alpha * globalAlpha,
	  c1.red, c1.green, c1.blue, c1.alpha * globalAlpha
	};
	
	CGGradientRef myGradient = CGGradientCreateWithColorComponents(cache->getColorSpace(), components, locations, num_locations);
	
        CGContextSaveGState(gc);
        sendPath(path, display_scale);
        CGContextClip(gc);
	
	CGPoint myStartPoint, myEndPoint;
	myStartPoint.x = style.x0 * display_scale;
	myStartPoint.y = style.y0 * display_scale;
	myEndPoint.x = style.x1 * display_scale;
	myEndPoint.y = style.y1 * display_scale;
	CGContextDrawLinearGradient(gc, myGradient, myStartPoint, myEndPoint, 0);
	
        CGContextRestoreGState(gc);

#ifdef MEMDEBUG
	if (CFGetRetainCount(myGradient) != 1) cerr << "leaking memory 7!\n";
#endif
	CGGradientRelease(myGradient);
      }
    } else {
      sendPath(path, display_scale);
      CGContextSetRGBFillColor(gc, style.color.red,
			       style.color.green,
			       style.color.blue,
			       style.color.alpha * globalAlpha);
      CGContextFillPath(gc);
    }
  }
  if (op != SOURCE_OVER) {
    CGContextSetBlendMode(gc, kCGBlendModeNormal);
  }
  if (has_shadow || !clipPath.empty()) {
    CGContextRestoreGState(gc);
  }
}

void
Quartz2DSurface::resize(unsigned int _logical_width, unsigned int _logical_height, unsigned int _actual_width, unsigned int _actual_height, unsigned int _num_channels) {
  Surface::resize(_logical_width, _logical_height, _actual_width, _actual_height, _num_channels);
  
  if (gc) {
#ifdef MEMDEBUG
    if (CFGetRetainCount(gc) != 1) std::cerr << "leaking memory E!\n";
#endif
    CGContextRelease(gc);
    gc = 0;
  }
  delete[] bitmapData;
  
  assert(getActualWidth() && getActualHeight());
  unsigned int bitmapByteCount = 4 * getActualWidth() * getActualHeight();
  bitmapData = new unsigned char[bitmapByteCount];
  memset(bitmapData, 0, bitmapByteCount);
}

void
Quartz2DSurface::drawImage(const ImageData & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled) {
  initializeContext();
  bool has_shadow = shadowBlur > 0.0f || shadowOffsetX != 0.0f || shadowOffsetY != 0.0f;
  if (has_shadow || !clipPath.empty()) {
    CGContextSaveGState(gc);
  }
  if (!clipPath.empty()) {
    sendPath(clipPath, displayScale);
    CGContextClip(gc);
  }
  if (has_shadow) {
    setShadow(shadowOffsetX, shadowOffsetY, shadowBlur, shadowColor, displayScale);
  }
  unsigned int num_channels = _img.getNumChannels();
  auto has_alpha = num_channels == 4;
  CGDataProviderRef provider = CGDataProviderCreateWithData(0, _img.getData(), num_channels * _img.getWidth() * _img.getHeight(), 0);
  auto f = (has_alpha ? kCGImageAlphaPremultipliedLast : kCGImageAlphaNoneSkipLast);
  CGImageRef img = CGImageCreate(_img.getWidth(), _img.getHeight(), 8, num_channels * 8, num_channels * _img.getWidth(), cache->getColorSpace(), f, provider, 0, imageSmoothingEnabled, kCGRenderingIntentDefault);
  if (img) {
    flipY();
    if (globalAlpha < 1.0f) CGContextSetAlpha(gc, globalAlpha);
    CGContextDrawImage(gc, CGRectMake(displayScale * p.x, getActualHeight() - 1 - displayScale * (p.y + h), displayScale * w, displayScale * h), img);
    if (globalAlpha < 1.0f) CGContextSetAlpha(gc, 1.0f);
    flipY();
  
#ifdef MEMDEBUG
    if (CFGetRetainCount(img) != 1) std::cerr << "leaking memory O!\n";
#endif
    CGImageRelease(img);
#ifdef MEMDEBUG
    if (CFGetRetainCount(provider) != 1) std::cerr << "leaking memory P!\n";
#endif
  }
  CGDataProviderRelease(provider);
  if (has_shadow || !clipPath.empty()) {
    CGContextRestoreGState(gc);
  }
}

class Quartz2DImage : public Image {
public:
  Quartz2DImage(float _display_scale)
    : Image(_display_scale) { }

  Quartz2DImage(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels, float _display_scale) : Image(_data, _width, _height, _num_channels, _display_scale) { }
};

std::unique_ptr<Image>
Quartz2DContextFactory::createImage() {
  return std::unique_ptr<Image>(new Quartz2DImage(getDisplayScale()));
}

std::unique_ptr<Image>
Quartz2DContextFactory::createImage(const unsigned char * _data, unsigned int _width, unsigned int _height, unsigned int _num_channels) {
  return std::unique_ptr<Image>(new Quartz2DImage(_data, _width, _height, _num_channels, getDisplayScale()));
}

std::unique_ptr<Image>
Quartz2DSurface::createImage(float display_scale) {
  unsigned char * buffer = (unsigned char *)lockMemory(false);
  assert(buffer);

  auto image = std::unique_ptr<Image>(new Quartz2DImage(buffer, getActualWidth(), getActualHeight(), getNumChannels(), display_scale));
  releaseMemory();
  
  return image;
}
