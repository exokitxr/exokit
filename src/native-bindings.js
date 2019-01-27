const path = require('path');

const exokitNode = (() => {
  const oldCwd = process.cwd();
  const nodeModulesDir = path.resolve(path.dirname(require.resolve('native-graphics-deps')), '..');
  process.chdir(nodeModulesDir);
  const exokitNode = require(path.join(__dirname, '..', 'build', 'Release', 'exokit.node'));
  process.chdir(oldCwd);
  return exokitNode;
})();
const vmOne = require('vm-one');
const {nativeWindow} = exokitNode;
const webGlToOpenGl = require('webgl-to-opengl');
const GlobalContext = require('./GlobalContext');

const bindings = {};
for (const k in exokitNode) {
  bindings[k] = exokitNode[k];
}
bindings.nativeVm = vmOne;
const _decorateGlIntercepts = gl => {
  gl.createShader = (createShader => function(type) {
    const result = createShader.call(this, type);
    result.type = type;
    return result;
  })(gl.createShader);
  gl.shaderSource = (shaderSource => function(shader, source) {
    if (shader.type === gl.VERTEX_SHADER) {
      source = webGlToOpenGl.vertex(source);
    } else if (shader.type === gl.FRAGMENT_SHADER) {
      source = webGlToOpenGl.fragment(source);
    }
    return shaderSource.call(this, shader, source);
  })(gl.shaderSource);
  gl.getActiveAttrib = (getActiveAttrib => function(program, index) {
    const result = getActiveAttrib.call(this, program, index);
    if (result) {
      result.name = webGlToOpenGl.unmapName(result.name);
    }
    return result;
  })(gl.getActiveAttrib);
  gl.getActiveUniform = (getActiveUniform => function(program, index) {
    const result = getActiveUniform.call(this, program, index);
    if (result) {
      result.name = webGlToOpenGl.unmapName(result.name);
    }
    return result;
  })(gl.getActiveUniform);
  gl.getAttribLocation = (getAttribLocation => function(program, path) {
    path = webGlToOpenGl.mapName(path);
    return getAttribLocation.call(this, program, path);
  })(gl.getAttribLocation);
  gl.getUniformLocation = (getUniformLocation => function(program, path) {
    path = webGlToOpenGl.mapName(path);
    return getUniformLocation.call(this, program, path);
  })(gl.getUniformLocation);
  gl.setCompatibleXRDevice = () => Promise.resolve();
};
const _onGl3DConstruct = (gl, canvas) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  gl.canvas = canvas;

  const document = canvas.ownerDocument;
  const window = document.defaultView;

  const windowSpec = (() => {
    if (!args.headless) {
      try {
        const visible = document.documentElement.contains(canvas);
        const {hidden} = document;
        const firstWindowHandle = contexts.length > 0 ? contexts[0].getWindowHandle() : null;
        return nativeWindow.create3d(canvasWidth, canvasHeight, visible && !hidden, firstWindowHandle, gl);
      } catch (err) {
        console.warn(err.message);
        return null;
      }
    } else {
      return null;
    }
  })();

  if (windowSpec) {
    const [windowHandle, sharedFramebuffer, sharedColorTexture, sharedDepthStencilTexture, sharedMsFramebuffer, sharedMsColorTexture, sharedMsDepthStencilTexture, vao] = windowSpec;

    gl.setWindowHandle(windowHandle);
    gl.setDefaultVao(vao);
    nativeWindow.setEventHandler(windowHandle, (type, data) => {
      switch (type) {
        case 'focus': {
          const {focused} = data;
          if (!focused && window.top.document.pointerLockElement) {
            window.top.document.exitPointerLock();
          }
          break;
        }
        case 'framebufferResize': {
          const {width, height} = data;
          // innerWidth = width;
          // innerHeight = height;

          window.innerWidth = width / window.devicePixelRatio;
          window.innerHeight = height / window.devicePixelRatio;
          window.dispatchEvent(new window.Event('resize'));
          break;
        }
        case 'keydown': {
          let handled = false;
          if (data.keyCode === 27) { // ESC
            if (window.top.document.pointerLockElement) {
              window.top.document.exitPointerLock();
              handled = true;
            }
            if (window.top.document.fullscreenElement) {
              window.top.document.exitFullscreen();
              handled = true;
            }
          }
          if (data.keyCode === 122) { // F11
            if (window.top.document.fullscreenElement) {
              window.top.document.exitFullscreen();
              handled = true;
            } else {
              window.top.document.requestFullscreen();
              handled = true;
            }
          }

          if (!handled) {
            canvas.dispatchEvent(new window.KeyboardEvent(type, data));
          }
          break;
        }
        case 'keyup':
        case 'keypress': {
          canvas.dispatchEvent(new window.KeyboardEvent(type, data));
          break;
        }
        case 'mousedown':
        case 'mouseup':
        case 'click': {
          canvas.dispatchEvent(new window.MouseEvent(type, data));
          break;
        }
        case 'wheel': {
          canvas.dispatchEvent(new window.WheelEvent(type, data));
          break;
        }
        case 'mousemove': {
          canvas.dispatchEvent(new window.MouseEvent(type, data));
          break;
        }
        case 'drop': {
          const _readFiles = paths => {
            const result = [];

            return Promise.all(paths.map(p =>
              new Promise((accept, reject) => {
                fs.lstat(p, (err, stats) => {
                  if (!err) {
                    if (stats.isFile()) {
                      fs.readFile(p, (err, data) => {
                        if (!err) {
                          const file = new window.Blob([data]);
                          file.name = path.basename(p);
                          file.path = p;
                          result.push(file);

                          accept();
                        } else {
                          reject(err);
                        }
                      });
                    } else if (stats.isDirectory()) {
                      fs.readdir(p, (err, fileNames) => {
                        if (!err) {
                          _readFiles(fileNames.map(fileName => path.join(p, fileName)))
                            .then(files => {
                              result.push.apply(result, files);

                              accept();
                            })
                            .catch(err => {
                              reject(err);
                            });
                        } else {
                          reject(err);
                        }
                      });
                    } else {
                      accept();
                    }
                  } else {
                    reject(err);
                  }
                });
              })
            ))
              .then(() => result);
          };

          _readFiles(data.paths)
            .then(files => {
              const dataTransfer = new window.DataTransfer({
                files,
              });
              const e = new window.DragEvent('drop');
              e.dataTransfer = dataTransfer;
              canvas.dispatchEvent(e);
            })
            .catch(err => {
              console.warn(err.stack);
            });
          break;
        }
        case 'quit': {
          context.destroy();
          break;
        }
      }
    });

    const nativeWindowSize = nativeWindow.getFramebufferSize(windowHandle);
    const nativeWindowHeight = nativeWindowSize.height;
    const nativeWindowWidth = nativeWindowSize.width;

    // Calculate devicePixelRatio.
    window.devicePixelRatio = nativeWindowWidth / canvasWidth;

    // Tell DOM how large the window is.
    // window.innerHeight = nativeWindowHeight / window.devicePixelRatio;
    // window.innerWidth = nativeWindowWidth / window.devicePixelRatio;

    const title = `Exokit ${version}`;
    nativeWindow.setWindowTitle(windowHandle, title);

    const {hidden} = document;
    if (hidden) {
      const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeWindow.createRenderTarget(gl, canvasWidth, canvasHeight, sharedColorTexture, sharedDepthStencilTexture, sharedMsColorTexture, sharedMsDepthStencilTexture);

      gl.setDefaultFramebuffer(msFbo);

      gl.resize = (width, height) => {
        nativeWindow.setCurrentWindowContext(windowHandle);
        nativeWindow.resizeRenderTarget(gl, width, height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
      };

      // TODO: handle multiple child canvases
      document.framebuffer = {
        canvas,
        msFbo,
        msTex,
        msDepthTex,
        fbo,
        tex,
        depthTex,
      };
    } else {
      gl.resize = (width, height) => {
        nativeWindow.setCurrentWindowContext(windowHandle);
        nativeWindow.resizeRenderTarget(gl, width, height, sharedFramebuffer, sharedColorTexture, sharedDepthStencilTexture, sharedMsFramebuffer, sharedMsColorTexture, sharedMsDepthStencilTexture);
      };
    }

    const ondomchange = () => {
      process.nextTick(() => { // show/hide synchronously emits events
        if (!hidden) {
          const domVisible = canvas.ownerDocument.documentElement.contains(canvas);
          const windowVisible = nativeWindow.isVisible(windowHandle);
          if (domVisible) {
            if (!windowVisible) {
              nativeWindow.show(windowHandle);
            }
          } else {
            if (windowVisible) {
              nativeWindow.hide(windowHandle);
            }
          }
        }
      });
    };
    canvas.ownerDocument.on('domchange', ondomchange);

    gl.destroy = (destroy => function() {
      destroy.call(this);

      nativeWindow.setCurrentWindowContext(windowHandle);

      if (gl === vrPresentState.glContext) {
        nativeVr.VR_Shutdown();

        vrPresentState.glContext = null;
        vrPresentState.system = null;
        vrPresentState.compositor = null;
      }
      if (gl === mlPresentState.mlGlContext) {
        mlPresentState.mlGlContext = null;
      }

      nativeWindow.destroy(windowHandle);
      canvas._context = null;

      if (hidden) {
        document._emit('framebuffer', null);
      }
      canvas.ownerDocument.removeListener('domchange', ondomchange);

      window.postMessage({
        method: 'context.destroy',
        args: {
          address: gl.toAddressArray(),
        },
      });
      // contexts.splice(contexts.indexOf(gl), 1);

      if (!contexts.some(context => nativeWindow.isVisible(context.getWindowHandle()))) { // XXX handle this in the parent
        process.exit();
      }
    })(gl.destroy);
  } else {
    gl.destroy();
  }

  window.postMessage({ // XXX handle these
    method: 'context.create',
    args: {
      address: gl.toAddressArray(),
    },
  });
  // contexts.push(gl);
};
bindings.nativeGl = (nativeGl => {
  function WebGLRenderingContext(canvas) {
    const gl = new nativeGl();
    _decorateGlIntercepts(gl);
    _onGl3DConstruct(gl, canvas);
    return gl;
  }
  for (const k in nativeGl) {
    WebGLRenderingContext[k] = nativeGl[k];
  }
  return WebGLRenderingContext;
})(bindings.nativeGl);
bindings.nativeGl2 = (nativeGl2 => {
  function WebGL2RenderingContext(canvas) {
    const gl = new nativeGl2();
    _decorateGlIntercepts(gl);
    _onGl3DConstruct(gl, canvas);
    return gl;
  }
  for (const k in nativeGl2) {
    WebGL2RenderingContext[k] = nativeGl2[k];
  }
  return WebGL2RenderingContext;
})(bindings.nativeGl2);

