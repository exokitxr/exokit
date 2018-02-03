#include <ContextGDIPlus.h>

#include <utf8.h>
#include <cassert>

#define M_PI 3.14159265358979323846

bool canvas::ContextGDIPlus::is_initialized = false;
ULONG_PTR canvas::ContextGDIPlus::m_gdiplusToken;

using namespace std;
using namespace canvas;

static std::wstring convert_to_wstring(const std::string & input) {
  const char * str = input.c_str();
  const char * str_i = str;
  const char * end = str + input.size();
  std::wstring output;
  while (str_i < end) {
    output += (wchar_t)utf8::next(str_i, end);
  }
  return output;
}

static void toGDIPath(const Path2D & path, Gdiplus::GraphicsPath & output, float display_scale) {  
  output.StartFigure();
  Gdiplus::PointF current_pos;

  for (auto pc : path.getData()) {
    switch (pc.type) {
    case PathComponent::MOVE_TO:
      current_pos = Gdiplus::PointF(Gdiplus::REAL(pc.x0 * display_scale), Gdiplus::REAL(pc.y0 * display_scale));
      break;
    case PathComponent::LINE_TO:
      {
	Gdiplus::PointF point(Gdiplus::REAL(pc.x0 * display_scale), Gdiplus::REAL(pc.y0 * display_scale));
	output.AddLine(current_pos, point);
	current_pos = point;
      }
      break;
    case PathComponent::CLOSE:
      output.CloseFigure();
      break;
    case PathComponent::ARC:
      {
	double span = 0;
	if (0 && ((!pc.anticlockwise && (pc.ea - pc.sa >= 2 * M_PI)) || (pc.anticlockwise && (pc.sa - pc.ea >= 2 * M_PI)))) {
	  // If the anticlockwise argument is false and endAngle-startAngle is equal to or greater than 2*PI, or, if the
	  // anticlockwise argument is true and startAngle-endAngle is equal to or greater than 2*PI, then the arc is the whole
	  // circumference of this circle.
	  span = 2 * M_PI;
	} else {
	  if (!pc.anticlockwise && (pc.ea < pc.sa)) {
	    span += 2 * M_PI;
	  } else if (pc.anticlockwise && (pc.sa < pc.ea)) {
	    span -= 2 * M_PI;
	  }
 
#if 0
    // this is also due to switched coordinate system
    // we would end up with a 0 span instead of 360
    if (!(qFuzzyCompare(span + (ea - sa) + 1, 1.0) && qFuzzyCompare(qAbs(span), 360.0))) {
      // mod 360
      span += (ea - sa) - (static_cast<int>((ea - sa) / 360)) * 360;
    }
#else
	  span += pc.ea - pc.sa;
#endif
	}
 
#if 0
  // If the path is empty, move to where the arc will start to avoid painting a line from (0,0)
  // NOTE: QPainterPath::isEmpty() won't work here since it ignores a lone MoveToElement
  if (!m_path.elementCount())
    m_path.arcMoveTo(xs, ys, width, height, sa);
  else if (!radius) {
    m_path.lineTo(xc, yc);
    return;
  }
#endif

#if 0
  if (anticlockwise) {
    span = -M_PI / 2.0;
  } else {
    span = M_PI / 2.0;
  }
#endif
  Gdiplus::RectF rect(Gdiplus::REAL(pc.x0 * display_scale - pc.radius * display_scale), Gdiplus::REAL(pc.y0 * display_scale - pc.radius * display_scale), Gdiplus::REAL(2 * pc.radius * display_scale), Gdiplus::REAL(2 * pc.radius * display_scale));

	output.AddArc(rect, Gdiplus::REAL(pc.sa * 180.0f / M_PI), Gdiplus::REAL(span * 180.0f / M_PI));
	output.GetLastPoint(&current_pos);
      }
      break;
    }
  }
}

static Gdiplus::Color toGDIColor(const Color & input, float globalAlpha = 1.0f) {
  int red = int(input.red * 255), green = int(input.green * 255), blue = int(input.blue * 255), alpha = int(input.alpha * globalAlpha * 255);
  if (red < 0) red = 0;
  else if (red > 255) red = 255;
  if (green < 0) green = 0;
  else if (green > 255) green = 255;
  if (blue < 0) blue = 0;
  else if (blue > 255) blue = 255;
  if (alpha < 0) alpha = 0;
  else if (alpha > 255) alpha = 255;
#if 0
  return Gdiplus::Color::FromArgb(alpha, red, green, blue);
#else
  return Gdiplus::Color(alpha, red, green, blue);
#endif
}

