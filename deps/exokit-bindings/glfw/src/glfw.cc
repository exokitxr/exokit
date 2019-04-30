#include <glfw/include/glfw.h>
#include <windowsystem.h>
#include <webgl.h>

#include <exout>

namespace glfw {

thread_local NATIVEwindow *currentWindow = nullptr;
std::mutex windowHandleMutex;
NATIVEwindow *sharedWindow = nullptr;
std::map<NATIVEwindow *, EventHandler *> eventHandlerMap;
std::map<uv_async_t *, EventHandler *> eventHandlerMap2;
InjectionHandler mainThreadInjectionHandler;
std::mutex eventHandlerMapMutex;
std::mutex injectionHandlerMapMutex;
int lastX = 0, lastY = 0; // XXX track this per-window
#ifdef TARGET_OS_MAC
std::thread::id mainThreadId;
bool hasMainThreadId = false;
#endif

void RunEventInWindowThread(uv_async_t *async) {
  Nan::HandleScope scope;

  std::deque<std::function<void(std::function<void(int argc, Local<Value> *argv)>)>> localFns;
  Local<Function> handlerFn;
  {
    std::lock_guard<std::mutex> lock(eventHandlerMapMutex);

    EventHandler *handler = eventHandlerMap2[async];
    localFns = std::move(handler->fns);
    handler->fns.clear();

    handlerFn = Nan::New(handler->handlerFn);
  }
  for (auto iter = localFns.begin(); iter != localFns.end(); iter++) {
    Nan::HandleScope scope;

    (*iter)([&](int argc, Local<Value> *argv) -> void {
      Local<Object> asyncObject = Nan::New<Object>();
      AsyncResource asyncResource(Isolate::GetCurrent(), asyncObject, "mlEvents");

      asyncResource.MakeCallback(handlerFn, argc, argv);
    });
  }
}

EventHandler::EventHandler(uv_loop_t *loop, Local<Function> handlerFn) : async(new uv_async_t()), handlerFn(handlerFn) {
  uv_async_init(loop, async.get(), RunEventInWindowThread);
}

EventHandler::~EventHandler() {
  uv_close((uv_handle_t *)async.release(), [](uv_handle_t *handle) {
    delete handle;
  });
}

InjectionHandler::InjectionHandler() {}

void QueueEvent(NATIVEwindow *window, std::function<void(std::function<void(int, Local<Value> *)>)> fn) {
  EventHandler *eventHandler;
  {
    std::lock_guard<std::mutex> lock(eventHandlerMapMutex);

    auto iter = eventHandlerMap.find(window);
    if (iter != eventHandlerMap.end()) {
      eventHandler = iter->second;
      eventHandler->fns.push_back(fn);
    } else {
      eventHandler = nullptr;
    }
  }

  if (eventHandler) {
    uv_async_send(eventHandler->async.get());
  }
}

bool glfwInitialized = false;
void initializeGlfw() {
  glewExperimental = GL_TRUE;

  if (glfwInit() == GLFW_TRUE) {
    atexit([]() {
      glfwTerminate();
    });

    glfwDefaultWindowHints();

    // we use OpenGL 2.1, GLSL 1.20. Comment this for now as this is for GLSL 1.50
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    glfwWindowHint(GLFW_VISIBLE, 1);
    glfwWindowHint(GLFW_DECORATED, 1);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_REFRESH_RATE, 0);
    glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_NONE);

    glfwSetErrorCallback([](int err, const char *errString) {
      fprintf(stderr, "GLFW error: %d: %s", err, errString);
    });
  } else {
    exerr << "Failed to initialize GLFW" << std::endl;
    abort();
  }
}
void handleInjections() {
  std::lock_guard<std::mutex> lock(injectionHandlerMapMutex);

  for (auto iter = mainThreadInjectionHandler.fns.begin(); iter != mainThreadInjectionHandler.fns.end(); iter++) {
    std::function<void(InjectionHandler *)> &fn = *iter;
    fn(&mainThreadInjectionHandler);
  }
  mainThreadInjectionHandler.fns.clear();
}
void QueueInjection(NATIVEwindow *window, std::function<void(InjectionHandler *injectionHandler)> fn) {
  if (!glfwInitialized) {
#ifndef TARGET_OS_MAC
    std::thread([&]() -> void {
      initializeGlfw();

      for (;;) {
        glfwWaitEvents();

        handleInjections();
      }
    }).detach();
#else
    {
      std::lock_guard<std::mutex> lock(injectionHandlerMapMutex);

      mainThreadInjectionHandler.fns.push_back([](InjectionHandler *injectionHandler) -> void {
        initializeGlfw();
      });
    }
#endif

    glfwInitialized = true;
  }

  {
    std::lock_guard<std::mutex> lock(injectionHandlerMapMutex);

    mainThreadInjectionHandler.fns.push_back(fn);
  }

#ifndef TARGET_OS_MAC
  glfwPostEmptyEvent();
#else
  if (std::this_thread::get_id() == mainThreadId) {
    handleInjections();
  }
#endif
}

GLFWmonitor* _activeMonitor;
GLFWmonitor* getMonitor() {
  if (_activeMonitor) {
    return _activeMonitor;
  } else {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    return monitor;
  }
}

NAN_METHOD(GetMonitors) {
  int monitor_count, mode_count, xpos, ypos, width, height;
  int i, j;
  GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);
  GLFWmonitor *primary = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode, *modes;

  Local<Array> js_monitors = Nan::New<Array>(monitor_count);
  Local<Object> js_monitor, js_mode;
  Local<Array> js_modes;
  for(i=0; i<monitor_count; i++){
    js_monitor = Nan::New<Object>();
    js_monitor->Set(JS_STR("is_primary"), JS_BOOL(monitors[i] == primary));
    js_monitor->Set(JS_STR("index"), JS_INT(i));

    js_monitor->Set(JS_STR("name"), JS_STR(glfwGetMonitorName(monitors[i])));

    glfwGetMonitorPos(monitors[i], &xpos, &ypos);
    js_monitor->Set(JS_STR("pos_x"), JS_INT(xpos));
    js_monitor->Set(JS_STR("pos_y"), JS_INT(ypos));

    glfwGetMonitorPhysicalSize(monitors[i], &width, &height);
    js_monitor->Set(JS_STR("width_mm"), JS_INT(width));
    js_monitor->Set(JS_STR("height_mm"), JS_INT(height));

    mode = glfwGetVideoMode(monitors[i]);
    js_monitor->Set(JS_STR("width"), JS_INT(mode->width));
    js_monitor->Set(JS_STR("height"), JS_INT(mode->height));
    js_monitor->Set(JS_STR("rate"), JS_INT(mode->refreshRate));

    modes = glfwGetVideoModes(monitors[i], &mode_count);
    js_modes = Nan::New<Array>(mode_count);
    for(j=0; j<mode_count; j++){
      js_mode = Nan::New<Object>();
      js_mode->Set(JS_STR("width"), JS_INT(modes[j].width));
      js_mode->Set(JS_STR("height"), JS_INT(modes[j].height));
      js_mode->Set(JS_STR("rate"), JS_INT(modes[j].refreshRate));
      // NOTE: Are color bits necessary?
      js_modes->Set(JS_INT(j), js_mode);
    }
    js_monitor->Set(JS_STR("modes"), js_modes);

    js_monitors->Set(JS_INT(i), js_monitor);
  }

  info.GetReturnValue().Set(js_monitors);
}

NAN_METHOD(SetMonitor) {  
  int index = TO_INT32(info[0]);
  int monitor_count;
  GLFWmonitor **monitors = glfwGetMonitors(&monitor_count);
  _activeMonitor = monitors[index];
}

void GetScreenSize(NATIVEwindow *window, int *width, int *height) {
  uv_sem_t sem;
  uv_sem_init(&sem, 0);

  QueueInjection(window, [&](InjectionHandler *injectionHandler) -> void {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *videoMode = glfwGetVideoMode(monitor);
    *width = videoMode->width;
    *height = videoMode->height;

    uv_sem_post(&sem);
  });

  uv_sem_wait(&sem);
  uv_sem_destroy(&sem);
}

