const path = require('path');
const bindings = require(path.join(__dirname, 'build', 'Release', 'exokit.node'));
const {nativeAudio, nativeVr} = bindings;
const WindowWorker = require('window-worker');
const vmOne = require('vm-one');
const webGlToOpenGl = require('webgl-to-opengl');

bindings.nativeWorker = WindowWorker;
bindings.nativeVm = vmOne;
bindings.nativeGl = (nativeGl => {
  function WebGLRenderingContext(canvas) {
    const gl = new nativeGl();
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

    if (!WebGLRenderingContext.onconstruct || WebGLRenderingContext.onconstruct(gl, canvas)) {
      return gl;
    } else {
      return null;
    }
  }
  for (const k in nativeGl) {
    WebGLRenderingContext[k] = nativeGl[k];
  }
  WebGLRenderingContext.onconstruct = null;
  return WebGLRenderingContext;
})(bindings.nativeGl);

const {PannerNode} = nativeAudio;
PannerNode.setPath(path.join(require.resolve(__dirname, 'node_modules', 'node-native-audio-deps', 'assets', 'hrtf')));

nativeVr.EVRInitError = {
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

nativeVr.EVRApplicationType = {
  Other: 0,
  Scene: 1,
  Overlay: 2,
  Background: 3,
  Utility: 4,
  VRMonitor: 5,
  SteamWatchdog: 6,
  Bootstrapper: 7,
};

nativeVr.EVREye = {
  Left: 0,
  Right: 1,
};

nativeVr.ETrackingUniverseOrigin = {
  Seated: 0,
  Standing: 1,
  RawAndUncalibrated: 2,
};

nativeVr.ETrackingResult = {
  Uninitialized: 1,
  Calibrating_InProgress: 100,
  Calibrating_OutOfRange: 101,
  Running_OK: 200,
  Running_OutOfRange: 201,
};

nativeVr.ETrackedDeviceClass = {
  Invalid: 0,
  HMD: 1,
  Controller: 2,
  GenericTracker: 3,
  TrackingReference: 4,
  DisplayRedirect: 5,
};

module.exports = bindings;
