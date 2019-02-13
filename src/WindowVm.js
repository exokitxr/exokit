const nativeWorker = require('worker-native');
const GlobalContext = require('./GlobalContext');

const _makeWindow = (options = {}) => {
  if (!GlobalContext.xrState.windowHandle[0] && !GlobalContext.xrState.windowHandle[1]) {
    GlobalContext.xrState.windowHandle.set(nativeWindow.createNull());
  }
  
  const window = nativeWorker.make({
    initModule: path.join(__dirname, 'Window.js'),
    args: {
      options,
      xrState: GlobalContext.xrState,
    },
  });
  
  window.tickAnimationFrame = () => window.runAsync(`return window.tickAnimationFrame();`);
  const requestHandlers = {
    requestPresentVr() {
      const {vrPresentState} = GlobalContext;
      
      if (!vrPresentState.vrContext) {
        const vrContext = nativeVr.getContext();
        const system = nativeVr.VR_Init(nativeVr.EVRApplicationType.Scene);
        const compositor = vrContext.compositor.NewCompositor();

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
          wasPresenting: false,
          width,
          height,
        });
      } else {
        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = vrPresentState;
        return Promise.resolve({
          wasPresenting: true,
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        });
      }
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
      const {vrPresentState} = GlobalContext;
      
      const context = GlobalContext.contexts.find(context => context.window === window && context.id === id);
      context.framebuffer = framebuffer;
      vrPresentState.glContextId = context.id;
      vrPresentState.msFbo = msFbo;
      vrPresentState.msTex = msTex;
      vrPresentState.msDepthTex = msDepthTex;
      vrPresentState.fbo = fbo;
      vrPresentState.tex = tex;
      vrPresentState.depthTex = depthTex;
    },
    exitPresentVr() {
      const {vrPresentState} = GlobalContext;
      
      if (vrPresentState.vrContext) {
        nativeVr.VR_Shutdown();
        
        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = vrPresentState;

        vrPresentState.vrContext = null;
        vrPresentState.system = null;
        vrPresentState.compositor = null;
        vrPresentState.glContextId = 0;
        vrPresentState.msFbo = null;
        vrPresentState.msTex = null;
        vrPresentState.msDepthTex = null;
        vrPresentState.fbo = null;
        vrPresentState.tex = null;
        vrPresentState.depthTex = null;
        
        return Promise.resolve({
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        });
      } else {
        return Promise.resolve(null);
      }
    },
    requestPresentMl() {
      const {mlPresentState} = GlobalContext;
      
      if (!mlPresentState.mlContext) {
        mlPresentState.mlContext = new nativeMl();
        mlPresentState.mlContext.Present(windowHandle, context); // XXX remove context dependency

        const {width: halfWidth, height} = mlPresentState.mlContext.GetSize();
        const width = halfWidth * 2;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;
        
        return Promise.resolve({
          wasPresenting: false,
          width,
          height,
        });
      } else {
        const {mlMsFbo: msFbo, mlMsTex: msTex, mlMsDepthTex: msDepthTex, mlFbo: fbo, mlTex: tex, mlDepthTex: depthTex} = mlPresentState;
        return Promise.resolve({
          wasPresenting: true,
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        });
        /* const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = vrPresentState;
        return {
          wasPresenting: true,
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        }; */
      }
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
      const {mlPresentState} = GlobalContext;

      mlPresentState.mlContext.SetContentTexture(tex);

      const context = GlobalContext.contexts.find(context => context.window === window && context.id === id);
      context.framebuffer = framebuffer;
      mlPresentState.mlGlContextId = context.id;
      mlPresentState.mlFbo = fbo;
      mlPresentState.mlTex = tex;
      mlPresentState.mlDepthTex = depthTex;
      mlPresentState.mlMsFbo = msFbo;
      mlPresentState.mlMsTex = msTex;
      mlPresentState.mlMsDepthTex = msDepthTex
    },
    exitPresentMl() {
      const {mlPresentState} = GlobalContext;
      
      if (mlPresentState.mlContext) {
        mlPresentState.mlContext.Exit();
        mlPresentState.mlContext.Destroy();

        const {mlMsFbo: msFbo, mlMsTex: msTex, mlMsDepthTex: msDepthTex, mlFbo: fbo, mlTex: tex, mlDepthTex: depthTex} = mlPresentState;

        mlPresentState.mlContext = null;
        mlPresentState.mlGlContextId = 0;
        mlPresentState.mlFbo = null;
        mlPresentState.mlTex = null;
        mlPresentState.mlDepthTex = null;
        mlPresentState.mlMsFbo = null;
        mlPresentState.mlMsTex = null;
        mlPresentState.mlMsDepthTex = null;
        mlPresentState.mlCleanups = null;
        mlPresentState.mlHasPose = false;
        
        return Promise.resolve({
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        });
      } else {
        return Promise.resolve(null);
      }
    },
  };
  window.oninternalmessage = async m => {
    switch (m.type) {
      case 'postRequestAsync': {
        const handler = requestHandlers[m.method];
        let result, error;
        if (handler) {
          try {
            result = await handler(args);
          } catch(err) {
            error = err;
          }
        } else {
          error = new Error(`method not found: ${m.method}`);
        }
        window.postInternalMessage({
          type: 'postRequestAsync',
          id: m.id,
          result,
          error,
        });
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