NAN_METHOD(GetScreenSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  
  int width, height;
  GetScreenSize(window, &width, &height);

  Local<Array> result = Nan::New<Array>(2);
  result->Set(0, JS_INT(width));
  result->Set(1, JS_INT(height));
  info.GetReturnValue().Set(result);
}

// Window callbacks handling
void APIENTRY windowPosCB(NATIVEwindow *window, int xpos, int ypos) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("window_pos"));
    evt->Set(JS_STR("xpos"),JS_INT(xpos));
    evt->Set(JS_STR("ypos"),JS_INT(ypos));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("window_pos"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

void APIENTRY windowSizeCB(NATIVEwindow *window, int w, int h) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("resize"));
    evt->Set(JS_STR("width"),JS_INT(w));
    evt->Set(JS_STR("height"),JS_INT(h));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("windowResize"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

void APIENTRY windowFramebufferSizeCB(NATIVEwindow *window, int w, int h) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("framebuffer_resize"));
    evt->Set(JS_STR("width"),JS_INT(w));
    evt->Set(JS_STR("height"),JS_INT(h));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("framebufferResize"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

void APIENTRY windowDropCB(NATIVEwindow *window, int count, const char **paths) {
  std::vector<char *> localPaths(count);
  for (int i = 0; i < count; i++) {
    const char *path = paths[i];
    size_t size = strlen(path) + 1;
    char *localPath = new char[size];
    memcpy(localPath, path, size);
    localPaths[i] = localPath;
  }

  QueueEvent(window, [localPaths{std::move(localPaths)}](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Array> pathsArray = Nan::New<Array>(localPaths.size());
    for (int i = 0; i < localPaths.size(); i++) {
      pathsArray->Set(i, JS_STR(localPaths[i]));
    }

    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("paths"), pathsArray);
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("drop"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);

    for (int i = 0; i < localPaths.size(); i++) {
      delete[] localPaths[i];
    }
  });
}

