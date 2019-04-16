#ifdef ANDROID

#include <oculus-context.h>

namespace oculusmobile {

ovrJava java;

OculusMobileContext::OculusMobileContext(NATIVEwindow *windowHandle) :
  windowHandle(windowHandle),
  ovrState(nullptr),
  running(false),
  androidNativeWindow(nullptr),
  swapChainMetrics{0, 0},
  swapChainGl(nullptr),
  swapChainLength(0),
  swapChainIndex(0),
  fbo(0),
  msFbo(0),
  msColorTex(0),
  msDepthStencilTex(0),
  frameIndex(0),
  displayTime(0),
  swapInterval(1)
{
  {
    java.Vm = androidApp->activity->vm;
    java.Env = androidJniEnv;
    java.ActivityObject = androidApp->activity->clazz;

    const ovrInitParms initParms = vrapi_DefaultInitParms(&java);
    int32_t initResult = vrapi_Initialize(&initParms);
    if (initResult != VRAPI_INITIALIZE_SUCCESS) {
      exerr << "VRAPI failed to initialize: " << initResult << std::endl;
    }
  }

  {
    androidApp->userData = this;
    androidApp->onAppCmd = handleAppCmd;
  }
}

OculusMobileContext::~OculusMobileContext() {}

void OculusMobileContext::handleAppCmd(struct android_app *app, int32_t cmd) {
	OculusMobileContext *oculusMobileContext = (OculusMobileContext *)app->userData;

	switch (cmd) {
		// There is no APP_CMD_CREATE. The ANativeActivity creates the
		// application thread from onCreate(). The application thread
		// then calls android_main().
		case APP_CMD_START: {
      exerr << "APP_CMD_START" << std::endl;
			break;
		}
		case APP_CMD_RESUME: {
      exerr << "APP_CMD_RESUME" << std::endl;
      oculusMobileContext->running = true;
			break;
		}
		case APP_CMD_PAUSE: {
      exerr << "APP_CMD_PAUSE" << std::endl;
      oculusMobileContext->running = false;
			break;
		}
		case APP_CMD_STOP: {
      exerr << "APP_CMD_STOP" << std::endl;
			break;
		}
		case APP_CMD_DESTROY: {
      exerr << "APP_CMD_DESTROY" << std::endl;
      oculusMobileContext->androidNativeWindow = nullptr;
			break;
		}
		case APP_CMD_INIT_WINDOW: {
      exerr << "APP_CMD_INIT_WINDOW" << std::endl;
      oculusMobileContext->androidNativeWindow = app->window;
			break;
		}
		case APP_CMD_TERM_WINDOW: {
      exerr << "APP_CMD_TERM_WINDOW" << std::endl;
      oculusMobileContext->androidNativeWindow = nullptr;
			break;
		}
	}
}

Local<Function> OculusMobileContext::Initialize() {  
  Nan::EscapableHandleScope scope;

  // constructor
  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("OculusMobileContext"));

  // prototype
  Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
  Nan::SetMethod(proto, "RequestPresent", RequestPresent);
  Nan::SetMethod(proto, "WaitGetPoses", WaitGetPoses);
  Nan::SetMethod(proto, "Submit", Submit);
  Nan::SetMethod(proto, "GetRecommendedRenderTargetSize", GetRecommendedRenderTargetSize);

  Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();
  return scope.Escape(ctorFn);
}

