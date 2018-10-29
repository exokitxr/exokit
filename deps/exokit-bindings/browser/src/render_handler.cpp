#include "render_handler.h"

#include <iostream>

RenderHandler::RenderHandler(OnPaintFn onPaint) : onPaint(onPaint), width_(2), height_(2)/*, tex_(0)*/ {}

void RenderHandler::init()
{
  std::cout << "RenderHandler::init" << std::endl;
	/* glGenTextures(1, &tex_);
	glBindTexture(GL_TEXTURE_2D, tex_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// dummy texture data - for debugging
	const unsigned char data[] = {
		255, 0, 0, 255,
		0, 255, 0, 255,
		0, 0, 255, 255,
		255, 255, 255, 255,
	};
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, 0); */
}
void RenderHandler::resize(int w, int h)
{
  // std::cout << "RenderHandler::resize" << std::endl;
	width_ = w;
	height_ = h;
}

bool RenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
  // std::cout << "RenderHandler::GetViewRect" << std::endl;
	rect = CefRect(0, 0, width_, height_);
	return true;
}

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
{
  // std::cout << "RenderHandler::OnPaint" << std::endl;
  onPaint(dirtyRects, buffer, width, height);
  
	/* glBindTexture(GL_TEXTURE_2D, tex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (unsigned char*)buffer);
	glBindTexture(GL_TEXTURE_2D, 0); */
  
  /* glPixelStorei(GL_UNPACK_ROW_LENGTH, view_width_);

  if (old_width != view_width_ || old_height != view_height_) {
    // Update/resize the whole texture.
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, view_width_, view_height_, 0,
                 GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, buffer);
  } else {
    // Update just the dirty rectangles.
    CefRenderHandler::RectList::const_iterator i = dirtyRects.begin();
    for (; i != dirtyRects.end(); ++i) {
      const CefRect& rect = *i;
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, rect.x);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, rect.y);
      glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.width,
                      rect.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                      buffer);
    }
  } */
}