const _onGl2DConstruct = (ctx, canvas) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  ctx.canvas = canvas;

  const window = canvas.ownerDocument.defaultView;

  const windowSpec = (() => {
    if (!window[symbols.optionsSymbol].args.headless) {
      try {
        const firstWindowHandle = contexts.length > 0 ? contexts[0].getWindowHandle() : null;
        return nativeWindow.create2d(canvasWidth, canvasHeight, firstWindowHandle);
      } catch (err) {
        console.warn(err.message);
        return null;
      }
    } else {
      return null;
    }
  })();

  if (windowSpec) {
    const [windowHandle, tex] = windowSpec;

    ctx.setWindowHandle(windowHandle);
    ctx.setTexture(tex, canvasWidth, canvasHeight);
    
    ctx.destroy = (destroy => function() {
      destroy.call(this);
      
      nativeWindow.destroy(windowHandle);
      canvas._context = null;
      
      window.postMessage({
        method: 'context.destroy',
        args: {
          address: ctx.toAddressArray(),
        },
      });
      // contexts.splice(contexts.indexOf(ctx), 1);
    })(ctx.destroy);
  } else {
    ctx.destroy();
  }
  
  window.postMessage({
    method: 'context.create',
    args: {
      address: ctx.toAddressArray(),
    },
  });
  // contexts.push(ctx);
};
bindings.nativeCanvasRenderingContext2D = (nativeCanvasRenderingContext2D => {
  function CanvasRenderingContext2D(canvas) {
    const ctx = new nativeCanvasRenderingContext2D();
    _onGl2DConstruct(ctx, canvas);
    return ctx;
  }
  for (const k in nativeCanvasRenderingContext2D) {
    CanvasRenderingContext2D[k] = nativeCanvasRenderingContext2D[k];
  }
  return CanvasRenderingContext2D;
})(bindings.nativeCanvasRenderingContext2D);
GlobalContext.CanvasRenderingContext2D = bindings.nativeCanvasRenderingContext2D;
GlobalContext.WebGLRenderingContext = bindings.nativeGl;
GlobalContext.WebGL2RenderingContext = bindings.nativeGl2;