NAN_METHOD(OculusMobileContext::New) {
  NATIVEwindow *windowHandle = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));

  Local<Object> oculusMobileContextObj = info.This();
  OculusMobileContext *oculusMobileContext = new OculusMobileContext(windowHandle);
  oculusMobileContext->Wrap(oculusMobileContextObj);

  /* Local<Function> audioDestinationNodeConstructor = Local<Function>::Cast(JS_OBJ(audioContextObj->Get(JS_STR("constructor")))->Get(JS_STR("AudioDestinationNode")));
  Local<Value> argv1[] = {
    audioContextObj,
  };
  Local<Object> audioDestinationNodeObj = audioDestinationNodeConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv1)/sizeof(argv1[0]), argv1).ToLocalChecked();
  audioContextObj->Set(JS_STR("destination"), audioDestinationNodeObj);

  Local<Function> audioListenerConstructor = Local<Function>::Cast(JS_OBJ(audioContextObj->Get(JS_STR("constructor")))->Get(JS_STR("AudioListener")));
  Local<Value> argv2[] = {
    audioContextObj,
  };
  Local<Object> audioListenerObj = audioListenerConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv2)/sizeof(argv2[0]), argv2).ToLocalChecked();
  audioContextObj->Set(JS_STR("listener"), audioListenerObj); */

  info.GetReturnValue().Set(oculusMobileContextObj);
}

void OculusMobileContext::RequestPresent() {
  OculusMobileContext *oculusMobileContext = this;

  while (oculusMobileContext->ovrState == nullptr) {
    oculusMobileContext->PollEvents(true);
  }
}

void OculusMobileContext::CreateSwapChain(WebGLRenderingContext *gl, int width, int height) {
  if (this->fbo != 0) {
    glDeleteFramebuffers(1, &this->fbo);
    this->fbo = 0;
    glDeleteFramebuffers(1, &this->msFbo);
    this->msFbo = 0;
    glDeleteTextures(1, &msColorTex);
    this->msColorTex = 0;
    glDeleteTextures(1, &msDepthStencilTex);
    this->msDepthStencilTex = 0;

    vrapi_DestroyTextureSwapChain(this->swapChain.Color);
    vrapi_DestroyTextureSwapChain(this->swapChain.Depth);
  }

  this->swapChain.Color = vrapi_CreateTextureSwapChain3(VRAPI_TEXTURE_TYPE_2D, GL_RGBA8, width, height, 1, 3);
  this->swapChain.Depth = vrapi_CreateTextureSwapChain3(VRAPI_TEXTURE_TYPE_2D, GL_DEPTH24_STENCIL8, width, height, 1, 3);
  this->swapChainMetrics[0] = width;
  this->swapChainMetrics[1] = height;
  this->swapChainGl = gl;
  this->swapChainLength = vrapi_GetTextureSwapChainLength(this->swapChain.Color);
  this->swapChainIndex = 0;

  for (int index = 0; index < this->swapChainLength; index++) {
    const GLuint colorTexture = vrapi_GetTextureSwapChainHandle(this->swapChain.Color, index);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const GLuint depthStencilTexture = vrapi_GetTextureSwapChainHandle(this->swapChain.Depth, index);
    glBindTexture(GL_TEXTURE_2D, depthStencilTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  glGenFramebuffers(1, &this->fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fbo);
  {
    GLuint colorTex = vrapi_GetTextureSwapChainHandle(this->swapChain.Color, this->swapChainIndex);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    GLuint depthStencilTex = vrapi_GetTextureSwapChainHandle(this->swapChain.Depth, this->swapChainIndex);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);
  }

  {
    glGenFramebuffers(1, &msFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, msFbo);

    glGenTextures(1, &msDepthStencilTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH24_STENCIL8, this->swapChainMetrics[0], this->swapChainMetrics[1], true);
    // glFramebufferTexture2DMultisampleEXT(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, msDepthStencilTex, 0, 4);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, msDepthStencilTex, 0);

    glGenTextures(1, &msColorTex);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msColorTex);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, this->swapChainMetrics[0], this->swapChainMetrics[1], true);
    // glFramebufferTexture2DMultisampleEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, msColorTex, 0, 4);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msColorTex, 0);
    
    glClear(GL_DEPTH_BUFFER_BIT); // initialize to far depth
  }

  if (this->swapChainGl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->swapChainGl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->swapChainGl->defaultFramebuffer);
  }
  if (this->swapChainGl->HasTextureBinding(this->swapChainGl->activeTexture, GL_TEXTURE_2D)) {
    glBindTexture(GL_TEXTURE_2D, this->swapChainGl->GetTextureBinding(this->swapChainGl->activeTexture, GL_TEXTURE_2D));
  } else {
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  if (this->swapChainGl->HasTextureBinding(this->swapChainGl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE)) {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->swapChainGl->GetTextureBinding(this->swapChainGl->activeTexture, GL_TEXTURE_2D_MULTISAMPLE));
  } else {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  }
}

