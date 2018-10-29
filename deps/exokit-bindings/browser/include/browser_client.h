// Ŭnicode please
#include <include/cef_client.h>

class RenderHandler;

// for manual render handler
class BrowserClient : public CefClient
{
public:
	BrowserClient(RenderHandler *renderHandler);

	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() {
		return m_renderHandler;
	}

	CefRefPtr<CefRenderHandler> m_renderHandler;

	IMPLEMENT_REFCOUNTING(BrowserClient);
};
