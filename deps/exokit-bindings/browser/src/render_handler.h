// Ŭnicode please
#include <GL/glew.h>
#include <include/cef_render_handler.h>

class RenderHandler : public CefRenderHandler
{
public:
	RenderHandler();

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

public:
	GLuint tex() const { return tex_; }

private:
	int width_;
	int height_;

	GLuint tex_;
};
