// Ŭnicode please
#include <include/cef_client.h>
#include <string>
#include <vector>

class RenderHandler;
class BrowserClient;

class WebCore {
public:
	WebCore(const std::string &url);
	~WebCore();

	void reshape(int w, int h);

	void mouseMove(int x, int y);
	void mouseClick(CefBrowserHost::MouseButtonType btn, bool mouse_up);
	void keyPress(int key, bool pressed);

	RenderHandler* render_handler() const { return render_handler_; }

private:
	int mouse_x_;
	int mouse_y_;

	CefRefPtr<CefBrowser> browser_;
	CefRefPtr<BrowserClient> client_;

	RenderHandler* render_handler_;
};

class WebCoreManager {
public:
	WebCoreManager();
	~WebCoreManager();

	bool setUp(int *exit_code);
	bool shutDown();

	void update();

	std::weak_ptr<WebCore> createBrowser(const std::string &url);
	void removeBrowser(std::weak_ptr<WebCore> web_core);

private:
	std::vector<std::shared_ptr<WebCore>> browsers_;
};
