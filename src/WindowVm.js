const nativeWorker = require('worker-native');
const GlobalContext = require('./GlobalContext');

const _makeWindow = (options = {}) => {
  const window = nativeWorker.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: options,
  });
  
  window.tickAnimationFrame = () => {
    window.runDetached(`window.tickAnimationFrame();`);
  };
  const requestHandlers = {
    'context.create'(args) {
      const {id} = args;
      GlobalContext.contexts.push({
        window,
        id,
        framebuffer,
      });
    },
    'context.destroy'(args) {
      const {id} = args;
      const index = GlobalContext.contexts.findIndex(context => context.window === window && context.id === id);
      GlobalContext.contexts.splice(index, 1);
      
      if (!GlobalContext.contexts.some(context => nativeWindow.isVisible(context.getWindowHandle()))) { // XXX handle window handle access
        process.exit();
      }
    },
    requestPresentVr() {
      const vrContext = vrPresentState.vrContext || nativeVr.getContext();
      const system = vrPresentState.system || nativeVr.VR_Init(nativeVr.EVRApplicationType.Scene);
      const compositor = vrPresentState.compositor || vrContext.compositor.NewCompositor();

      // const lmContext = vrPresentState.lmContext || (nativeLm && new nativeLm());

      vrPresentState.vrContext = vrContext;
      vrPresentState.system = system;
      vrPresentState.compositor = compositor;

      let {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
      const MAX_TEXTURE_SIZE = 4096;
      const MAX_TEXTURE_SIZE_HALF = MAX_TEXTURE_SIZE/2;
      if (halfWidth > MAX_TEXTURE_SIZE_HALF) {
        const factor = halfWidth / MAX_TEXTURE_SIZE_HALF;
        halfWidth = MAX_TEXTURE_SIZE_HALF;
        height = Math.floor(height / factor);
      }
      const width = halfWidth * 2;
      xrState.renderWidth[0] = halfWidth;
      xrState.renderHeight[0] = height;

      return Promise.resolve({
        width,
        height,
      });
    },
    'vr.bind'(args) {
      const {framebuffer, id} = args;
      const {
        msFbo,
        msTex,
        msDepthTex,
        fbo,
        tex,
        depthTex,
      } = framebuffer;
      
      vrPresentState.isPresenting = true;
      const context = GlobalContext.contexts.find(context => context.window === window && context.id === id);
      context.framebuffer = framebuffer;
      vrPresentState.glContext = context;
      vrPresentState.msFbo = msFbo;
      vrPresentState.msTex = msTex;
      vrPresentState.msDepthTex = msDepthTex;
      vrPresentState.fbo = fbo;
      vrPresentState.tex = tex;
      vrPresentState.depthTex = depthTex;
    },
    exitPresentVr() {
      nativeVr.VR_Shutdown();
      
      vrPresentState.isPresenting = false;
      vrPresentState.system = null;
      vrPresentState.compositor = null;
      vrPresentState.glContext = null;
      vrPresentState.msFbo = null;
      vrPresentState.msTex = null;
      vrPresentState.msDepthTex = null;
      vrPresentState.fbo = null;
      vrPresentState.tex = null;
      vrPresentState.depthTex = null;
    },
    requestPresentMl() {
      mlPresentState.mlContext = new nativeMl();
      mlPresentState.mlContext.Present(windowHandle, context); // XXX remove context dependency

      const {width: halfWidth, height} = mlPresentState.mlContext.GetSize();
      const width = halfWidth * 2;
      xrState.renderWidth[0] = halfWidth;
      xrState.renderHeight[0] = height;
      
      return Promise.resolve({
        width,
        height,
      });
    },
    'ml.bind'(args) {
      const {framebuffer, id} = args;
      const {
        msFbo,
        msTex,
        msDepthTex,
        fbo,
        tex,
        depthTex,
      } = framebuffer;

      mlPresentState.mlContext.SetContentTexture(tex);

      const context = GlobalContext.contexts.find(context => context.window === window && context.id === id);
      context.framebuffer = framebuffer;
      mlPresentState.mlGlContext = context;
      mlPresentState.mlFbo = fbo;
      mlPresentState.mlTex = tex;
      mlPresentState.mlDepthTex = depthTex;
      mlPresentState.mlMsFbo = msFbo;
      mlPresentState.mlMsTex = msTex;
      mlPresentState.mlMsDepthTex = msDepthTex
    },
    exitPresentMl() {
      mlPresentState.mlContext.Exit();
      mlPresentState.mlContext.Destroy();

      mlPresentState.mlContext = null;
      mlPresentState.mlFbo = null;
      mlPresentState.mlTex = null;
      mlPresentState.mlDepthTex = null;
      mlPresentState.mlMsFbo = null;
      mlPresentState.mlMsTex = null;
      mlPresentState.mlMsDepthTex = null;
      mlPresentState.mlGlContext = null;
      mlPresentState.mlCleanups = null;
      mlPresentState.mlHasPose = false;
    },
  };
  /* window.setRequestHandler = (method, handler) => {
    requestHandlers[method] = handler;
  }; */
  window.oninternalmessage = async m => {
    switch (m.type) {
      case 'postRequestAsync': {
        const handler = requestHandlers[m.method];
        if (handler) {
          let result, error;
          try {
            result = await handler(args);
          } catch(err) {
            error = err;
          }
          window,postInternalMessage({
            type: 'postRequestAsync',
            id: m.id,
            result,
            error,
          });
        }
        break;
      }
    }
  };
  window.on('exit', () => {
    window.emit('destroy');
  });

  return window;
};
module.exports._makeWindow = _makeWindow;
GlobalContext._makeWindow = _makeWindow;

const _makeWindowWithDocument = (s, options) => { // XXX fold this into Window
  const window = _makeWindow(options);
  window.document = _parseDocument(s, window);
  return window;
};
module.exports._makeWindowWithDocument = _makeWindowWithDocument;