void APIENTRY windowCloseCB(NATIVEwindow *window) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("quit"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

void APIENTRY windowRefreshCB(NATIVEwindow *window) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("refresh"));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("refresh"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

void APIENTRY windowIconifyCB(NATIVEwindow *window, int iconified) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("iconified"));
    evt->Set(JS_STR("iconified"),JS_BOOL(iconified));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("iconified"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

void APIENTRY windowFocusCB(NATIVEwindow *window, int focused) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("focused"));
    evt->Set(JS_STR("focused"),JS_BOOL(focused));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("focus"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

static int jsKeyCode[]={
/*GLFW_KEY_ESCAPE*/       27,
/*GLFW_KEY_ENTER*/        13,
/*GLFW_KEY_TAB*/          9,
/*GLFW_KEY_BACKSPACE*/    8,
/*GLFW_KEY_INSERT*/       45,
/*GLFW_KEY_DELETE*/       46,
/*GLFW_KEY_RIGHT*/        39,
/*GLFW_KEY_LEFT*/         37,
/*GLFW_KEY_DOWN*/         40,
/*GLFW_KEY_UP*/           38,
/*GLFW_KEY_PAGE_UP*/      33,
/*GLFW_KEY_PAGE_DOWN*/    34,
/*GLFW_KEY_HOME*/         36,
/*GLFW_KEY_END*/          35,
/*GLFW_KEY_CAPS_LOCK*/    20,
/*GLFW_KEY_SCROLL_LOCK*/  145,
/*GLFW_KEY_NUM_LOCK*/     144,
/*GLFW_KEY_PRINT_SCREEN*/ 144, /* TODO */
/*GLFW_KEY_PAUSE*/        19,
/*GLFW_KEY_F1*/           112,
/*GLFW_KEY_F2*/           113,
/*GLFW_KEY_F3*/           114,
/*GLFW_KEY_F4*/           115,
/*GLFW_KEY_F5*/           116,
/*GLFW_KEY_F6*/           117,
/*GLFW_KEY_F7*/           118,
/*GLFW_KEY_F8*/           119,
/*GLFW_KEY_F9*/           120,
/*GLFW_KEY_F10*/          121,
/*GLFW_KEY_F11*/          122,
/*GLFW_KEY_F12*/          123,
/*GLFW_KEY_F13*/          123, /* unknown */
/*GLFW_KEY_F14*/          123, /* unknown */
/*GLFW_KEY_F15*/          123, /* unknown */
/*GLFW_KEY_F16*/          123, /* unknown */
/*GLFW_KEY_F17*/          123, /* unknown */
/*GLFW_KEY_F18*/          123, /* unknown */
/*GLFW_KEY_F19*/          123, /* unknown */
/*GLFW_KEY_F20*/          123, /* unknown */
/*GLFW_KEY_F21*/          123, /* unknown */
/*GLFW_KEY_F22*/          123, /* unknown */
/*GLFW_KEY_F23*/          123, /* unknown */
/*GLFW_KEY_F24*/          123, /* unknown */
/*GLFW_KEY_F25*/          123, /* unknown */
/*GLFW_KEY_KP_0*/         96,
/*GLFW_KEY_KP_1*/         97,
/*GLFW_KEY_KP_2*/         98,
/*GLFW_KEY_KP_3*/         99,
/*GLFW_KEY_KP_4*/         100,
/*GLFW_KEY_KP_5*/         101,
/*GLFW_KEY_KP_6*/         102,
/*GLFW_KEY_KP_7*/         103,
/*GLFW_KEY_KP_8*/         104,
/*GLFW_KEY_KP_9*/         105,
/*GLFW_KEY_KP_DECIMAL*/   110,
/*GLFW_KEY_KP_DIVIDE*/    111,
/*GLFW_KEY_KP_MULTIPLY*/  106,
/*GLFW_KEY_KP_SUBTRACT*/  109,
/*GLFW_KEY_KP_ADD*/       107,
/*GLFW_KEY_KP_ENTER*/     13,
/*GLFW_KEY_KP_EQUAL*/     187,
/*GLFW_KEY_LEFT_SHIFT*/   16,
/*GLFW_KEY_LEFT_CONTROL*/ 17,
/*GLFW_KEY_LEFT_ALT*/     18,
/*GLFW_KEY_LEFT_SUPER*/   91,
/*GLFW_KEY_RIGHT_SHIFT*/  16,
/*GLFW_KEY_RIGHT_CONTROL*/17,
/*GLFW_KEY_RIGHT_ALT*/    18,
/*GLFW_KEY_RIGHT_SUPER*/  93,
/*GLFW_KEY_MENU*/         18
};

const char *actionNames = "keyup\0  keydown\0keypress";
void APIENTRY keyCB(NATIVEwindow *window, int key, int scancode, int action, int mods) {
  if (key >= 0) { // media keys are -1
    bool isPrintable = true;
    switch (key) {
      case GLFW_KEY_ESCAPE:
      case GLFW_KEY_ENTER:
      case GLFW_KEY_TAB:
      case GLFW_KEY_BACKSPACE:
      case GLFW_KEY_INSERT:
      case GLFW_KEY_DELETE:
      case GLFW_KEY_RIGHT:
      case GLFW_KEY_LEFT:
      case GLFW_KEY_DOWN:
      case GLFW_KEY_UP:
      case GLFW_KEY_PAGE_UP:
      case GLFW_KEY_PAGE_DOWN:
      case GLFW_KEY_HOME:
      case GLFW_KEY_END:
      case GLFW_KEY_CAPS_LOCK:
      case GLFW_KEY_SCROLL_LOCK:
      case GLFW_KEY_NUM_LOCK:
      case GLFW_KEY_PRINT_SCREEN:
      case GLFW_KEY_PAUSE:
      case GLFW_KEY_F1:
      case GLFW_KEY_F2:
      case GLFW_KEY_F3:
      case GLFW_KEY_F4:
      case GLFW_KEY_F5:
      case GLFW_KEY_F6:
      case GLFW_KEY_F7:
      case GLFW_KEY_F8:
      case GLFW_KEY_F9:
      case GLFW_KEY_F10:
      case GLFW_KEY_F11:
      case GLFW_KEY_F12:
      case GLFW_KEY_F13:
      case GLFW_KEY_F14:
      case GLFW_KEY_F15:
      case GLFW_KEY_F16:
      case GLFW_KEY_F17:
      case GLFW_KEY_F18:
      case GLFW_KEY_F19:
      case GLFW_KEY_F20:
      case GLFW_KEY_F21:
      case GLFW_KEY_F22:
      case GLFW_KEY_F23:
      case GLFW_KEY_F24:
      case GLFW_KEY_F25:
      case GLFW_KEY_LEFT_SHIFT:
      case GLFW_KEY_LEFT_CONTROL:
      case GLFW_KEY_LEFT_ALT:
      case GLFW_KEY_LEFT_SUPER:
      case GLFW_KEY_RIGHT_SHIFT:
      case GLFW_KEY_RIGHT_CONTROL:
      case GLFW_KEY_RIGHT_ALT:
      case GLFW_KEY_RIGHT_SUPER:
      case GLFW_KEY_MENU:
        isPrintable = false;
    }

    if (!isPrintable && action == GLFW_REPEAT) {
      action = GLFW_PRESS;
    }

    int charCode = key;
    if (action == GLFW_RELEASE || action == GLFW_PRESS) {
      switch (key) {
        case GLFW_KEY_SLASH:        key = 191; break; // /
        case GLFW_KEY_GRAVE_ACCENT: key = 192; break; // `
        case GLFW_KEY_LEFT_BRACKET: key = 219; break; // [
        case GLFW_KEY_BACKSLASH:    key = 220; break; /* \ */
        case GLFW_KEY_RIGHT_BRACKET: key = 221; break; // ]
        case GLFW_KEY_APOSTROPHE:   key = 222; break; // '
        case GLFW_KEY_PERIOD:       key = 190; break; // '
        case GLFW_KEY_COMMA:        key = 188; break; // '
        case GLFW_KEY_SEMICOLON:    key = 186; break; // ;
        case GLFW_KEY_EQUAL:        key = 187; break; // =
        case GLFW_KEY_MINUS:        key = 189; break; // -
      }
    }
    switch (key) {
      case GLFW_KEY_ESCAPE:       key = 27; break;
      case GLFW_KEY_ENTER:        key = 13; break;
      case GLFW_KEY_TAB:          key = 9; break;
      case GLFW_KEY_BACKSPACE:    key = 8; break;
      case GLFW_KEY_INSERT:       key = 45; break;
      case GLFW_KEY_DELETE:       key = 46; break;
      case GLFW_KEY_RIGHT:        key = 39; break;
      case GLFW_KEY_LEFT:         key = 37; break;
      case GLFW_KEY_DOWN:         key = 40; break;
      case GLFW_KEY_UP:           key = 38; break;
      case GLFW_KEY_PAGE_UP:      key = 33; break;
      case GLFW_KEY_PAGE_DOWN:    key = 34; break;
      case GLFW_KEY_HOME:         key = 36; break;
      case GLFW_KEY_END:          key = 35; break;
      case GLFW_KEY_CAPS_LOCK:    key = 20; break;
      case GLFW_KEY_SCROLL_LOCK:  key = 145; break;
      case GLFW_KEY_NUM_LOCK:     key = 144; break;
      case GLFW_KEY_PRINT_SCREEN: key = 144; break; /* TODO */
      case GLFW_KEY_PAUSE:        key = 19; break;
      case GLFW_KEY_F1:           key = 112; break;
      case GLFW_KEY_F2:           key = 113; break;
      case GLFW_KEY_F3:           key = 114; break;
      case GLFW_KEY_F4:           key = 115; break;
      case GLFW_KEY_F5:           key = 116; break;
      case GLFW_KEY_F6:           key = 117; break;
      case GLFW_KEY_F7:           key = 118; break;
      case GLFW_KEY_F8:           key = 119; break;
      case GLFW_KEY_F9:           key = 120; break;
      case GLFW_KEY_F10:          key = 121; break;
      case GLFW_KEY_F11:          key = 122; break;
      case GLFW_KEY_F12:          key = 123; break;
      case GLFW_KEY_F13:          key = 123; break; /* unknown */
      case GLFW_KEY_F14:          key = 123; break; /* unknown */
      case GLFW_KEY_F15:          key = 123; break; /* unknown */
      case GLFW_KEY_F16:          key = 123; break; /* unknown */
      case GLFW_KEY_F17:          key = 123; break; /* unknown */
      case GLFW_KEY_F18:          key = 123; break; /* unknown */
      case GLFW_KEY_F19:          key = 123; break; /* unknown */
      case GLFW_KEY_F20:          key = 123; break; /* unknown */
      case GLFW_KEY_F21:          key = 123; break; /* unknown */
      case GLFW_KEY_F22:          key = 123; break; /* unknown */
      case GLFW_KEY_F23:          key = 123; break; /* unknown */
      case GLFW_KEY_F24:          key = 123; break; /* unknown */
      case GLFW_KEY_F25:          key = 123; break; /* unknown */
      case GLFW_KEY_KP_0:         key = 96; break;
      case GLFW_KEY_KP_1:         key = 97; break;
      case GLFW_KEY_KP_2:         key = 98; break;
      case GLFW_KEY_KP_3:         key = 99; break;
      case GLFW_KEY_KP_4:         key = 100; break;
      case GLFW_KEY_KP_5:         key = 101; break;
      case GLFW_KEY_KP_6:         key = 102; break;
      case GLFW_KEY_KP_7:         key = 103; break;
      case GLFW_KEY_KP_8:         key = 104; break;
      case GLFW_KEY_KP_9:         key = 105; break;
      case GLFW_KEY_KP_DECIMAL:   key = 110; break;
      case GLFW_KEY_KP_DIVIDE:    key = 111; break;
      case GLFW_KEY_KP_MULTIPLY:  key = 106; break;
      case GLFW_KEY_KP_SUBTRACT:  key = 109; break;
      case GLFW_KEY_KP_ADD:       key = 107; break;
      case GLFW_KEY_KP_ENTER:     key = 13; break;
      case GLFW_KEY_KP_EQUAL:     key = 187; break;
      case GLFW_KEY_LEFT_SHIFT:   key = 16; break;
      case GLFW_KEY_LEFT_CONTROL: key = 17; break;
      case GLFW_KEY_LEFT_ALT:     key = 18; break;
      case GLFW_KEY_LEFT_SUPER:   key = 91; break;
      case GLFW_KEY_RIGHT_SHIFT:  key = 16; break;
      case GLFW_KEY_RIGHT_CONTROL: key = 17; break;
      case GLFW_KEY_RIGHT_ALT:    key = 18; break;
      case GLFW_KEY_RIGHT_SUPER:  key = 93; break;
      case GLFW_KEY_MENU:         key = 18; break;
    }
    if (
      action == 2 && // keypress
      key >= 65 && // A
      key <= 90 // Z
    ) {
      key += 32;
    }

    int which = key;
    QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
      Local<Object> evt = Nan::New<Object>();
      evt->Set(JS_STR("type"), JS_STR(&actionNames[action << 3]));
      evt->Set(JS_STR("ctrlKey"), JS_BOOL(mods & GLFW_MOD_CONTROL));
      evt->Set(JS_STR("shiftKey"), JS_BOOL(mods & GLFW_MOD_SHIFT));
      evt->Set(JS_STR("altKey"), JS_BOOL(mods & GLFW_MOD_ALT));
      evt->Set(JS_STR("metaKey"), JS_BOOL(mods & GLFW_MOD_SUPER));
      evt->Set(JS_STR("which"), JS_INT(which));
      evt->Set(JS_STR("keyCode"), JS_INT(key));
      evt->Set(JS_STR("charCode"), JS_INT(charCode));
      // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

      Local<Value> argv[] = {
        JS_STR(&actionNames[action << 3]), // event name
        evt,
      };
      eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
    });

    if (action == GLFW_PRESS && isPrintable) {
      keyCB(window, charCode, scancode, GLFW_REPEAT, mods);
    }
  }
}

