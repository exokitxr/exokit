#ifndef _RENDER_HANDLER_H_
#define _RENDER_HANDLER_H_

// #include <GL/glew.h>
#include <include/cef_render_handler.h>

#include <functional>

class RenderHandler : public CefRenderHandler
{
public:
  typedef std::function<void(const RectList &, const void *, int, int)> OnPaintFn;
  
public:
	RenderHandler(OnPaintFn onPaint);

public:
	void init();
	void resize(int w, int h);

	// CefRenderHandler interface
public:
	bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect);
	void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height);

	// CefBase interface
public:
	IMPLEMENT_REFCOUNTING(RenderHandler);

/* public:
	GLuint tex() const { return tex_; } */

private:
  std::function<void(const RectList &, const void *, int, int)> onPaint;

	int width_;
	int height_;

	// GLuint tex_;
};

#endif