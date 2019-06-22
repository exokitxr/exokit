package com.webmr.exokit;

import android.app.Activity;
import android.content.Context;
import android.widget.RelativeLayout;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.WebChromeClient;
import android.graphics.SurfaceTexture;
import android.graphics.Canvas;
import android.graphics.Bitmap;
import android.view.Surface;
import android.view.ViewGroup;
import android.os.Looper;
import android.os.Handler;

public class ExokitWebView extends WebView
{
    private Surface _webViewSurface = null;
    private SurfaceTexture _webViewSurfaceTexture = null;

    public ExokitWebView(Context context, int width, int height, int colorTex, Surface webviewSurface, SurfaceTexture webViewSurfaceTexture, String url) {
      super(context);

      _webViewSurface = webviewSurface;
      _webViewSurfaceTexture = webViewSurfaceTexture;

      try {
        ExokitWebView webView = this;

        setWebViewClient(new WebViewClient(){
          @Override
          public boolean shouldOverrideUrlLoading(WebView view, String url) {
            view.loadUrl(url);
            return true;
          }

          public void onPageStarted(WebView view, String url, Bitmap favicon) {
            // System.out.println("webview onpagestarted 1");
          }

          public void onPageFinished(WebView view, String url) {
            // System.out.printf("webview onpagefinished 1 %s\n", String.valueOf(view.isEnabled()));
          }
        });
        setWebChromeClient(new WebChromeClient() { });
        /* setLayoutParams( new ViewGroup.LayoutParams( width, height ) );
        forceLayout(); */

        // layout(0, 0, width, height);
        // setWillNotDraw(false);
        getSettings().setJavaScriptEnabled(true);
        loadUrl(url);
        // setLayoutParams(new ViewGroup.LayoutParams(width, height));
        /* ExokitWebView webView = this;
        post(new Runnable() {
            public void run() {
                webView.loadUrl(url);
            }
        }); */
      } catch (Exception err) {
        err.printStackTrace();
      }
    }

    public static ExokitWebView make(Activity activity, Context context, int width, int height, int colorTex, String url) {
      SurfaceTexture webViewSurfaceTexture = new SurfaceTexture(colorTex);
      webViewSurfaceTexture.setDefaultBufferSize(width, height);
      Surface webViewSurface = new Surface(webViewSurfaceTexture);

      class Runnable2 implements Runnable {
        public volatile ExokitWebView result = null;
        public void run() {
          try {
            ExokitWebView webView = new ExokitWebView(context, width, height, colorTex, webViewSurface, webViewSurfaceTexture, url);

            activity.addContentView(webView, new RelativeLayout.LayoutParams(width, height));

            result = webView;

            synchronized(this) {
              this.notify();
            }
          } catch (Exception err) {
            err.printStackTrace();
          }
        }
      }

      Runnable2 runnable = new Runnable2();
      synchronized(runnable) {
        try {
          activity.runOnUiThread(runnable);
          runnable.wait();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
      }
      webViewSurfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
        public void onFrameAvailable(SurfaceTexture surfaceTexture) {
          // System.out.println("webview frame available 1");
        }
      });
      return runnable.result;
      // return null;
    }

    @Override
    public void draw(Canvas canvas) {
      try {
        // Returns canvas attached to OpenGL texture to draw on
        Canvas glAttachedCanvas = _webViewSurface.lockCanvas(null);

        if (glAttachedCanvas != null) {
            // Draw the view to provided canvas
            super.draw(glAttachedCanvas);

            _webViewSurface.unlockCanvasAndPost(glAttachedCanvas);
        } else {
            /* System.out.println("webview draw 1.3");
            super.draw(canvas); */
        }
      } catch (Exception e) {
          e.printStackTrace();
      }
    }

    public void draw() {
      try {
        draw(null);

        _webViewSurfaceTexture.updateTexImage();
      } catch (Exception err) {
        err.printStackTrace();
      }
    }

}
