package com.webmr.exokit;

import java.text.MessageFormat;
import android.app.Activity;
import android.content.Context;
// import android.widget.RelativeLayout;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.WebChromeClient;
import android.webkit.ConsoleMessage;
import android.webkit.ValueCallback;
import android.graphics.SurfaceTexture;
import android.graphics.Canvas;
import android.graphics.Bitmap;
import android.view.Surface;
import android.view.ViewGroup;
import android.os.Looper;
import android.os.Handler;
import android.os.SystemClock;
import android.view.KeyEvent;
import android.view.InputDevice;
import android.view.MotionEvent;

public class ExokitWebView extends WebView
{
    private Activity _activity = null;
    private Surface _webViewSurface = null;
    private SurfaceTexture _webViewSurfaceTexture = null;

    public ExokitWebView(Activity activity, Context context, int width, int height, int colorTex, Surface webviewSurface, SurfaceTexture webViewSurfaceTexture, String url) {
      super(context);

      _activity = activity;
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
        setWebChromeClient(new WebChromeClient() {
          public boolean onConsoleMessage(ConsoleMessage consoleMessage) {
            System.out.println(consoleMessage.message());
            return true;
          }
        });
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
            ExokitWebView webView = new ExokitWebView(activity, context, width, height, colorTex, webViewSurface, webViewSurfaceTexture, url);

            activity.addContentView(webView, new ViewGroup.LayoutParams(width, height));

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

    public void runJs(String jsString) {
      _activity.runOnUiThread(new Runnable() {
        public void run() {
          evaluateJavascript(jsString, new ValueCallback<String>() {
            public void onReceiveValue(String value) {
              // nothing
            }
          });
        }
      });
    }

    public void keyDown(int keyCode) {
      ExokitWebView webView = this;
      _activity.runOnUiThread(new Runnable() {
        public void run() {
          webView.requestFocus();
          KeyEvent keyEvent = new KeyEvent(KeyEvent.ACTION_DOWN, keyCode);
          webView.dispatchKeyEvent(keyEvent);
        }
      });
    }

    public void keyUp(int keyCode) {
      ExokitWebView webView = this;
      _activity.runOnUiThread(new Runnable() {
        public void run() {
          webView.requestFocus();
          KeyEvent keyEvent = new KeyEvent(KeyEvent.ACTION_UP, keyCode);
          webView.dispatchKeyEvent(keyEvent);
        }
      });
    }

    public void keyPress(int keyCode) {
      // nothing
    }

    public void mouseDown(int x, int y, int button) {
      ExokitWebView webView = this;
      _activity.runOnUiThread(new Runnable() {
        public void run() {
          webView.requestFocus();

          long downTime = SystemClock.uptimeMillis();
          long eventTime = downTime; // SystemClock.uptimeMillis() + 100;
          int metaState = 0;
          MotionEvent motionEvent = MotionEvent.obtain(
            downTime,
            eventTime,
            MotionEvent.ACTION_DOWN,
            x,
            y,
            metaState
          );
          webView.dispatchTouchEvent(motionEvent);
        }
      });
    }

    public void mouseUp(int x, int y, int button) {
      ExokitWebView webView = this;
      _activity.runOnUiThread(new Runnable() {
        public void run() {
          webView.requestFocus();

          long downTime = SystemClock.uptimeMillis();
          long eventTime = downTime; // SystemClock.uptimeMillis() + 100;
          int metaState = 0;
          MotionEvent motionEvent = MotionEvent.obtain(
            downTime,
            eventTime,
            MotionEvent.ACTION_UP,
            x,
            y,
            metaState
          );
          webView.dispatchTouchEvent(motionEvent);
        }
      });
    }

    public void click(int x, int y, int button) {
      // nothing

      /* ExokitWebView webView = this;
      _activity.runOnUiThread(new Runnable() {
        public void run() {
          long downTime = SystemClock.uptimeMillis();
          long eventTime = downTime; // SystemClock.uptimeMillis() + 100;
          int metaState = 0;
          MotionEvent motionEvent = MotionEvent.obtain(
            downTime,
            eventTime,
            MotionEvent.ACTION_CLICK,
            x,
            y,
            metaState
          );
          webView.dispatchTouchEvent(motionEvent);
        }
      }); */
    }

    public void mouseMove(int x, int y) {
      /* ExokitWebView webView = this;
      _activity.runOnUiThread(new Runnable() {
        public void run() {
          webView.requestFocus();

          long downTime = SystemClock.uptimeMillis();
          long eventTime = downTime; // SystemClock.uptimeMillis() + 100;
          int metaState = 0;
          MotionEvent motionEvent = MotionEvent.obtain(
            downTime,
            eventTime,
            MotionEvent.ACTION_HOVER_MOVE,
            x,
            y,
            metaState
          );
          webView.dispatchTouchEvent(motionEvent);
        }
      }); */
    }

    public void mouseWheel(int x, int y, int deltaX, int deltaY) {
      ExokitWebView webView = this;
      _activity.runOnUiThread(new Runnable() {
        public void run() {
          webView.requestFocus();

          webView.scrollBy(deltaX, deltaY);

          /* long downTime = SystemClock.uptimeMillis();
          long eventTime = downTime; // SystemClock.uptimeMillis() + 100;
          int metaState = 0;
          int buttonState = 0;
          float xPrecision = 1.0f;
          float yPrecision = 1.0f;
          int deviceId = 0;
          int edgeFlags = 0;
          int source = InputDevice.SOURCE_CLASS_POINTER;
          int flags = 0;

          int pointerCount = 1;

          MotionEvent.PointerProperties properties = new MotionEvent.PointerProperties();
          properties.id = 0;
          MotionEvent.PointerProperties[] prop = { properties };

          MotionEvent.PointerCoords coord = new MotionEvent.PointerCoords();
          coord.x = 0;
          coord.y = 0;
          coord.setAxisValue(MotionEvent.AXIS_HSCROLL, deltaX);
          coord.setAxisValue(MotionEvent.AXIS_VSCROLL, deltaY);
          MotionEvent.PointerCoords[] coords = { coord };

          MotionEvent motionEvent = MotionEvent.obtain(
            downTime,
            eventTime,
            MotionEvent.ACTION_MOVE,
            pointerCount,
            prop,
            coords,
            metaState,
            buttonState,
            xPrecision,
            yPrecision,
            deviceId,
            edgeFlags,
            source,
            flags
          );
          webView.dispatchTouchEvent(motionEvent); */
        }
      });
    }

}