void OculusMobileContext::PollEvents(bool wait) {
  OculusMobileContext *oculusMobileContext = this;

  // Read all pending events.
  for (;;) {
    int events;
    struct android_poll_source *source;
    if (ALooper_pollAll(wait ? -1 : 0, NULL, &events, (void **)&source ) < 0) { // timeout counts as an error
      break;
    }

    // Process this event.
    if (source) {
      source->process(androidApp, source);
    }

    bool shouldBeRunning = oculusMobileContext->running && oculusMobileContext->androidNativeWindow != nullptr;
    if (shouldBeRunning && oculusMobileContext->ovrState == nullptr) {
      ovrModeParms parms = vrapi_DefaultModeParms(&java);
      // No need to reset the FLAG_FULLSCREEN window flag when using a View
      parms.Flags &= ~VRAPI_MODE_FLAG_RESET_WINDOW_FULLSCREEN;

      parms.Flags |= VRAPI_MODE_FLAG_NATIVE_WINDOW;

      parms.WindowSurface = (unsigned long long)oculusMobileContext->androidNativeWindow;
      parms.Display = (unsigned long long)oculusMobileContext->windowHandle->display;
      parms.ShareContext = (unsigned long long)oculusMobileContext->windowHandle->context;

      oculusMobileContext->ovrState = vrapi_EnterVrMode(&parms);
      if (vrapi_SetTrackingSpace(oculusMobileContext->ovrState, VRAPI_TRACKING_SPACE_STAGE) < 0) {
        std::cerr << "failed to set tracking space" << std::endl;
      }

      if (oculusMobileContext->ovrState) {
        static const int CPU_LEVEL = 3;
        static const int GPU_LEVEL = 3;
        const float REFRESH_RATE = 72;
        pid_t tid = gettid();

        vrapi_SetClockLevels(oculusMobileContext->ovrState, CPU_LEVEL, GPU_LEVEL);
        vrapi_SetPerfThread(oculusMobileContext->ovrState, VRAPI_PERF_THREAD_TYPE_MAIN, tid);
        vrapi_SetPerfThread(oculusMobileContext->ovrState, VRAPI_PERF_THREAD_TYPE_RENDERER, tid);
        vrapi_SetDisplayRefreshRate(oculusMobileContext->ovrState, REFRESH_RATE);
      } else {
        // If entering VR mode failed then the ANativeWindow was not valid.
        oculusMobileContext->androidNativeWindow = nullptr;
      }

      /* static const int CPU_LEVEL = 2;
      static const int GPU_LEVEL = 3;
      static const int NUM_MULTI_SAMPLES = 4;
      // Set performance parameters once we have entered VR mode and have a valid ovrMobile.
      if (oculusMobileContext->ovrState) {
        vrapi_SetClockLevels( app->Ovr, app->CpuLevel, app->GpuLevel );
        vrapi_SetPerfThread( app->Ovr, VRAPI_PERF_THREAD_TYPE_MAIN, app->MainThreadTid );
        vrapi_SetPerfThread( app->Ovr, VRAPI_PERF_THREAD_TYPE_RENDERER, app->RenderThreadTid );
      } */
    } else if (!shouldBeRunning && oculusMobileContext->ovrState != nullptr) {
      vrapi_LeaveVrMode(oculusMobileContext->ovrState);
      oculusMobileContext->ovrState = nullptr;
    }

    if (wait) {
      break;
    }
  }
}

bool isQuest() {
  int	deviceType = vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_DEVICE_TYPE);
  return deviceType >= VRAPI_DEVICE_TYPE_OCULUSQUEST_START && deviceType <= VRAPI_DEVICE_TYPE_OCULUSQUEST_END;
}

