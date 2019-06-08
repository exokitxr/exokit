const path = require('path');
const {isMainThread, parentPort} = require('worker_threads');
const {process} = global;

const exokitNode = require(path.join(__dirname, '..', 'build', 'Release', 'exokit.node'));
const {nativeWindow} = exokitNode;

const webGlToOpenGl = require('webgl-to-opengl');
const symbols = require('./symbols');
const GlobalContext = require('./GlobalContext');

const bindings = {};
for (const k in exokitNode) {
  bindings[k] = exokitNode[k];
}

const isAndroid = bindings.nativePlatform === 'android';
const glslVersion = isAndroid ? '300 es' : '330';

const _decorateGlIntercepts = gl => {
  gl.createShader = (createShader => function(type) {
    const result = createShader.call(this, type);
    result.type = type;
    return result;
  })(gl.createShader);
  gl.shaderSource = (shaderSource => function(shader, source) {
    if (shader.type === gl.VERTEX_SHADER) {
      source = webGlToOpenGl.vertex(source, glslVersion);
    } else if (shader.type === gl.FRAGMENT_SHADER) {
      source = webGlToOpenGl.fragment(source, glslVersion);
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
const _onGl3DConstruct = (gl, canvas, attrs) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  gl.d = 3;
  gl.canvas = canvas;
  gl.attrs = {
    antialias: !!attrs.antialias,
    desynchronized: !!attrs.desynchronized,
  };

  const document = canvas.ownerDocument;
  const window = document.defaultView;

  const windowSpec = (() => {
    if (!window[symbols.optionsSymbol].args.nogl) {
      try {
        const contained = document.documentElement.contains(canvas);
        const {hidden} = document;
        const {headless} = window[symbols.optionsSymbol].args;
        // XXX also set title
        // const title = `Exokit ${GlobalContext.version}`;

        const windowHandle = nativeWindow.createWindowHandle(canvasWidth, canvasHeight, contained && !hidden && !headless);
        return nativeWindow.initWindow3D(windowHandle, gl);
      } catch (err) {
        console.warn(err.stack);
        return null;
      }
    } else {
      return null;
    }
  })();

  if (windowSpec) {
    const [windowHandle, vao] = windowSpec;

    gl.setWindowHandle(windowHandle);
    gl.setDefaultVao(vao);
    nativeWindow.setEventHandler(windowHandle, (type, data) => {
      switch (type) {
        case 'focus': {
          const {focused} = data;
          if (!focused && window.document.pointerLockElement) {
            window.document.exitPointerLock();
          }
          break;
        }
        case 'windowResize': {
          const {width, height} = data;
          window.innerWidth = width;
          window.innerHeight = height;

          window.dispatchEvent(new window.Event('resize'));
          break;
        }
        /* case 'framebufferResize': {
          const {width, height} = data;
          // innerWidth = width;
          // innerHeight = height;

          window.innerWidth = width / window.devicePixelRatio;
          window.innerHeight = height / window.devicePixelRatio;
          window.dispatchEvent(new window.Event('resize'));
          break;
        } */
        case 'keydown': {
          let handled = false;
          if (data.keyCode === 27) { // ESC
            if (window.document.pointerLockElement) {
              window.document.exitPointerLock();
              handled = true;
            }
            if (window.document.fullscreenElement) {
              window.document.exitFullscreen();
              handled = true;
            }
          }
          if (data.keyCode === 122) { // F11
            if (window.document.fullscreenElement) {
              window.document.exitFullscreen();
              handled = true;
            } else {
              window.document.requestFullscreen();
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
          gl.destroy();
          break;
        }
      }
    });
    
    const [fbo, tex, depthTex, msFbo, msTex, msDepthTex] = gl.attrs.desynchronized ? [
      0, 0, 0, 0, 0, 0,
    ] : nativeWindow.createRenderTarget(gl, canvasWidth, canvasHeight);
    if (msFbo) {
      gl.setDefaultFramebuffer(msFbo);
    }
    gl.framebuffer = {
      type: 'canvas',
      msFbo,
      msTex,
      msDepthTex,
      fbo,
      tex,
      depthTex,
    };
    parentPort.postMessage({
      method: 'emit',
      type: 'framebuffer',
      event: gl.framebuffer,
    });
    gl.resize = (width, height) => {
      if (!gl.attrs.desynchronized && gl.framebuffer.type === 'canvas') {
        nativeWindow.setCurrentWindowContext(windowHandle);
        const [newFbo, newTex, newDepthTex, newMsFbo, newMsTex, newMsDepthTex] = nativeWindow.resizeRenderTarget(gl, width, height, fbo, tex, depthTex, msFbo, msTex, msDepthTex);

        gl.framebuffer = {
          type: 'canvas',
          msFbo: newMsFbo,
          msTex: newMsTex,
          msDepthTex: newMsDepthTex,
          fbo: newFbo,
          tex: newTex,
          depthTex: newDepthTex,
        };
        parentPort.postMessage({
          method: 'emit',
          type: 'framebuffer',
          event: gl.framebuffer,
        });
      }
    };

    const ondomchange = () => {
      process.nextTick(() => { // show/hide synchronously emits events
        if (!document.hidden && !window[symbols.optionsSymbol].args.headless) {
          const domVisible = canvas.ownerDocument.documentElement.contains(canvas);
          const windowVisible = nativeWindow.isVisible(windowHandle);
          if (domVisible && !windowVisible) {
            nativeWindow.setVisibility(windowHandle, true);
          } else if (!domVisible && windowVisible) {
            nativeWindow.setVisibility(windowHandle, false);
          }
        }
      });
    };
    canvas.ownerDocument.on('domchange', ondomchange);

    gl.destroy = (destroy => function() {
      destroy.call(this);

      if (gl === GlobalContext.vrPresentState.glContext) {
        throw new Error('destroyed vr presenting context');
        /* bindings.nativeOpenVR.VR_Shutdown();

        GlobalContext.vrPresentState.glContextId = 0;
        GlobalContext.vrPresentState.system = null;
        GlobalContext.vrPresentState.compositor = null; */
      }

      nativeWindow.destroyWindowHandle(windowHandle);
      canvas._context = null;

      canvas.ownerDocument.removeListener('domchange', ondomchange);

      GlobalContext.contexts.splice(GlobalContext.contexts.indexOf(gl), 1);

      if (gl.id === 1) {
        process.kill(process.pid); // XXX make this a softer process.exit()
      }
    })(gl.destroy);
    
    gl.id = Atomics.add(GlobalContext.xrState.id, 0) + 1;
    GlobalContext.contexts.push(gl);

    if (gl.attrs.antialias) {
      GlobalContext.xrState.aaEnabled[0] = 1;
    }
  } else {
    gl.destroy();
  }
};
bindings.nativeGl = (nativeGl => {
  function WebGLRenderingContext(canvas, attrs) {
    const gl = new nativeGl();
    _decorateGlIntercepts(gl);
    _onGl3DConstruct(gl, canvas, attrs);
    return gl;
  }
  for (const k in nativeGl) {
    WebGLRenderingContext[k] = nativeGl[k];
  }
  return WebGLRenderingContext;
})(bindings.nativeGl);
bindings.nativeGl2 = (nativeGl2 => {
  function WebGL2RenderingContext(canvas, attrs) {
    const gl = new nativeGl2();
    _decorateGlIntercepts(gl);
    _onGl3DConstruct(gl, canvas, attrs);
    return gl;
  }
  for (const k in nativeGl2) {
    WebGL2RenderingContext[k] = nativeGl2[k];
  }
  return WebGL2RenderingContext;
})(bindings.nativeGl2);

const _onGl2DConstruct = (ctx, canvas, attrs) => {
  const canvasWidth = canvas.width || innerWidth;
  const canvasHeight = canvas.height || innerHeight;

  ctx.d = 2;
  ctx.canvas = canvas;

  const window = canvas.ownerDocument.defaultView;

  const windowSpec = (() => {
    if (!window[symbols.optionsSymbol].args.nogl) {
      try {
        const windowHandle = nativeWindow.createWindowHandle(canvasWidth, canvasHeight, false);
        return nativeWindow.initWindow2D(windowHandle);
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
      
      nativeWindow.destroyWindowHandle(windowHandle);
      canvas._context = null;

      GlobalContext.contexts.splice(GlobalContext.contexts.indexOf(ctx), 1);
    })(ctx.destroy);
    
    ctx.id = Atomics.add(GlobalContext.xrState.id, 0) + 1;
    GlobalContext.contexts.push(ctx);
  } else {
    ctx.destroy();
  }
};
bindings.nativeCanvasRenderingContext2D = (nativeCanvasRenderingContext2D => {
  function CanvasRenderingContext2D(canvas, attrs) {
    const ctx = new nativeCanvasRenderingContext2D();
    _onGl2DConstruct(ctx, canvas, attrs);
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

if (bindings.nativeAudio) {
  bindings.nativeAudio.AudioContext = (OldAudioContext => class AudioContext extends OldAudioContext {
    decodeAudioData(arrayBuffer, successCallback, errorCallback) {
      const result = new Promise((accept, reject) => {
        const audioBuffer = this.createEmptyBuffer();
        audioBuffer.load(arrayBuffer, errorString => {
          if (!errorString) {
            if (successCallback) {
              successCallback(audioBuffer);
            }
            accept(audioBuffer);
          } else {
            const err = new Error(errorString);
            if (errorCallback) {
              errorCallback(err);
            }
            reject(err);
          }
        });
      });
      return result;
    }
  })(bindings.nativeAudio.AudioContext);
  bindings.nativeAudio.PannerNode.setPath(path.join(require.resolve('native-audio-deps').slice(0, -'index.js'.length), 'assets', 'hrtf'));
}

if (bindings.nativeOpenVR) {
	bindings.nativeOpenVR.EVRInitError = {
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

	bindings.nativeOpenVR.EVRApplicationType = {
	  Other: 0,
	  Scene: 1,
	  Overlay: 2,
	  Background: 3,
	  Utility: 4,
	  VRMonitor: 5,
	  SteamWatchdog: 6,
	  Bootstrapper: 7,
	};

	bindings.nativeOpenVR.EVREye = {
	  Left: 0,
	  Right: 1,
	};

	bindings.nativeOpenVR.ETrackingUniverseOrigin = {
	  Seated: 0,
	  Standing: 1,
	  RawAndUncalibrated: 2,
	};

	bindings.nativeOpenVR.ETrackingResult = {
	  Uninitialized: 1,
	  Calibrating_InProgress: 100,
	  Calibrating_OutOfRange: 101,
	  Running_OK: 200,
	  Running_OutOfRange: 201,
	};

	bindings.nativeOpenVR.ETrackedDeviceClass = {
	  Invalid: 0,
	  HMD: 1,
	  Controller: 2,
	  GenericTracker: 3,
	  TrackingReference: 4,
	  DisplayRedirect: 5,
	};
}

GlobalContext.nativeOculusVR = bindings.nativeOculusVR;
GlobalContext.nativeOpenVR = bindings.nativeOpenVR;
GlobalContext.nativeOculusMobileVr = bindings.nativeOculusMobileVr;
GlobalContext.nativeMl = bindings.nativeMl;
GlobalContext.nativeBrowser = bindings.nativeBrowser;

if (bindings.nativeMl) {
  if (isMainThread) {
    if (!bindings.nativeMl.IsSimulated()) {
      bindings.nativeMl.InitLifecycle();
    } else {
      // try to connect to MLSDK
      const MLSDK_PORT = 17955;
      const s = net.connect(MLSDK_PORT, '127.0.0.1', () => {
        s.destroy();

        bindings.nativeMl.InitLifecycle();
      });
      s.on('error', () => {});
    }
  }
}

module.exports = bindings;