GDIPlusSurface::GDIPlusSurface(const std::string & filename) : Surface(0, 0, 0, 0, false) {
  std::wstring tmp = convert_to_wstring(filename);
  bitmap = std::unique_ptr<Gdiplus::Bitmap>(Gdiplus::Bitmap::FromFile(tmp.data()));
  Surface::resize(bitmap->GetWidth(), bitmap->GetHeight(), bitmap->GetWidth(), bitmap->GetHeight(), true);
}

GDIPlusSurface::GDIPlusSurface(const unsigned char * buffer, size_t size) : Surface(0, 0, 0, 0, false) {
	assert(0);
}


void
GDIPlusSurface::renderPath(RenderMode mode, const Path2D & input_path, const Style & style, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) {
  initializeContext();
  
  switch (op) {
  case SOURCE_OVER:
    g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    break;
  case COPY:
    g->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
    break;    
  }

  Gdiplus::GraphicsPath path;
  toGDIPath(input_path, path, displayScale);

  switch (mode) {
  case STROKE:
    {
      Gdiplus::Pen pen(toGDIColor(style.color, globalAlpha), lineWidth);
      g->DrawPath(&pen, &path);
    }
    break;
  case FILL:
    if (style.getType() == Style::LINEAR_GRADIENT) {
      const std::map<float, Color> & colors = style.getColors();
      if (!colors.empty()) {
	std::map<float, Color>::const_iterator it0 = colors.begin(), it1 = colors.end();
	it1--;
	const Color & c0 = it0->second, c1 = it1->second;
	Gdiplus::LinearGradientBrush brush(Gdiplus::PointF(Gdiplus::REAL(style.x0), Gdiplus::REAL(style.y0)),
					   Gdiplus::PointF(Gdiplus::REAL(style.x1), Gdiplus::REAL(style.y1)),
					   toGDIColor(c0, globalAlpha),
					   toGDIColor(c1, globalAlpha));
	g->FillPath(&brush, &path);
      }
    } else {
      Gdiplus::SolidBrush brush(toGDIColor(style.color, globalAlpha));
      g->FillPath(&brush, &path);
    }
  }
}

void
GDIPlusSurface::clip(const Path2D & input_path, float display_scale) {
  initializeContext();
  
  Gdiplus::GraphicsPath path;
  toGDIPath(input_path, path, display_scale);

  Gdiplus::Region region(&path);
  g->SetClip(&region);
}

void
GDIPlusSurface::drawNativeSurface(GDIPlusSurface & img, const Point & p, double w, double h, float displayScale, float globalAlpha, bool imageSmoothingEnabled) {
  initializeContext();

  g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
  
  if (imageSmoothingEnabled) {
    // g->SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBicubic );
    g->SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBilinear );
  } else {
    g->SetInterpolationMode( Gdiplus::InterpolationModeNearestNeighbor );
  }
  if (globalAlpha < 1.0f && 0) {
#if 0
    ImageAttributes  imageAttributes;
    ColorMatrix colorMatrix = {
      1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, alpha, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
   
    imageAttributes.SetColorMatrix( &colorMatrix, 
				    ColorMatrixFlagsDefault,
				    ColorAdjustTypeBitmap);
    graphics.DrawImage( &(*(img.bitmap)),
			Gdiplus::Rect(p.x, p.y, w, h), // destination rectangle 
			0, 0,        // upper-left corner of source rectangle 
			getWidth(),       // width of source rectangle
			getHeight(),      // height of source rectangle
			Gdiplus::UnitPixel,
			&imageAttributes);
#endif
  } else if (img.getActualWidth() == (unsigned int)w && img.getActualHeight() == (unsigned int)h && 0) { // this scales image weirdly
    g->DrawImage(&(*(img.bitmap)), Gdiplus::REAL(p.x), Gdiplus::REAL(p.y));
  } else {
    g->DrawImage(&(*(img.bitmap)), Gdiplus::REAL(p.x), Gdiplus::REAL(p.y), Gdiplus::REAL(w), Gdiplus::REAL(h));
  }
}

void
GDIPlusSurface::drawImage(Surface & _img, const Point & p, double w, double h, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath, bool imageSmoothingEnabled) {
  GDIPlusSurface * img = dynamic_cast<GDIPlusSurface*>(&_img);
  if (img) {
    drawNativeSurface(*img, p, w, h, displayScale, globalAlpha, imageSmoothingEnabled);
  } else {
    auto img = _img.createImage(displayScale);
    GDIPlusSurface cs(img->getData());
    drawNativeSurface(cs, p, w, h, displayScale, globalAlpha, imageSmoothingEnabled);
  }
}