void APIENTRY cursorPosCB(NATIVEwindow* window, double x, double y) {
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  if(x<0 || x>=w) return;
  if(y<0 || y>=h) return;

  int mode = glfwGetInputMode(window, GLFW_CURSOR);

  int movementX, movementY;
  if (mode == GLFW_CURSOR_DISABLED) {
    movementX = x - (w / 2);
    movementY = y - (h / 2);

    glfwSetCursorPos(window, w / 2, h / 2);
  } else {
    movementX = 0;
    movementY = 0;
  }

  lastX = x;
  lastY = y;

  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("mousemove"));
    evt->Set(JS_STR("clientX"),JS_NUM(x));
    evt->Set(JS_STR("clientY"),JS_NUM(y));
    evt->Set(JS_STR("pageX"),JS_NUM(x));
    evt->Set(JS_STR("pageY"),JS_NUM(y));
    evt->Set(JS_STR("offsetX"),JS_NUM(x));
    evt->Set(JS_STR("offsetY"),JS_NUM(y));
    evt->Set(JS_STR("movementX"),JS_NUM(movementX));
    evt->Set(JS_STR("movementY"),JS_NUM(movementY));
    evt->Set(JS_STR("ctrlKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
    evt->Set(JS_STR("shiftKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
    evt->Set(JS_STR("altKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
    evt->Set(JS_STR("metaKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("mousemove"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

void APIENTRY cursorEnterCB(NATIVEwindow* window, int entered) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("mouseenter"));
    evt->Set(JS_STR("entered"),JS_INT(entered));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("mouseenter"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

void APIENTRY mouseButtonCB(NATIVEwindow *window, int button, int action, int mods) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    {
      Local<Object> evt = Nan::New<Object>();
      evt->Set(JS_STR("type"),JS_STR(action ? "mousedown" : "mouseup"));
      evt->Set(JS_STR("button"),JS_INT(button));
      evt->Set(JS_STR("which"),JS_INT(button));
      evt->Set(JS_STR("clientX"),JS_INT(lastX));
      evt->Set(JS_STR("clientY"),JS_INT(lastY));
      evt->Set(JS_STR("pageX"),JS_INT(lastX));
      evt->Set(JS_STR("pageY"),JS_INT(lastY));
      evt->Set(JS_STR("offsetX"),JS_INT(lastX));
      evt->Set(JS_STR("offsetY"),JS_INT(lastY));
      evt->Set(JS_STR("shiftKey"),JS_BOOL(mods & GLFW_MOD_SHIFT));
      evt->Set(JS_STR("ctrlKey"),JS_BOOL(mods & GLFW_MOD_CONTROL));
      evt->Set(JS_STR("altKey"),JS_BOOL(mods & GLFW_MOD_ALT));
      evt->Set(JS_STR("metaKey"),JS_BOOL(mods & GLFW_MOD_SUPER));
      // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

      Local<Value> argv[] = {
        JS_STR(action ? "mousedown" : "mouseup"), // event name
        evt
      };
      eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
    }

    if (!action) {
      Local<Object> evt = Nan::New<Object>();
      evt->Set(JS_STR("type"),JS_STR("click"));
      evt->Set(JS_STR("button"),JS_INT(button));
      evt->Set(JS_STR("which"),JS_INT(button));
      evt->Set(JS_STR("clientX"),JS_INT(lastX));
      evt->Set(JS_STR("clientY"),JS_INT(lastY));
      evt->Set(JS_STR("pageX"),JS_INT(lastX));
      evt->Set(JS_STR("pageY"),JS_INT(lastY));
      evt->Set(JS_STR("offsetX"),JS_INT(lastX));
      evt->Set(JS_STR("offsetY"),JS_INT(lastY));
      evt->Set(JS_STR("shiftKey"),JS_BOOL(mods & GLFW_MOD_SHIFT));
      evt->Set(JS_STR("ctrlKey"),JS_BOOL(mods & GLFW_MOD_CONTROL));
      evt->Set(JS_STR("altKey"),JS_BOOL(mods & GLFW_MOD_ALT));
      evt->Set(JS_STR("metaKey"),JS_BOOL(mods & GLFW_MOD_SUPER));
      // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

      Local<Value> argv[] = {
        JS_STR("click"), // event name
        evt,
      };
      eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
    }
  });
}

void APIENTRY scrollCB(NATIVEwindow *window, double xoffset, double yoffset) {
  QueueEvent(window, [=](std::function<void(int, Local<Value> *)> eventHandlerFn) -> void {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR("wheel"));
    evt->Set(JS_STR("deltaX"),JS_NUM(-xoffset*120));
    evt->Set(JS_STR("deltaY"),JS_NUM(-yoffset*120));
    evt->Set(JS_STR("deltaZ"),JS_INT(0));
    evt->Set(JS_STR("deltaMode"),JS_INT(0));
    // evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("wheel"), // event name
      evt,
    };
    eventHandlerFn(sizeof(argv)/sizeof(argv[0]), argv);
  });
}

NAN_METHOD(BlitFrameBuffer) {
  Local<Object> glObj = Local<Object>::Cast(info[0]);
  GLuint fbo1 = TO_UINT32(info[1]);
  GLuint fbo2 = TO_UINT32(info[2]);
  int sw = TO_UINT32(info[3]);
  int sh = TO_UINT32(info[4]);
  int dw = TO_UINT32(info[5]);
  int dh = TO_UINT32(info[6]);
  bool color = TO_BOOL(info[7]);
  bool depth = TO_BOOL(info[8]);
  bool stencil = TO_BOOL(info[9]);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo1);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);

  glBlitFramebuffer(
    0, 0,
    sw, sh,
    0, 0,
    dw, dh,
    (color ? GL_COLOR_BUFFER_BIT : 0) |
    (depth ? GL_DEPTH_BUFFER_BIT : 0) |
    (stencil ? GL_STENCIL_BUFFER_BIT : 0),
    (depth || stencil) ? GL_NEAREST : GL_LINEAR
  );

  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);
  if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->defaultFramebuffer);
  }
  if (gl->HasFramebufferBinding(GL_DRAW_FRAMEBUFFER)) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->GetFramebufferBinding(GL_DRAW_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->defaultFramebuffer);
  }
}

NATIVEwindow *GetCurrentWindowContext() {
  return currentWindow;
}

void SetCurrentWindowContext(NATIVEwindow *window) {
  if (currentWindow != window) {
    glfwMakeContextCurrent(window);
    currentWindow = window;
  }
}

void ReadPixels(WebGLRenderingContext *gl, unsigned int fbo, int x, int y, int width, int height, unsigned int format, unsigned int type, unsigned char *data) {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

  if (gl->HasFramebufferBinding(GL_READ_FRAMEBUFFER)) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->GetFramebufferBinding(GL_READ_FRAMEBUFFER));
  } else {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->defaultFramebuffer);
  }
}

NAN_METHOD(SetCurrentWindowContext) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  SetCurrentWindowContext(window);
}

NATIVEwindow *CreateNativeWindow(unsigned int width, unsigned int height, bool visible) {
  glfwWindowHint(GLFW_VISIBLE, visible);

  {
    std::lock_guard<std::mutex> lock(windowHandleMutex);

    if (!sharedWindow) {
      sharedWindow = glfwCreateWindow(1, 1, "Exokit", nullptr, nullptr);
    }
  }
  NATIVEwindow *window = glfwCreateWindow(width, height, "Exokit", nullptr, sharedWindow);
  if (!window) {
    exerr << "Can't create GLFW window" << std::endl;
    abort();
  }
  return window;
}
void DestroyNativeWindow(NATIVEwindow *window) {
  glfwDestroyWindow(window);
}

/* NAN_METHOD(DestroyWindow) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwDestroyWindow(window);

  if (currentWindow == window) {
    currentWindow = nullptr;
  }
} */

NAN_METHOD(SetWindowTitle) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  Nan::Utf8String str(Local<String>::Cast(info[1]));
  glfwSetWindowTitle(window, *str);
}

void GetWindowSize(NATIVEwindow *window, int *width, int *height) {
  glfwGetWindowSize(window, width, height);
}