NAN_METHOD(OculusMobileContext::RequestPresent) {
  OculusMobileContext *oculusMobileContext = ObjectWrap::Unwrap<OculusMobileContext>(info.This());
  oculusMobileContext->RequestPresent();
}

NAN_METHOD(OculusMobileContext::CreateSwapChain) {
  OculusMobileContext *oculusMobileContext = ObjectWrap::Unwrap<OculusMobileContext>(info.This());
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = TO_INT32(info[1]);
  int height = TO_INT32(info[2]);

  oculusMobileContext->CreateSwapChain(gl, width, height);
}

NAN_METHOD(OculusMobileContext::WaitGetPoses) {
  OculusMobileContext *oculusMobileContext = ObjectWrap::Unwrap<OculusMobileContext>(info.This());
  Local<Float32Array> float32Array = Local<Float32Array>::Cast(info[0]);
  float *float32ArrayData = (float *)((char *)float32Array->Buffer()->GetContents().Data() + float32Array->ByteOffset());

  int index = 0;
  float *positionVector = float32ArrayData + index;
  index += 3;
  float *orientationQuaternion = float32ArrayData + index;
  index += 4;
  float *eyeIpd = float32ArrayData + index;
  index += 1;
  float *eyeFov = float32ArrayData + index;
  index += 4;
  float *viewMatrixLeft = float32ArrayData + index;
  index += 16;
  float *viewMatrixRight = float32ArrayData + index;
  index += 16;
  float *projectionMatrixLeft = float32ArrayData + index;
  index += 16;
  float *projectionMatrixRight = float32ArrayData + index;
  index += 16;
  float *controllerMatrixLeft = float32ArrayData + index;
  index += 16;
  float *controllerStateLeft = float32ArrayData + index;
  index += 5;
  float *controllerMatrixRight = float32ArrayData + index;
  index += 16;
  float *controllerStateRight = float32ArrayData + index;
  index += 5;

  oculusMobileContext->PollEvents(false);

  bool hasPose = oculusMobileContext->ovrState != nullptr;
  if (hasPose) {
    oculusMobileContext->frameIndex++;
    const double predictedDisplayTime = vrapi_GetPredictedDisplayTime(oculusMobileContext->ovrState, oculusMobileContext->frameIndex);
    const ovrTracking2 &tracking = vrapi_GetPredictedTracking2(oculusMobileContext->ovrState, predictedDisplayTime);

    // headset
    {
      memcpy(positionVector, &tracking.HeadPose.Pose.Position, sizeof(float)*3);
      memcpy(orientationQuaternion, &tracking.HeadPose.Pose.Orientation, sizeof(float)*4);

      eyeIpd[0] = vrapi_GetInterpupillaryDistance(&tracking);

      ovrMatrix4f_ExtractFov(&tracking.Eye[0].ProjectionMatrix, &eyeFov[0], &eyeFov[1], &eyeFov[2], &eyeFov[3]);

      ovrMatrix4f eyeViewMatrixTransposed[2];
      eyeViewMatrixTransposed[0] = ovrMatrix4f_Transpose(&tracking.Eye[0].ViewMatrix);
      eyeViewMatrixTransposed[1] = ovrMatrix4f_Transpose(&tracking.Eye[1].ViewMatrix);
      memcpy(viewMatrixLeft, &eyeViewMatrixTransposed[0].M[0][0], sizeof(float)*16);
      memcpy(viewMatrixRight, &eyeViewMatrixTransposed[1].M[0][0], sizeof(float)*16);

      ovrMatrix4f projectionMatrixTransposed[2];
      projectionMatrixTransposed[0] = ovrMatrix4f_Transpose(&tracking.Eye[0].ProjectionMatrix);
      projectionMatrixTransposed[1] = ovrMatrix4f_Transpose(&tracking.Eye[1].ProjectionMatrix);
      memcpy(projectionMatrixLeft, &projectionMatrixTransposed[0].M[0][0], sizeof(float)*16);
      memcpy(projectionMatrixRight, &projectionMatrixTransposed[1].M[0][0], sizeof(float)*16);
    }

    // controllers
    controllerMatrixLeft[0] = std::numeric_limits<float>::quiet_NaN();
    controllerMatrixRight[0] = std::numeric_limits<float>::quiet_NaN();
    for (uint32_t index = 0;; index++) {
      ovrInputCapabilityHeader capsHeader;
      if (vrapi_EnumerateInputDevices(oculusMobileContext->ovrState, index, &capsHeader) >= 0) {
         if (capsHeader.Type == ovrControllerType_TrackedRemote) {
            ovrInputTrackedRemoteCapabilities remoteCaps;
            remoteCaps.Header = capsHeader;
            if (vrapi_GetInputDeviceCapabilities(oculusMobileContext->ovrState, &remoteCaps.Header) >= 0) {
              if (remoteCaps.ControllerCapabilities & ovrControllerCaps_LeftHand) {
                ovrTracking tracking;
                vrapi_GetInputTrackingState(oculusMobileContext->ovrState, capsHeader.DeviceID, predictedDisplayTime, &tracking);
                ovrVector3f pivot{-0.2, 1, 0};
                ovrVector3f point{-0.2, 1, -0.2};
                tracking.HeadPose.Pose.Position = ovrVector3f_RotateAboutPivot(&tracking.HeadPose.Pose.Orientation, &pivot, &point);
                // const ovrMatrix4f &matrix = vrapi_GetViewMatrixFromPose(&tracking.HeadPose.Pose);
                const ovrMatrix4f &matrix = vrapi_GetTransformFromPose(&tracking.HeadPose.Pose);
                const ovrMatrix4f &matrix2 = ovrMatrix4f_Transpose(&matrix);
                memcpy(controllerMatrixLeft, matrix2.M, sizeof(matrix2.M));

                ovrInputStateTrackedRemote remoteState;
                remoteState.Header.ControllerType = ovrControllerType_TrackedRemote;
                vrapi_GetCurrentInputState(oculusMobileContext->ovrState, capsHeader.DeviceID, &remoteState.Header);
                controllerStateLeft[0] = remoteState.IndexTrigger;
                controllerStateLeft[1] = remoteState.GripTrigger;
                controllerStateLeft[2] = remoteState.TrackpadPosition.x;
                controllerStateLeft[3] = remoteState.TrackpadPosition.y;
                controllerStateLeft[4] = (remoteState.Buttons & ovrButton_Enter) ? 1 : (remoteState.TrackpadStatus ? 0.5 : 0);
              } else if (remoteCaps.ControllerCapabilities & (ovrControllerCaps_RightHand|ovrControllerCaps_ModelOculusGo)) {
                ovrTracking tracking;
                vrapi_GetInputTrackingState(oculusMobileContext->ovrState, capsHeader.DeviceID, predictedDisplayTime, &tracking);
                ovrVector3f pivot{0.2, 1, 0};
                ovrVector3f point{0.2, 1, -0.2};
                tracking.HeadPose.Pose.Position = ovrVector3f_RotateAboutPivot(&tracking.HeadPose.Pose.Orientation, &pivot, &point);
                // const ovrMatrix4f &matrix = vrapi_GetViewMatrixFromPose(&tracking.HeadPose.Pose);
                const ovrMatrix4f &matrix = vrapi_GetTransformFromPose(&tracking.HeadPose.Pose);
                const ovrMatrix4f &matrix2 = ovrMatrix4f_Transpose(&matrix);
                memcpy(controllerMatrixRight, matrix2.M, sizeof(matrix2.M));

                ovrInputStateTrackedRemote remoteState;
                remoteState.Header.ControllerType = ovrControllerType_TrackedRemote;
                vrapi_GetCurrentInputState(oculusMobileContext->ovrState, capsHeader.DeviceID, &remoteState.Header);
                controllerStateRight[0] = remoteState.IndexTrigger;
                controllerStateRight[1] = remoteState.GripTrigger;
                controllerStateRight[2] = remoteState.TrackpadPosition.x;
                controllerStateRight[3] = remoteState.TrackpadPosition.y;
                controllerStateRight[4] = (remoteState.Buttons & ovrButton_Enter) ? 1 : (remoteState.TrackpadStatus ? 0.5 : 0);
              } else {
                // not a hand
              }
            } else {
              // not connected
            }
         }
      } else {
        break;
      }
    }

    // advance
    oculusMobileContext->tracking = tracking;
    oculusMobileContext->displayTime = predictedDisplayTime;
  }

  info.GetReturnValue().Set(JS_BOOL(hasPose));
}