bindings.nativeAudio.AudioContext = (OldAudioContext => class AudioContext extends OldAudioContext {
  decodeAudioData(arrayBuffer, successCallback, errorCallback) {
    return new Promise((resolve, reject) => {
      try {
        let audioBuffer = this._decodeAudioDataSync(arrayBuffer);
        if (successCallback) {
          process.nextTick(() => {
            try {
              successCallback(audioBuffer);
            } catch(err) {
              console.warn(err);
            }
          });
        }
        resolve(audioBuffer);
      } catch(err) {
        console.warn(err);
        if (errorCallback) {
          process.nextTick(() => {
            try {
              errorCallback(err);
            } catch(err) {
              console.warn(err);
            }
          });
        }
        reject(err);
      }
    });
  }
})(bindings.nativeAudio.AudioContext);

bindings.nativeAudio.PannerNode.setPath(path.join(require.resolve('native-audio-deps').slice(0, -'index.js'.length), 'assets', 'hrtf'));

if (bindings.nativeVr) {
	bindings.nativeVr.EVRInitError = {
	  None: 0,
	  Unknown: 1,

	  Init_InstallationNotFound: 100,
	  Init_InstallationCorrupt: 101,
	  Init_VRClientDLLNotFound: 102,
	  Init_FileNotFound: 103,
	  Init_FactoryNotFound: 104,
	  Init_InterfaceNotFound: 105,
	  Init_InvalidInterface: 106,
	  Init_UserConfigDirectoryInvalid: 107,
	  Init_HmdNotFound: 108,
	  Init_NotInitialized: 109,
	  Init_PathRegistryNotFound: 110,
	  Init_NoConfigPath: 111,
	  Init_NoLogPath: 112,
	  Init_PathRegistryNotWritable: 113,
	  Init_AppInfoInitFailed: 114,
	  Init_Retry: 115,
	  Init_InitCanceledByUser: 116,
	  Init_AnotherAppLaunching: 117,
	  Init_SettingsInitFailed: 118,
	  Init_ShuttingDown: 119,
	  Init_TooManyObjects: 120,
	  Init_NoServerForBackgroundApp: 121,
	  Init_NotSupportedWithCompositor: 122,
	  Init_NotAvailableToUtilityApps: 123,
	  Init_Internal: 124,
	  Init_HmdDriverIdIsNone: 125,
	  Init_HmdNotFoundPresenceFailed: 126,
	  Init_VRMonitorNotFound: 127,
	  Init_VRMonitorStartupFailed: 128,
	  Init_LowPowerWatchdogNotSupported: 129,
	  Init_InvalidApplicationType: 130,
	  Init_NotAvailableToWatchdogApps: 131,
	  Init_WatchdogDisabledInSettings: 132,
	  Init_VRDashboardNotFound: 133,
	  Init_VRDashboardStartupFailed: 134,

	  Driver_Failed: 200,
	  Driver_Unknown: 201,
	  Driver_HmdUnknown: 202,
	  Driver_NotLoaded: 203,
	  Driver_RuntimeOutOfDate: 204,
	  Driver_HmdInUse: 205,
	  Driver_NotCalibrated: 206,
	  Driver_CalibrationInvalid: 207,
	  Driver_HmdDisplayNotFound: 208,
	  Driver_TrackedDeviceInterfaceUnknown: 209,
	  Driver_HmdDriverIdOutOfBounds: 211,
	  Driver_HmdDisplayMirrored: 212,

	  IPC_ServerInitFailed: 300,
	  IPC_ConnectFailed: 301,
	  IPC_SharedStateInitFailed: 302,
	  IPC_CompositorInitFailed: 303,
	  IPC_MutexInitFailed: 304,
	  IPC_Failed: 305,
	  IPC_CompositorConnectFailed: 306,
	  IPC_CompositorInvalidConnectResponse: 307,
	  IPC_ConnectFailedAfterMultipleAttempts: 308,

	  Compositor_Failed: 400,
	  Compositor_D3D11HardwareRequired: 401,
	  Compositor_FirmwareRequiresUpdate: 402,
	  Compositor_OverlayInitFailed: 403,
	  Compositor_ScreenshotsInitFailed: 404,

	  VendorSpecific_UnableToConnectToOculusRuntime: 1000,

	  VendorSpecific_HmdFound_CantOpenDevice: 1101,
	  VendorSpecific_HmdFound_UnableToRequestConfigStart: 1102,
	  VendorSpecific_HmdFound_NoStoredConfig: 1103,
	  VendorSpecific_HmdFound_ConfigTooBig: 1104,
	  VendorSpecific_HmdFound_ConfigTooSmall: 1105,
	  VendorSpecific_HmdFound_UnableToInitZLib: 1106,
	  VendorSpecific_HmdFound_CantReadFirmwareVersion: 1107,
	  VendorSpecific_HmdFound_UnableToSendUserDataStart: 1108,
	  VendorSpecific_HmdFound_UnableToGetUserDataStart: 1109,
	  VendorSpecific_HmdFound_UnableToGetUserDataNext: 1110,
	  VendorSpecific_HmdFound_UserDataAddressRange: 1111,
	  VendorSpecific_HmdFound_UserDataError: 1112,
	  VendorSpecific_HmdFound_ConfigFailedSanityCheck: 1113,

	  Steam_SteamInstallationNotFound: 2000,
	};

	bindings.nativeVr.EVRApplicationType = {
	  Other: 0,
	  Scene: 1,
	  Overlay: 2,
	  Background: 3,
	  Utility: 4,
	  VRMonitor: 5,
	  SteamWatchdog: 6,
	  Bootstrapper: 7,
	};

	bindings.nativeVr.EVREye = {
	  Left: 0,
	  Right: 1,
	};

	bindings.nativeVr.ETrackingUniverseOrigin = {
	  Seated: 0,
	  Standing: 1,
	  RawAndUncalibrated: 2,
	};

	bindings.nativeVr.ETrackingResult = {
	  Uninitialized: 1,
	  Calibrating_InProgress: 100,
	  Calibrating_OutOfRange: 101,
	  Running_OK: 200,
	  Running_OutOfRange: 201,
	};

	bindings.nativeVr.ETrackedDeviceClass = {
	  Invalid: 0,
	  HMD: 1,
	  Controller: 2,
	  GenericTracker: 3,
	  TrackingReference: 4,
	  DisplayRedirect: 5,
	};
}