NAN_METHOD(GetWindowSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int w, h;
  GetWindowSize(window, &w, &h);
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"),JS_INT(w));
  result->Set(JS_STR("height"),JS_INT(h));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(SetWindowSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwSetWindowSize(window, TO_UINT32(info[1]), TO_UINT32(info[2]));
}

NAN_METHOD(SetWindowPos) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwSetWindowPos(window, TO_UINT32(info[1]), TO_UINT32(info[2]));
}

NAN_METHOD(GetWindowPos) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int xpos, ypos;
  glfwGetWindowPos(window, &xpos, &ypos);
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("xpos"),JS_INT(xpos));
  result->Set(JS_STR("ypos"),JS_INT(ypos));
  info.GetReturnValue().Set(result);
}

void GetFramebufferSize(NATIVEwindow *window, int *width, int *height) {
  glfwGetFramebufferSize(window, width, height);
}

NAN_METHOD(GetFramebufferSize) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int width, height;
  GetFramebufferSize(window, &width, &height);
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"),JS_INT(width));
  result->Set(JS_STR("height"),JS_INT(height));
  info.GetReturnValue().Set(result);
}

double GetDevicePixelRatio(NATIVEwindow *window) {
  int width, height;

  uv_sem_t sem;
  uv_sem_init(&sem, 0);

  QueueInjection(window, [&](InjectionHandler *injectionHandler) -> void {
    NATIVEwindow *window = CreateNativeWindow(100, 100, false);
    glfwGetFramebufferSize(window, &width, &height);
    glfwDestroyWindow(window);

    uv_sem_post(&sem);
  });

  uv_sem_wait(&sem);
  uv_sem_destroy(&sem);

  return static_cast<double>(width)/100.0;
}

NAN_METHOD(GetDevicePixelRatio) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  double devicePixelRatio = GetDevicePixelRatio(window);
  info.GetReturnValue().Set(JS_NUM(devicePixelRatio));
}

NATIVEwindow *GetGLContext(NATIVEwindow *window) {
  return window;
}

NAN_METHOD(IconifyWindow) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwIconifyWindow(window);
}

NAN_METHOD(RestoreWindow) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwRestoreWindow(window);
}

NAN_METHOD(SetVisibility) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  bool visible = TO_BOOL(info[1]);

  QueueInjection(window, [window, visible](InjectionHandler *injectionHandler) -> void {
    if (visible) {
      glfwShowWindow(window);
    } else {
      glfwHideWindow(window);
    }
  });
}

NAN_METHOD(IsVisible) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  bool visible = glfwGetWindowAttrib(window, GLFW_VISIBLE);
  info.GetReturnValue().Set(JS_BOOL(visible));
}

const GLFWvidmode *getBestVidMode(NATIVEwindow *window, GLFWmonitor *monitor) {
  int numVidModes;
  const GLFWvidmode *vidModes = glfwGetVideoModes(monitor, &numVidModes);
  const GLFWvidmode *bestVidMode = nullptr;

  for (int i = 0; i < numVidModes; i++) {
    const GLFWvidmode *vidMode = &vidModes[i];
    if (bestVidMode == nullptr || vidMode->width > bestVidMode->width) {
      bestVidMode = vidMode;
    }
  }

  return bestVidMode;
}

NAN_METHOD(SetFullscreen) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  bool enabled = TO_BOOL(info[1]);

  QueueInjection(window, [window, enabled](InjectionHandler *injectionHandler) -> void {
    GLFWmonitor *monitor = getMonitor();

    if (enabled) {
      const GLFWvidmode *vidMode = getBestVidMode(window, monitor);
      if (vidMode != nullptr) {
        glfwSetWindowMonitor(window, monitor, 0, 0, vidMode->width, vidMode->height, 0);
      }
    } else {
      const GLFWvidmode *vidMode = getBestVidMode(window, monitor);
      glfwSetWindowMonitor(window, nullptr, vidMode->width/2 - 1280/2, vidMode->height/2 - 1024/2, 1280, 1024, 0);
    }
  });
}

NAN_METHOD(InitWindow3D) {
  NATIVEwindow *windowHandle = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[1]));

  SetCurrentWindowContext(windowHandle);

  windowsystembase::InitializeLocalGlState(gl);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

#ifdef GL_VERTEX_PROGRAM_POINT_SIZE
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif

#ifdef GL_PROGRAM_POINT_SIZE
  glEnable(GL_PROGRAM_POINT_SIZE);
#endif

  Local<Array> result = Nan::New<Array>(2);
  result->Set(0, pointerToArray(windowHandle));
  result->Set(1, JS_INT(vao));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(InitWindow2D) {
  NATIVEwindow *windowHandle = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));

  SetCurrentWindowContext(windowHandle);

  GLuint tex;
  glGenTextures(1, &tex);

  Local<Array> result = Nan::New<Array>(2);
  result->Set(0, pointerToArray(windowHandle));
  result->Set(1, JS_INT(tex));
  info.GetReturnValue().Set(result);
}

NATIVEwindow *CreateWindowHandle(unsigned int width, unsigned int height, bool initialVisible) {
  NATIVEwindow *windowHandle;

  uv_sem_t sem;
  uv_sem_init(&sem, 0);

  QueueInjection(nullptr, [&](InjectionHandler *injectionHandler) -> void {
    windowHandle = CreateNativeWindow(width, height, initialVisible);

    SetCurrentWindowContext(windowHandle);

    GLenum err = glewInit();
    if (!err) {
      // swap interval
      glfwSwapInterval(0);

      // input mode
      // glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

      // window callbacks
      glfwSetWindowPosCallback(windowHandle, windowPosCB);
      glfwSetWindowSizeCallback(windowHandle, windowSizeCB);
      glfwSetWindowCloseCallback(windowHandle, windowCloseCB);
      glfwSetWindowRefreshCallback(windowHandle, windowRefreshCB);
      glfwSetWindowFocusCallback(windowHandle, windowFocusCB);
      glfwSetWindowIconifyCallback(windowHandle, windowIconifyCB);
      glfwSetFramebufferSizeCallback(windowHandle, windowFramebufferSizeCB);
      glfwSetDropCallback(windowHandle, windowDropCB);

      // input callbacks
      glfwSetKeyCallback(windowHandle, keyCB);
      glfwSetMouseButtonCallback(windowHandle, mouseButtonCB);
      glfwSetCursorPosCallback(windowHandle, cursorPosCB);
      glfwSetCursorEnterCallback(windowHandle, cursorEnterCB);
      glfwSetScrollCallback(windowHandle, scrollCB);
    } else {
      /* Problem: glewInit failed, something is seriously wrong. */
      exerr << "Can't init GLEW (glew error " << (const char *)glewGetErrorString(err) << ")" << std::endl;

      DestroyNativeWindow(windowHandle);

      windowHandle = nullptr;
    }

    SetCurrentWindowContext(nullptr);

    uv_sem_post(&sem);
  });
  uv_sem_wait(&sem);
  uv_sem_destroy(&sem);
  
  return windowHandle;
}  

NAN_METHOD(CreateWindowHandle) {
  unsigned int width = info[0]->IsNumber() ? TO_UINT32(info[0]) : 1;
  unsigned int height = info[1]->IsNumber() ?  TO_UINT32(info[1]) : 1;
  bool initialVisible = TO_BOOL(info[2]);
  
  NATIVEwindow *windowHandle = CreateWindowHandle(width, height, initialVisible);

  if (windowHandle) {
    info.GetReturnValue().Set(pointerToArray(windowHandle));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(DestroyWindowHandle) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));

  uv_sem_t sem;
  uv_sem_init(&sem, 0);
  QueueInjection(window, [&](InjectionHandler *injectionHandler) -> void {
    DestroyNativeWindow(window);

    EventHandler *handler = eventHandlerMap[window];
    eventHandlerMap.erase(window);
    eventHandlerMap2.erase(handler->async.get());
    delete handler;
    
    uv_sem_post(&sem);
  });
  uv_sem_wait(&sem);
  uv_sem_destroy(&sem);
}

