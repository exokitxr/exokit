package com.webmr.exokit;

import android.app.Activity;
import android.content.Context;
import android.widget.RelativeLayout;
import android.webkit.WebView;
import android.graphics.SurfaceTexture;
import android.graphics.Canvas;
import android.view.Surface;

public class ExokitWebView extends WebView
{
    private Surface _webViewSurface;

    public ExokitWebView(Context context, int colorTex) {
      super(context);

      SurfaceTexture surfaceTexture = new SurfaceTexture(colorTex);
      _webViewSurface = new Surface(surfaceTexture);
    }

    public static ExokitWebView make(Activity activity, Context context, int colorTex) {
      ExokitWebView webView = new ExokitWebView(context, colorTex);

      activity.addContentView(webView, new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT));

      return webView;
    }

    @Override
    public void draw(Canvas canvas) {
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