GlobalContext.nativeVr = bindings.nativeVr;
GlobalContext.nativeMl = bindings.nativeMl;
GlobalContext.nativeBrowser = bindings.nativeBrowser;

if (nativeVr) {
  nativeVr.requestPresent = async function(layers) { // XXX handle this inside window context
    const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
    if (layer) {
      const canvas = layer.source;
      
      if (!vrPresentState.glContext) {
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }
        const window = canvas.ownerDocument.defaultView;

        await window.postRequest({ // XXX implement these
          method: 'requestPresentVr',
        });
        
        const windowHandle = context.getWindowHandle();
        nativeWindow.setCurrentWindowContext(windowHandle);

        const vrContext = vrPresentState.vrContext || nativeVr.getContext();
        const system = vrPresentState.system || nativeVr.VR_Init(nativeVr.EVRApplicationType.Scene);
        const compositor = vrPresentState.compositor || vrContext.compositor.NewCompositor();

        // const lmContext = vrPresentState.lmContext || (nativeLm && new nativeLm());

        const {width: halfWidth, height} = system.GetRecommendedRenderTargetSize();
        const width = halfWidth * 2;
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;

        const cleanups = [];

        const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeWindow.createRenderTarget(context, width, height, 0, 0, 0, 0);

        context.setDefaultFramebuffer(msFbo);

        vrPresentState.isPresenting = true; // XXX make these objects use SharedArrayBuffer
        vrPresentState.vrContext = vrContext; // XXX bubble this up to the top for WaitGetPoses/SubmitFrame
        vrPresentState.system = system;
        vrPresentState.compositor = compositor;
        vrPresentState.glContext = context;
        vrPresentState.msFbo = msFbo;
        vrPresentState.msTex = msTex;
        vrPresentState.msDepthTex = msDepthTex;
        vrPresentState.fbo = fbo;
        vrPresentState.tex = tex;
        vrPresentState.depthTex = depthTex;
        vrPresentState.cleanups = cleanups;

        // vrPresentState.lmContext = lmContext;

        canvas.framebuffer = { // XXX bubble this up to the top for layers support
          width,
          height,
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };

        const _attribute = (name, value) => {
          if (name === 'width' || name === 'height') {
            nativeWindow.setCurrentWindowContext(windowHandle);

            nativeWindow.resizeRenderTarget(context, canvas.width, canvas.height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
          }
        };
        canvas.on('attribute', _attribute);
        cleanups.push(() => {
          canvas.removeListener('attribute', _attribute);
        });

        return canvas.framebuffer;
      } else if (canvas.ownerDocument.framebuffer) {
        const {width, height} = canvas;
        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = canvas.ownerDocument.framebuffer;
        return {
          width,
          height,
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };
      } else {
        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = vrPresentState;
        return {
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };
      }
    } else {
      throw new Error('no HTMLCanvasElement source provided');
    }
  };
  nativeVr.exitPresent = await function() {
    await window.postRequest({
      method: 'exitPresentVr',
    });
    
    if (vrPresentState.isPresenting) {
      nativeVr.VR_Shutdown();

      nativeWindow.destroyRenderTarget(vrPresentState.msFbo, vrPresentState.msTex, vrPresentState.msDepthStencilTex);
      nativeWindow.destroyRenderTarget(vrPresentState.fbo, vrPresentState.tex, vrPresentState.msDepthTex);

      const context = vrPresentState.glContext;
      nativeWindow.setCurrentWindowContext(context.getWindowHandle());
      context.setDefaultFramebuffer(0);

      for (let i = 0; i < vrPresentState.cleanups.length; i++) {
        vrPresentState.cleanups[i]();
      }

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
      vrPresentState.cleanups = null;
    }

    return Promise.resolve();
  };
}