NAN_METHOD(SetEventHandler) {
  if (info[0]->IsArray() && info[1]->IsFunction()) {
    Local<Array> windowHandle = Local<Array>::Cast(info[0]);
    Local<Function> handlerFn = Local<Function>::Cast(info[1]);

    NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(windowHandle);
    {
      std::lock_guard<std::mutex> lock(eventHandlerMapMutex);

      uv_loop_t *loop = windowsystembase::GetEventLoop();
      EventHandler *handler = new EventHandler(loop, handlerFn);

      eventHandlerMap[window] = handler;
      eventHandlerMap2[handler->async.get()] = handler;
    }
  } else {
    Nan::ThrowError("SetEventHandler: invalid arguments");
  }
}

NAN_METHOD(SwapBuffers) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwSwapBuffers(window);
}

NAN_METHOD(GetRefreshRate) {
  int refreshRate;

  GLFWmonitor *monitor = getMonitor();
  if (monitor) {
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    refreshRate = mode->refreshRate;
  } else {
    refreshRate = 60;
  }

  info.GetReturnValue().Set(refreshRate);
}

NAN_METHOD(SetCursorMode) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  bool enabled = TO_BOOL(info[1]);

  QueueInjection(window, [window, enabled](InjectionHandler *injectionHandler) -> void {
    if (enabled) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

      int w, h;
      glfwGetWindowSize(window, &w, &h);

      int centerX = w/2;
      int centerY = h/2;
      glfwSetCursorPos(window, centerX, centerY);

      lastX = centerX;
      lastY = centerY;
    }
  });
}