NAN_METHOD(OculusMobileContext::GetRecommendedRenderTargetSize) {
  int width = vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH);
  int height = vrapi_GetSystemPropertyInt(&java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT);

  Local<Object> result = Nan::New<Object>();
  Nan::Set(result, JS_STR("width"), Nan::New<Integer>(width));
  Nan::Set(result, JS_STR("height"), Nan::New<Integer>(height));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(OculusMobileContext::Submit) {
  OculusMobileContext *oculusMobileContext = ObjectWrap::Unwrap<OculusMobileContext>(info.This());
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));

  ovrLayerProjection2 layer = vrapi_DefaultLayerProjection2();
  layer.HeadPose = oculusMobileContext->tracking.HeadPose;
  for (int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++) {
    layer.Textures[eye].ColorSwapChain = oculusMobileContext->swapChain.Color;
    layer.Textures[eye].SwapChainIndex = oculusMobileContext->swapChainIndex;
    {
      ovrRectf rect;
      if (eye == 0) {
        rect.x = 0;
        rect.y = 0;
        rect.width = oculusMobileContext->swapChainMetrics[0]/2;
        rect.height = oculusMobileContext->swapChainMetrics[1];
      } else {
        rect.x = oculusMobileContext->swapChainMetrics[0]/2;
        rect.y = 0;
        rect.width = oculusMobileContext->swapChainMetrics[0]/2;
        rect.height = oculusMobileContext->swapChainMetrics[1];
      }
      layer.Textures[eye].TextureRect = rect;
    }
    layer.Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection(&oculusMobileContext->tracking.Eye[eye].ProjectionMatrix);
	}
  layer.Header.Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

  const ovrLayerHeader2 *layers[] = {
    &layer.Header,
  };
  ovrSubmitFrameDescription2 frameDesc = {0};
  frameDesc.Flags = 0;
  frameDesc.SwapInterval = oculusMobileContext->swapInterval;
  frameDesc.FrameIndex = oculusMobileContext->frameIndex;
  frameDesc.DisplayTime = oculusMobileContext->displayTime;
  frameDesc.LayerCount = sizeof(layers)/sizeof(layers[0]);
  frameDesc.Layers = layers;

  // Hand over the eye images to the time warp.
  vrapi_SubmitFrame2(oculusMobileContext->ovrState, &frameDesc);
  oculusMobileContext->swapChainIndex = (oculusMobileContext->swapChainIndex + 1) % oculusMobileContext->swapChainLength;

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oculusMobileContext->fbo);
  {
    GLuint colorTex = vrapi_GetTextureSwapChainHandle(oculusMobileContext->swapChain.Color, oculusMobileContext->swapChainIndex);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    GLuint depthStencilTex = vrapi_GetTextureSwapChainHandle(oculusMobileContext->swapChain.Depth, oculusMobileContext->swapChainIndex);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);
  }

  if (oculusMobileContext->swapChainGl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, oculusMobileContext->swapChainGl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
  }
  if (oculusMobileContext->swapChainGl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oculusMobileContext->swapChainGl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  }
}

void OculusMobileContext::Destroy() {
  if (this->fbo != 0) {
    glDeleteFramebuffers(1, &this->fbo);
    glDeleteFramebuffers(1, &this->msFbo);
    glDeleteTextures(1, &msColorTex);
    glDeleteTextures(1, &msDepthStencilTex);

    vrapi_DestroyTextureSwapChain(this->swapChain.Color);
    vrapi_DestroyTextureSwapChain(this->swapChain.Depth);
  }
}

}

#endif