if (nativeMl) {
  mlPresentState.mlContext = new nativeMl(); // XXX move this to on present
  nativeMl.requestPresent = async function(layers) { // XXX handle this inside window context
    await window.postRequest({
      method: 'requestPresentMl',
    });
  
    const layer = layers.find(layer => layer && layer.source && layer.source.tagName === 'CANVAS');
    if (layer) {
      const canvas = layer.source;
      
      if (!mlPresentState.mlGlContext) {
        let context = canvas._context;
        if (!(context && context.constructor && context.constructor.name === 'WebGLRenderingContext')) {
          context = canvas.getContext('webgl');
        }

        const windowHandle = context.getWindowHandle();
        nativeWindow.setCurrentWindowContext(windowHandle);

        mlPresentState.mlContext.Present(windowHandle, context);

        const {width: halfWidth, height} = mlPresentState.mlContext.GetSize();
        const width = halfWidth * 2;

        const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = nativeWindow.createRenderTarget(context, width, height, 0, 0, 0, 0);
        mlPresentState.mlContext.SetContentTexture(tex);
        /* const {
          width: halfWidth,
          height,
          fbo,
          colorTex: tex,
          depthStencilTex: depthTex,
          msFbo,
          msColorTex: msTex,
          msDepthStencilTex: msDepthTex,
        } = initResult; */
        xrState.renderWidth[0] = halfWidth;
        xrState.renderHeight[0] = height;

        mlPresentState.mlFbo = fbo;
        mlPresentState.mlTex = tex;
        mlPresentState.mlDepthTex = depthTex;
        mlPresentState.mlMsFbo = msFbo;
        mlPresentState.mlMsTex = msTex;
        mlPresentState.mlMsDepthTex = msDepthTex;

        canvas.framebuffer = {
          width,
          height,
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };

        const cleanups = [];
        mlPresentState.mlCleanups = cleanups;

        const _attribute = (name, value) => {
          if (name === 'width' || name === 'height') {
            nativeWindow.setCurrentWindowContext(windowHandle);

            nativeWindow.resizeRenderTarget(context, canvas.width, canvas.height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);
          }
        };
        canvas.on('attribute', _attribute);
        cleanups.push(() => {
          canvas.removeListener('attribute', _attribute);
        });

        context.setDefaultFramebuffer(msFbo);

        mlPresentState.mlGlContext = context;

        return canvas.framebuffer;
      } else if (canvas.ownerDocument.framebuffer) {
        const {width, height} = canvas;
        const {msFbo, msTex, msDepthTex, fbo, tex, depthTex} = canvas.ownerDocument.framebuffer;
        
        return {
          width,
          height,
          msFbo,
          msTex,
          msDepthTex,
          fbo,
          tex,
          depthTex,
        };
      } else {
        return {
          width: xrState.renderWidth[0] * 2,
          height: xrState.renderHeight[0],
          msFbo: mlPresentState.mlMsFbo,
          msTex: mlPresentState.mlMsTex,
          msDepthTex: mlPresentState.mlMsDepthTex,
          fbo: mlPresentState.mlFbo,
          tex: mlPresentState.mlTex,
          depthTex: mlPresentState.mlDepthTex,
        };
      }
    } else {
      throw new Error('no HTMLCanvasElement source provided');
    }
  };
  nativeMl.exitPresent = function() {
    await window.postRequest({
      method: 'exitPresentMl',
    });
    
    nativeWindow.destroyRenderTarget(mlPresentState.mlMsFbo, mlPresentState.mlMsTex, mlPresentState.mlMsDepthTex);
    nativeWindow.destroyRenderTarget(mlPresentState.mlFbo, mlPresentState.mlTex, mlPresentState.mlDepthTex);

    nativeWindow.setCurrentWindowContext(mlPresentState.mlGlContext.getWindowHandle());
    mlPresentState.mlGlContext.setDefaultFramebuffer(0);

    for (let i = 0; i < mlPresentState.mlCleanups.length; i++) {
      mlPresentState.mlCleanups[i]();
    }

    mlPresentState.mlFbo = null;
    mlPresentState.mlTex = null;
    mlPresentState.mlDepthTex = null;
    mlPresentState.mlMsFbo = null;
    mlPresentState.mlMsTex = null;
    mlPresentState.mlMsDepthTex = null;
    mlPresentState.mlGlContext = null;
    mlPresentState.mlCleanups = null;
    mlPresentState.mlHasPose = false;
  };

  const _mlLifecycleEvent = e => {
    console.log('got ml lifecycle event', e);

    switch (e) {
      case 'newInitArg': {
        break;
      }
      case 'stop':
      case 'pause': {
        if (mlPresentState.mlContext) {
          mlPresentState.mlContext.Exit();
        }
        nativeMl.DeinitLifecycle();
        process.exit();
        break;
      }
      case 'resume': {
        break;
      }
      case 'unloadResources': {
        break;
      }
      default: {
        console.warn('invalid ml lifecycle event', e);
        break;
      }
    }
  };
  const _mlKeyboardEvent = e => {
    // console.log('got ml keyboard event', e);

    if (mlPresentState.mlGlContext) {
      const {canvas} = mlPresentState.mlGlContext; // XXX handle this per-window, like native window
      const window = canvas.ownerDocument.defaultView;

      switch (e.type) {
        case 'keydown': {
          let handled = false;
          if (e.keyCode === 27) { // ESC
            if (window.top.document.pointerLockElement) {
              window.top.document.exitPointerLock();
              handled = true;
            }
            if (window.top.document.fullscreenElement) {
              window.top.document.exitFullscreen();
              handled = true;
            }
          }
          if (e.keyCode === 122) { // F11
            if (window.top.document.fullscreenElement) {
              window.top.document.exitFullscreen();
              handled = true;
            } else {
              window.top.document.requestFullscreen();
              handled = true;
            }
          }

          if (!handled) {
            canvas.dispatchEvent(new window.KeyboardEvent(e.type, e));
          }
          break;
        }
        case 'keyup':
        case 'keypress': {
          canvas.dispatchEvent(new window.KeyboardEvent(e.type, e));
          break;
        }
        default:
          break;
      }
    }
  };
  if (!nativeMl.IsSimulated()) {
    nativeMl.InitLifecycle(_mlLifecycleEvent, _mlKeyboardEvent);
  } else {
    // try to connect to MLSDK
    const MLSDK_PORT = 17955;
    const s = net.connect(MLSDK_PORT, '127.0.0.1', () => {
      s.destroy();

      nativeMl.InitLifecycle(_mlLifecycleEvent, _mlKeyboardEvent);
    });
    s.on('error', () => {});
  }
}

module.exports = bindings;