NAN_METHOD(SetCursorPosition) {
  NATIVEwindow *window = (NATIVEwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int x = TO_INT32(info[1]);
  int y = TO_INT32(info[2]);
  glfwSetCursorPos(window, x, y);
}

NAN_METHOD(GetClipboard) {
  NATIVEwindow *window = GetCurrentWindowContext();
  const char *clipboardContents = glfwGetClipboardString(window);
  if (clipboardContents != nullptr) {
    info.GetReturnValue().Set(JS_STR(clipboardContents));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(SetClipboard) {
  if (info[0]->IsString()) {
    NATIVEwindow *window = GetCurrentWindowContext();
    Nan::Utf8String utf8_value(info[0]);
    glfwSetClipboardString(window, *utf8_value);
  } else {
    Nan::ThrowTypeError("Invalid arguments");
  }
}

#ifdef TARGET_OS_MAC
NAN_METHOD(PollEvents) {
  glfwPollEvents();

  handleInjections();
}
#endif

}

///////////////////////////////////////////////////////////////////////////////
//
// bindings
//
///////////////////////////////////////////////////////////////////////////////
#define JS_GLFW_CONSTANT(name) target->Set(JS_STR( #name ), JS_INT(GLFW_ ## name))
#define JS_GLFW_SET_METHOD(name) Nan::SetMethod(target, #name , glfw::name);

/* Local<Object> makeGlfw() {
  glfwInit();
  glewInit();
  atexit([]() {
    glfwTerminate();
  });

  Isolate *isolate = Isolate::GetCurrent();
  v8::EscapableHandleScope scope(isolate);

  Local<Object> target = Object::New(isolate);

  // GLFW initialization, termination and version querying
  JS_GLFW_SET_METHOD(GetVersion);
  JS_GLFW_SET_METHOD(GetVersionString);

  // Time
  JS_GLFW_SET_METHOD(GetTime);
  JS_GLFW_SET_METHOD(SetTime);

  // Monitor handling
  JS_GLFW_SET_METHOD(GetMonitors);

  // Window handling
  //JS_GLFW_SET_METHOD(CreateWindow);
  Nan::SetMethod(target, "CreateWindow", glfw::glfw_CreateWindow);
  Nan::SetMethod(target, "GetRenderTarget", glfw::GetRenderTarget);
  Nan::SetMethod(target, "BindFrameBuffer", glfw::BindFrameBuffer);
  Nan::SetMethod(target, "BlitFrameBuffer", glfw::BlitFrameBuffer);
  JS_GLFW_SET_METHOD(WindowHint);
  JS_GLFW_SET_METHOD(DefaultWindowHints);
  JS_GLFW_SET_METHOD(DestroyWindow);
  JS_GLFW_SET_METHOD(SetWindowShouldClose);
  JS_GLFW_SET_METHOD(WindowShouldClose);
  JS_GLFW_SET_METHOD(SetWindowTitle);
  JS_GLFW_SET_METHOD(GetWindowSize);
  JS_GLFW_SET_METHOD(SetWindowSize);
  JS_GLFW_SET_METHOD(SetWindowPos);
  JS_GLFW_SET_METHOD(GetWindowPos);
  JS_GLFW_SET_METHOD(GetFramebufferSize);
  JS_GLFW_SET_METHOD(IconifyWindow);
  JS_GLFW_SET_METHOD(RestoreWindow);
  JS_GLFW_SET_METHOD(ShowWindow);
  JS_GLFW_SET_METHOD(HideWindow);
  JS_GLFW_SET_METHOD(GetWindowAttrib);
  JS_GLFW_SET_METHOD(SetInputMode);
  JS_GLFW_SET_METHOD(PollEvents);
  JS_GLFW_SET_METHOD(WaitEvents);

  // Input handling
  JS_GLFW_SET_METHOD(GetKey);
  JS_GLFW_SET_METHOD(GetMouseButton);
  JS_GLFW_SET_METHOD(GetCursorPos);
  JS_GLFW_SET_METHOD(SetCursorPos);

  // Context handling
  JS_GLFW_SET_METHOD(MakeContextCurrent);
  JS_GLFW_SET_METHOD(GetCurrentContext);
  JS_GLFW_SET_METHOD(SwapBuffers);
  JS_GLFW_SET_METHOD(SwapInterval);
  JS_GLFW_SET_METHOD(ExtensionSupported);

  // Joystick
  JS_GLFW_SET_METHOD(JoystickPresent);
  JS_GLFW_SET_METHOD(GetJoystickAxes);
  JS_GLFW_SET_METHOD(GetJoystickButtons);
  JS_GLFW_SET_METHOD(GetJoystickName);

  // GLFW version

  JS_GLFW_CONSTANT(VERSION_MAJOR);
  JS_GLFW_CONSTANT(VERSION_MINOR);
  JS_GLFW_CONSTANT(VERSION_REVISION);

  // Input handling definitions

  // Key and button state/action definitions
  JS_GLFW_CONSTANT(RELEASE);
  JS_GLFW_CONSTANT(PRESS);
  JS_GLFW_CONSTANT(REPEAT);

  // These key codes are inspired by the *USB HID Usage Tables v1.12* (p. 53-60),
  // but re-arranged to map to 7-bit ASCII for printable keys (function keys are
  // put in the 256+ range).
  //
  // The naming of the key codes follow these rules:
  //  - The US keyboard layout is used
  //  - Names of printable alpha-numeric characters are used (e.g. "A", "R",
  //    "3", etc.)
  //  - For non-alphanumeric characters, Unicode:ish names are used (e.g.
  //    "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
  //    correspond to the Unicode standard (usually for brevity)
  //  - Keys that lack a clear US mapping are named "WORLD_x"
  //  - For non-printable keys, custom names are used (e.g. "F4",
  //    "BACKSPACE", etc.)

  // The unknown key
  JS_GLFW_CONSTANT(KEY_UNKNOWN);

  // Printable keys
  JS_GLFW_CONSTANT(KEY_SPACE);
  JS_GLFW_CONSTANT(KEY_APOSTROPHE);
  JS_GLFW_CONSTANT(KEY_COMMA);
  JS_GLFW_CONSTANT(KEY_MINUS);
  JS_GLFW_CONSTANT(KEY_PERIOD);
  JS_GLFW_CONSTANT(KEY_SLASH);
  JS_GLFW_CONSTANT(KEY_0);
  JS_GLFW_CONSTANT(KEY_1);
  JS_GLFW_CONSTANT(KEY_2);
  JS_GLFW_CONSTANT(KEY_3);
  JS_GLFW_CONSTANT(KEY_4);
  JS_GLFW_CONSTANT(KEY_5);
  JS_GLFW_CONSTANT(KEY_6);
  JS_GLFW_CONSTANT(KEY_7);
  JS_GLFW_CONSTANT(KEY_8);
  JS_GLFW_CONSTANT(KEY_9);
  JS_GLFW_CONSTANT(KEY_SEMICOLON);
  JS_GLFW_CONSTANT(KEY_EQUAL);
  JS_GLFW_CONSTANT(KEY_A);
  JS_GLFW_CONSTANT(KEY_B);
  JS_GLFW_CONSTANT(KEY_C);
  JS_GLFW_CONSTANT(KEY_D);
  JS_GLFW_CONSTANT(KEY_E);
  JS_GLFW_CONSTANT(KEY_F);
  JS_GLFW_CONSTANT(KEY_G);
  JS_GLFW_CONSTANT(KEY_H);
  JS_GLFW_CONSTANT(KEY_I);
  JS_GLFW_CONSTANT(KEY_J);
  JS_GLFW_CONSTANT(KEY_K);
  JS_GLFW_CONSTANT(KEY_L);
  JS_GLFW_CONSTANT(KEY_M);
  JS_GLFW_CONSTANT(KEY_N);
  JS_GLFW_CONSTANT(KEY_O);
  JS_GLFW_CONSTANT(KEY_P);
  JS_GLFW_CONSTANT(KEY_Q);
  JS_GLFW_CONSTANT(KEY_R);
  JS_GLFW_CONSTANT(KEY_S);
  JS_GLFW_CONSTANT(KEY_T);
  JS_GLFW_CONSTANT(KEY_U);
  JS_GLFW_CONSTANT(KEY_V);
  JS_GLFW_CONSTANT(KEY_W);
  JS_GLFW_CONSTANT(KEY_X);
  JS_GLFW_CONSTANT(KEY_Y);
  JS_GLFW_CONSTANT(KEY_Z);
  JS_GLFW_CONSTANT(KEY_LEFT_BRACKET);
  JS_GLFW_CONSTANT(KEY_BACKSLASH);
  JS_GLFW_CONSTANT(KEY_RIGHT_BRACKET);
  JS_GLFW_CONSTANT(KEY_GRAVE_ACCENT);
  JS_GLFW_CONSTANT(KEY_WORLD_1);
  JS_GLFW_CONSTANT(KEY_WORLD_2);

  // Function keys
  JS_GLFW_CONSTANT(KEY_ESCAPE);
  JS_GLFW_CONSTANT(KEY_ENTER);
  JS_GLFW_CONSTANT(KEY_TAB);
  JS_GLFW_CONSTANT(KEY_BACKSPACE);
  JS_GLFW_CONSTANT(KEY_INSERT);
  JS_GLFW_CONSTANT(KEY_DELETE);
  JS_GLFW_CONSTANT(KEY_RIGHT);
  JS_GLFW_CONSTANT(KEY_LEFT);
  JS_GLFW_CONSTANT(KEY_DOWN);
  JS_GLFW_CONSTANT(KEY_UP);
  JS_GLFW_CONSTANT(KEY_PAGE_UP);
  JS_GLFW_CONSTANT(KEY_PAGE_DOWN);
  JS_GLFW_CONSTANT(KEY_HOME);
  JS_GLFW_CONSTANT(KEY_END);
  JS_GLFW_CONSTANT(KEY_CAPS_LOCK);
  JS_GLFW_CONSTANT(KEY_SCROLL_LOCK);
  JS_GLFW_CONSTANT(KEY_NUM_LOCK);
  JS_GLFW_CONSTANT(KEY_PRINT_SCREEN);
  JS_GLFW_CONSTANT(KEY_PAUSE);
  JS_GLFW_CONSTANT(KEY_F1);
  JS_GLFW_CONSTANT(KEY_F2);
  JS_GLFW_CONSTANT(KEY_F3);
  JS_GLFW_CONSTANT(KEY_F4);
  JS_GLFW_CONSTANT(KEY_F5);
  JS_GLFW_CONSTANT(KEY_F6);
  JS_GLFW_CONSTANT(KEY_F7);
  JS_GLFW_CONSTANT(KEY_F8);
  JS_GLFW_CONSTANT(KEY_F9);
  JS_GLFW_CONSTANT(KEY_F10);
  JS_GLFW_CONSTANT(KEY_F11);
  JS_GLFW_CONSTANT(KEY_F12);
  JS_GLFW_CONSTANT(KEY_F13);
  JS_GLFW_CONSTANT(KEY_F14);
  JS_GLFW_CONSTANT(KEY_F15);
  JS_GLFW_CONSTANT(KEY_F16);
  JS_GLFW_CONSTANT(KEY_F17);
  JS_GLFW_CONSTANT(KEY_F18);
  JS_GLFW_CONSTANT(KEY_F19);
  JS_GLFW_CONSTANT(KEY_F20);
  JS_GLFW_CONSTANT(KEY_F21);
  JS_GLFW_CONSTANT(KEY_F22);
  JS_GLFW_CONSTANT(KEY_F23);
  JS_GLFW_CONSTANT(KEY_F24);
  JS_GLFW_CONSTANT(KEY_F25);
  JS_GLFW_CONSTANT(KEY_KP_0);
  JS_GLFW_CONSTANT(KEY_KP_1);
  JS_GLFW_CONSTANT(KEY_KP_2);
  JS_GLFW_CONSTANT(KEY_KP_3);
  JS_GLFW_CONSTANT(KEY_KP_4);
  JS_GLFW_CONSTANT(KEY_KP_5);
  JS_GLFW_CONSTANT(KEY_KP_6);
  JS_GLFW_CONSTANT(KEY_KP_7);
  JS_GLFW_CONSTANT(KEY_KP_8);
  JS_GLFW_CONSTANT(KEY_KP_9);
  JS_GLFW_CONSTANT(KEY_KP_DECIMAL);
  JS_GLFW_CONSTANT(KEY_KP_DIVIDE);
  JS_GLFW_CONSTANT(KEY_KP_MULTIPLY);
  JS_GLFW_CONSTANT(KEY_KP_SUBTRACT);
  JS_GLFW_CONSTANT(KEY_KP_ADD);
  JS_GLFW_CONSTANT(KEY_KP_ENTER);
  JS_GLFW_CONSTANT(KEY_KP_EQUAL);
  JS_GLFW_CONSTANT(KEY_LEFT_SHIFT);
  JS_GLFW_CONSTANT(KEY_LEFT_CONTROL);
  JS_GLFW_CONSTANT(KEY_LEFT_ALT);
  JS_GLFW_CONSTANT(KEY_LEFT_SUPER);
  JS_GLFW_CONSTANT(KEY_RIGHT_SHIFT);
  JS_GLFW_CONSTANT(KEY_RIGHT_CONTROL);
  JS_GLFW_CONSTANT(KEY_RIGHT_ALT);
  JS_GLFW_CONSTANT(KEY_RIGHT_SUPER);
  JS_GLFW_CONSTANT(KEY_MENU);
  JS_GLFW_CONSTANT(KEY_LAST);

  // Modifier key flags

  // If this bit is set one or more Shift keys were held down.
  JS_GLFW_CONSTANT(MOD_SHIFT);
  // If this bit is set one or more Control keys were held down.
  JS_GLFW_CONSTANT(MOD_CONTROL);
  // If this bit is set one or more Alt keys were held down.
  JS_GLFW_CONSTANT(MOD_ALT);
  // If this bit is set one or more Super keys were held down.
  JS_GLFW_CONSTANT(MOD_SUPER);

  // Mouse buttons
  JS_GLFW_CONSTANT(MOUSE_BUTTON_1);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_2);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_3);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_4);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_5);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_6);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_7);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_8);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_LAST);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_LEFT);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_RIGHT);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_MIDDLE);

  // Joysticks
  JS_GLFW_CONSTANT(JOYSTICK_1);
  JS_GLFW_CONSTANT(JOYSTICK_2);
  JS_GLFW_CONSTANT(JOYSTICK_3);
  JS_GLFW_CONSTANT(JOYSTICK_4);
  JS_GLFW_CONSTANT(JOYSTICK_5);
  JS_GLFW_CONSTANT(JOYSTICK_6);
  JS_GLFW_CONSTANT(JOYSTICK_7);
  JS_GLFW_CONSTANT(JOYSTICK_8);
  JS_GLFW_CONSTANT(JOYSTICK_9);
  JS_GLFW_CONSTANT(JOYSTICK_10);
  JS_GLFW_CONSTANT(JOYSTICK_11);
  JS_GLFW_CONSTANT(JOYSTICK_12);
  JS_GLFW_CONSTANT(JOYSTICK_13);
  JS_GLFW_CONSTANT(JOYSTICK_14);
  JS_GLFW_CONSTANT(JOYSTICK_15);
  JS_GLFW_CONSTANT(JOYSTICK_16);
  JS_GLFW_CONSTANT(JOYSTICK_LAST);

  // errors Error codes

  // GLFW has not been initialized.
  JS_GLFW_CONSTANT(NOT_INITIALIZED);
  // No context is current for this thread.
  JS_GLFW_CONSTANT(NO_CURRENT_CONTEXT);
  // One of the enum parameters for the function was given an invalid enum.
  JS_GLFW_CONSTANT(INVALID_ENUM);
  // One of the parameters for the function was given an invalid value.
  JS_GLFW_CONSTANT(INVALID_VALUE);
  // A memory allocation failed.
  JS_GLFW_CONSTANT(OUT_OF_MEMORY);
  // GLFW could not find support for the requested client API on the system.
  JS_GLFW_CONSTANT(API_UNAVAILABLE);
  // The requested client API version is not available.
  JS_GLFW_CONSTANT(VERSION_UNAVAILABLE);
  // A platform-specific error occurred that does not match any of the more specific categories.
  JS_GLFW_CONSTANT(PLATFORM_ERROR);
  // The clipboard did not contain data in the requested format.
  JS_GLFW_CONSTANT(FORMAT_UNAVAILABLE);

  JS_GLFW_CONSTANT(FOCUSED);
  JS_GLFW_CONSTANT(ICONIFIED);
  JS_GLFW_CONSTANT(RESIZABLE);
  JS_GLFW_CONSTANT(VISIBLE);
  JS_GLFW_CONSTANT(DECORATED);

  JS_GLFW_CONSTANT(RED_BITS);
  JS_GLFW_CONSTANT(GREEN_BITS);
  JS_GLFW_CONSTANT(BLUE_BITS);
  JS_GLFW_CONSTANT(ALPHA_BITS);
  JS_GLFW_CONSTANT(DEPTH_BITS);
  JS_GLFW_CONSTANT(STENCIL_BITS);
  JS_GLFW_CONSTANT(ACCUM_RED_BITS);
  JS_GLFW_CONSTANT(ACCUM_GREEN_BITS);
  JS_GLFW_CONSTANT(ACCUM_BLUE_BITS);
  JS_GLFW_CONSTANT(ACCUM_ALPHA_BITS);
  JS_GLFW_CONSTANT(AUX_BUFFERS);
  JS_GLFW_CONSTANT(STEREO);
  JS_GLFW_CONSTANT(SAMPLES);
  JS_GLFW_CONSTANT(SRGB_CAPABLE);
  JS_GLFW_CONSTANT(REFRESH_RATE);
  JS_GLFW_CONSTANT(DOUBLEBUFFER);
  JS_GLFW_CONSTANT(TRUE);
  JS_GLFW_CONSTANT(FALSE);

  JS_GLFW_CONSTANT(CLIENT_API);
  JS_GLFW_CONSTANT(CONTEXT_VERSION_MAJOR);
  JS_GLFW_CONSTANT(CONTEXT_VERSION_MINOR);
  JS_GLFW_CONSTANT(CONTEXT_REVISION);
  JS_GLFW_CONSTANT(CONTEXT_ROBUSTNESS);
  JS_GLFW_CONSTANT(OPENGL_FORWARD_COMPAT);
  JS_GLFW_CONSTANT(OPENGL_DEBUG_CONTEXT);
  JS_GLFW_CONSTANT(OPENGL_PROFILE);

  JS_GLFW_CONSTANT(OPENGL_API);
  JS_GLFW_CONSTANT(OPENGL_ES_API);

  JS_GLFW_CONSTANT(NO_ROBUSTNESS);
  JS_GLFW_CONSTANT(NO_RESET_NOTIFICATION);
  JS_GLFW_CONSTANT(LOSE_CONTEXT_ON_RESET);

  JS_GLFW_CONSTANT(OPENGL_ANY_PROFILE);
  JS_GLFW_CONSTANT(OPENGL_CORE_PROFILE);
  JS_GLFW_CONSTANT(OPENGL_COMPAT_PROFILE);

  JS_GLFW_CONSTANT(CURSOR);
  JS_GLFW_CONSTANT(STICKY_KEYS);
  JS_GLFW_CONSTANT(STICKY_MOUSE_BUTTONS);

  JS_GLFW_CONSTANT(CURSOR_NORMAL);
  JS_GLFW_CONSTANT(CURSOR_HIDDEN);
  JS_GLFW_CONSTANT(CURSOR_DISABLED);

  JS_GLFW_CONSTANT(CONNECTED);
  JS_GLFW_CONSTANT(DISCONNECTED);

  // test scene
  JS_GLFW_SET_METHOD(testScene);
  JS_GLFW_SET_METHOD(testJoystick);

  return scope.Escape(target);
} */

Local<Object> makeWindow() {
#ifdef TARGET_OS_MAC
  if (!glfw::hasMainThreadId) {
    glfw::mainThreadId = std::this_thread::get_id();
    glfw::hasMainThreadId = true;
  }
#endif

  Isolate *isolate = Isolate::GetCurrent();
  v8::EscapableHandleScope scope(isolate);

  Local<Object> target = Object::New(isolate);

  windowsystembase::Decorate(target);

  Nan::SetMethod(target, "initWindow3D", glfw::InitWindow3D);
  Nan::SetMethod(target, "initWindow2D", glfw::InitWindow2D);

  Nan::SetMethod(target, "createWindowHandle", glfw::CreateWindowHandle);
  Nan::SetMethod(target, "destroyWindowHandle", glfw::DestroyWindowHandle);
  Nan::SetMethod(target, "setVisibility", glfw::SetVisibility);
  Nan::SetMethod(target, "isVisible", glfw::IsVisible);
  Nan::SetMethod(target, "setFullscreen", glfw::SetFullscreen);
  Nan::SetMethod(target, "getMonitors", glfw::GetMonitors);
  Nan::SetMethod(target, "setMonitor", glfw::SetMonitor);
  Nan::SetMethod(target, "getScreenSize", glfw::GetScreenSize);
  Nan::SetMethod(target, "setWindowTitle", glfw::SetWindowTitle);
  Nan::SetMethod(target, "getWindowSize", glfw::GetWindowSize);
  Nan::SetMethod(target, "setWindowSize", glfw::SetWindowSize);
  Nan::SetMethod(target, "setWindowPos", glfw::SetWindowPos);
  Nan::SetMethod(target, "getWindowPos", glfw::GetWindowPos);
  Nan::SetMethod(target, "getFramebufferSize", glfw::GetFramebufferSize);
  Nan::SetMethod(target, "getDevicePixelRatio", glfw::GetDevicePixelRatio);
  Nan::SetMethod(target, "iconifyWindow", glfw::IconifyWindow);
  Nan::SetMethod(target, "restoreWindow", glfw::RestoreWindow);
  Nan::SetMethod(target, "setEventHandler", glfw::SetEventHandler);
  Nan::SetMethod(target, "swapBuffers", glfw::SwapBuffers);
  Nan::SetMethod(target, "getRefreshRate", glfw::GetRefreshRate);
  Nan::SetMethod(target, "setCursorMode", glfw::SetCursorMode);
  Nan::SetMethod(target, "setCursorPosition", glfw::SetCursorPosition);
  Nan::SetMethod(target, "getClipboard", glfw::GetClipboard);
  Nan::SetMethod(target, "setClipboard", glfw::SetClipboard);
  Nan::SetMethod(target, "blitFrameBuffer", glfw::BlitFrameBuffer);
  Nan::SetMethod(target, "setCurrentWindowContext", glfw::SetCurrentWindowContext);
#ifdef TARGET_OS_MAC
  Nan::SetMethod(target, "pollEvents", glfw::PollEvents);
#endif

  return scope.Escape(target);
}
