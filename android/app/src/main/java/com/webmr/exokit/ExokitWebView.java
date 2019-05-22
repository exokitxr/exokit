package com.webmr.exokit;

import android.app.Activity;
import android.content.Context;
import android.widget.RelativeLayout;
import android.webkit.WebView;
import android.graphics.Canvas;
import android.view.Surface;

public class ExokitWebView extends WebView
{
    private Surface _webViewSurface;

    public ExokitWebView(Context context) {
      super(context);
    }

    public static ExokitWebView make(Activity activity, Context context) {
      ExokitWebView webView = new ExokitWebView(context);

      activity.addContentView(webView, new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT));

      return webView;
    }

    public void setWebViewSurface(Surface webViewSurface)
    {
        _webViewSurface = webViewSurface;
    }

    @Override
    public void draw(Canvas canvas) {
        if (_webViewSurface == null)
        {
            super.draw(canvas);
            return;
        }

        // Returns canvas attached to OpenGL texture to draw on
        Canvas glAttachedCanvas = _webViewSurface.lockCanvas(null);

        if (glAttachedCanvas != null)
        {
            // Draw the view to provided canvas
            super.draw(glAttachedCanvas);
        }
        else
        {
            super.draw(canvas);
            return;
        }

        _webViewSurface.unlockCanvasAndPost(glAttachedCanvas);
    }
}