void
GDIPlusSurface::renderText(RenderMode mode, const Font & font, const Style & style, TextBaseline textBaseline, TextAlign textAlign, const std::string & text, const Point & p, float lineWidth, Operator op, float displayScale, float globalAlpha, float shadowBlur, float shadowOffsetX, float shadowOffsetY, const Color & shadowColor, const Path2D & clipPath) {
  initializeContext();

  double x = round(p.x);
  double y = round(p.y);
  
  switch (op) {
  case SOURCE_OVER:
    g->SetCompositingMode(Gdiplus::CompositingModeSourceOver);
    break;
  case COPY:
    g->SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
    break;    
  }

  if (font.cleartype) {
    g->SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);
  } else if (font.antialiasing && font.hinting && 0) {
    g->SetTextRenderingHint( Gdiplus::TextRenderingHintAntiAliasGridFit );
  } else if (font.antialiasing) {
    g->SetTextRenderingHint( Gdiplus::TextRenderingHintAntiAlias );
  } else if (font.hinting) {
    g->SetTextRenderingHint( Gdiplus::TextRenderingHintSingleBitPerPixelGridFit );
  } else {
    g->SetTextRenderingHint( Gdiplus::TextRenderingHintSingleBitPerPixel );
  }
    
  std::wstring text2 = convert_to_wstring(text);
  int style_bits = 0;
  if (font.weight.isBold()) {
    style_bits |= Gdiplus::FontStyleBold;
  }
  if (font.style == Font::Style::ITALIC) {
    style_bits |= Gdiplus::FontStyleItalic;
  }
  Gdiplus::Font gdifont(&Gdiplus::FontFamily(L"Arial"), font.size * displayScale, style_bits, Gdiplus::UnitPixel);

  Gdiplus::RectF rect(Gdiplus::REAL(x * displayScale), Gdiplus::REAL(y * displayScale), 0.0f, 0.0f);
  Gdiplus::StringFormat f;

  switch (textBaseline) {
  case TextBaseline::TOP: break;
  case TextBaseline::HANGING: break;
  case TextBaseline::MIDDLE: f.SetLineAlignment(Gdiplus::StringAlignmentCenter); break;
  case TextBaseline::BOTTOM: f.SetLineAlignment(Gdiplus::StringAlignmentFar);
  }

  switch (textAlign) {
  case TextAlign::ALIGN_CENTER: f.SetAlignment(Gdiplus::StringAlignmentCenter); break;
  case TextAlign::ALIGN_END: case TextAlign::ALIGN_RIGHT: f.SetAlignment(Gdiplus::StringAlignmentFar); break;
  case TextAlign::ALIGN_START: case TextAlign::ALIGN_LEFT: f.SetAlignment(Gdiplus::StringAlignmentNear); break;
  }

  f.SetFormatFlags(Gdiplus::StringFormatFlagsBypassGDI);

  switch (mode) {
  case STROKE:
    // implement
    break;
  case FILL:
    {
      Gdiplus::SolidBrush brush(toGDIColor(style.color, globalAlpha));
      g->DrawString(text2.data(), text2.size(), &gdifont, rect, &f, &brush);
    }
    break;
  }
}

TextMetrics
GDIPlusSurface::measureText(const Font & font, const std::string & text, TextBaseline textBaseline, float displayScale) {
  initializeContext();
  std::wstring text2 = convert_to_wstring(text);
  int style = 0;
  if (font.weight.isBold()) {
    style |= Gdiplus::FontStyleBold;
  }
  if (font.style == Font::Style::ITALIC) {
    style |= Gdiplus::FontStyleItalic;
  }

  Gdiplus::FontFamily fontFamily(L"Arial");

  Gdiplus::Font gdi_font(&fontFamily, font.size * displayScale, style, Gdiplus::UnitPixel);
  Gdiplus::RectF layoutRect(0, 0, 512, 512), boundingBox;
  g->MeasureString(text2.data(), text2.size(), &gdi_font, layoutRect, &boundingBox);
  Gdiplus::SizeF size;
  boundingBox.GetSize(&size);

  float ascent = font.size * fontFamily.GetCellAscent(style) / fontFamily.GetEmHeight(style);
  float descent = font.size * fontFamily.GetCellDescent(style) / fontFamily.GetEmHeight(style);
  float baseline = 0;
  if (textBaseline == TextBaseline::MIDDLE) {
    baseline = (ascent + descent) / 2;
  } else if (textBaseline == TextBaseline::TOP) {
    baseline = (ascent + descent);
  }
  
  return TextMetrics(size.Width / displayScale, (descent - baseline) / displayScale, (ascent - baseline) / displayScale);
}

std::unique_ptr<Image>
GDIPlusSurface::createImage(float display_scale) {
  std::unique_ptr<Image> image;
  unsigned char * buffer = (unsigned char *)lockMemory(false);
  assert(buffer);
  if (buffer) {
    image = std::unique_ptr<Image>(new Image(buffer, getActualWidth(), getActualHeight(), getNumChannels(), display_scale));
    releaseMemory();
  }
  return image;
}