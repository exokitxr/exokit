#include <glfw.h>

namespace glfw {

NAN_METHOD(GetVersion) {
  int major, minor, rev;
  glfwGetVersion(&major,&minor,&rev);
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("major"),JS_INT(major));
  result->Set(JS_STR("minor"),JS_INT(minor));
  result->Set(JS_STR("rev"),JS_INT(rev));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(GetVersionString) {
  const char *version = glfwGetVersionString();
  info.GetReturnValue().Set(JS_STR(version));
}

/* @Module: Time input */

NAN_METHOD(GetTime) {
  info.GetReturnValue().Set(JS_NUM(glfwGetTime()));
}

NAN_METHOD(SetTime) {
  double time = info[0]->NumberValue();
  glfwSetTime(time);
}

/* @Module: monitor handling */

/* TODO: Monitor configuration change callback */

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

/* @Module: Window handling */
GLFWwindow *currentWindow = nullptr;
int lastX = 0, lastY = 0; // XXX track this per-window
std::unique_ptr<Nan::Persistent<Function>> eventHandler;

void NAN_INLINE(CallEmitter(int argc, Local<Value> argv[])) {
  if (eventHandler && !(*eventHandler).IsEmpty()) {
    Local<Function> eventHandlerFn = Nan::New(*eventHandler);
    eventHandlerFn->Call(Nan::Null(), argc, argv);
  }
}

/* Window callbacks handling */
void APIENTRY windowPosCB(GLFWwindow *window, int xpos, int ypos) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("window_pos"));
  evt->Set(JS_STR("xpos"),JS_INT(xpos));
  evt->Set(JS_STR("ypos"),JS_INT(ypos));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("window_pos"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY windowSizeCB(GLFWwindow *window, int w, int h) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("resize"));
  evt->Set(JS_STR("width"),JS_INT(w));
  evt->Set(JS_STR("height"),JS_INT(h));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("windowResize"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY windowFramebufferSizeCB(GLFWwindow *window, int w, int h) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("framebuffer_resize"));
  evt->Set(JS_STR("width"),JS_INT(w));
  evt->Set(JS_STR("height"),JS_INT(h));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("framebufferResize"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY windowDropCB(GLFWwindow *window, int count, const char **paths) {
  Nan::HandleScope scope;

  Local<Array> pathsArray = Nan::New<Array>(count);
  for (int i = 0; i < count; i++) {
    pathsArray->Set(i, JS_STR(paths[i]));
  }
  
  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));
  evt->Set(JS_STR("paths"), pathsArray);

  Local<Value> argv[] = {
    JS_STR("drop"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY windowCloseCB(GLFWwindow *window) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("quit"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY windowRefreshCB(GLFWwindow *window) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("refresh"));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("refresh"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY windowIconifyCB(GLFWwindow *window, int iconified) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("iconified"));
  evt->Set(JS_STR("iconified"),JS_BOOL(iconified));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("iconified"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY windowFocusCB(GLFWwindow *window, int focused) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("focused"));
  evt->Set(JS_STR("focused"),JS_BOOL(focused));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("focus"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
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

void APIENTRY keyCB(GLFWwindow *window, int key, int scancode, int action, int mods) {
  const char *actionNames = "keyup\0  keydown\0keypress";

  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"), JS_STR(&actionNames[action << 3]));
  evt->Set(JS_STR("ctrlKey"),JS_BOOL(mods & GLFW_MOD_CONTROL));
  evt->Set(JS_STR("shiftKey"),JS_BOOL(mods & GLFW_MOD_SHIFT));
  evt->Set(JS_STR("altKey"),JS_BOOL(mods & GLFW_MOD_ALT));
  evt->Set(JS_STR("metaKey"),JS_BOOL(mods & GLFW_MOD_SUPER));

  int which = key;
  int charCode = key;
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
    case GLFW_KEY_SEMICOLON:    key = 186; break; // ;
    case GLFW_KEY_EQUAL:        key = 187; break; // =
    case GLFW_KEY_COMMA:        key = 188; break; // ,
    case GLFW_KEY_MINUS:        key = 189; break; // -
    case GLFW_KEY_PERIOD:       key = 190; break; // .
    case GLFW_KEY_SLASH:        key = 191; break; // /
    case GLFW_KEY_GRAVE_ACCENT: key = 192; break; // `
    case GLFW_KEY_LEFT_BRACKET: key = 219; break; // [
    case GLFW_KEY_BACKSLASH:    key = 220; break; /* \ */
    case GLFW_KEY_RIGHT_BRACKET: key = 221; break; // ]
    case GLFW_KEY_APOSTROPHE:   key = 222; break; // '
  }
  if (
    action == 2 && // keypress
    key >= 65 && // A
    key <= 90 // Z
  ) {
    key += 32;
  }
  evt->Set(JS_STR("which"),JS_INT(which));
  evt->Set(JS_STR("keyCode"),JS_INT(key));
  evt->Set(JS_STR("charCode"),JS_INT(charCode));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR(&actionNames[action << 3]), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);

  if (action == GLFW_PRESS) {
    keyCB(window, which, scancode, GLFW_REPEAT, mods);
  }
}

void APIENTRY cursorPosCB(GLFWwindow* window, double x, double y) {
  int w,h;
  glfwGetWindowSize(window, &w, &h);
  if(x<0 || x>=w) return;
  if(y<0 || y>=h) return;

  lastX=x;
  lastY=y;

  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("mousemove"));
  evt->Set(JS_STR("clientX"),JS_NUM(x));
  evt->Set(JS_STR("clientY"),JS_NUM(y));
  evt->Set(JS_STR("pageX"),JS_NUM(x));
  evt->Set(JS_STR("pageY"),JS_NUM(y));
  evt->Set(JS_STR("ctrlKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS));
  evt->Set(JS_STR("shiftKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS));
  evt->Set(JS_STR("altKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS));
  evt->Set(JS_STR("metaKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("mousemove"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY cursorEnterCB(GLFWwindow* window, int entered) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("mouseenter"));
  evt->Set(JS_STR("entered"),JS_INT(entered));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("mouseenter"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

void APIENTRY mouseButtonCB(GLFWwindow *window, int button, int action, int mods) {
  Nan::HandleScope scope;

  {
    Local<Object> evt = Nan::New<Object>();
    evt->Set(JS_STR("type"),JS_STR(action ? "mousedown" : "mouseup"));
    evt->Set(JS_STR("button"),JS_INT(button));
    evt->Set(JS_STR("which"),JS_INT(button));
    evt->Set(JS_STR("clientX"),JS_INT(lastX));
    evt->Set(JS_STR("clientY"),JS_INT(lastY));
    evt->Set(JS_STR("pageX"),JS_INT(lastX));
    evt->Set(JS_STR("pageY"),JS_INT(lastY));
    evt->Set(JS_STR("shiftKey"),JS_BOOL(mods & GLFW_MOD_SHIFT));
    evt->Set(JS_STR("ctrlKey"),JS_BOOL(mods & GLFW_MOD_CONTROL));
    evt->Set(JS_STR("altKey"),JS_BOOL(mods & GLFW_MOD_ALT));
    evt->Set(JS_STR("metaKey"),JS_BOOL(mods & GLFW_MOD_SUPER));
    evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR(action ? "mousedown" : "mouseup"), // event name
      evt
    };
    CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
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
    evt->Set(JS_STR("shiftKey"),JS_BOOL(mods & GLFW_MOD_SHIFT));
    evt->Set(JS_STR("ctrlKey"),JS_BOOL(mods & GLFW_MOD_CONTROL));
    evt->Set(JS_STR("altKey"),JS_BOOL(mods & GLFW_MOD_ALT));
    evt->Set(JS_STR("metaKey"),JS_BOOL(mods & GLFW_MOD_SUPER));
    evt->Set(JS_STR("windowHandle"), pointerToArray(window));

    Local<Value> argv[] = {
      JS_STR("click"), // event name
      evt,
    };
    CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
  }
}

void APIENTRY scrollCB(GLFWwindow *window, double xoffset, double yoffset) {
  Nan::HandleScope scope;

  Local<Object> evt = Nan::New<Object>();
  evt->Set(JS_STR("type"),JS_STR("wheel"));
  evt->Set(JS_STR("deltaX"),JS_NUM(-xoffset*120));
  evt->Set(JS_STR("deltaY"),JS_NUM(-yoffset*120));
  evt->Set(JS_STR("deltaZ"),JS_INT(0));
  evt->Set(JS_STR("deltaMode"),JS_INT(0));
  evt->Set(JS_STR("windowHandle"), pointerToArray(window));

  Local<Value> argv[] = {
    JS_STR("wheel"), // event name
    evt,
  };
  CallEmitter(sizeof(argv)/sizeof(argv[0]), argv);
}

/* NAN_METHOD(testJoystick) {
  Nan::HandleScope scope;

  int width = info[0]->Uint32Value();
  int height = info[1]->Uint32Value();
  float ratio = width / (float) height;

  float translateX = info[2]->NumberValue();
  float translateY = info[3]->NumberValue();
  float translateZ = info[4]->NumberValue();

  float rotateX = info[5]->NumberValue();
  float rotateY = info[6]->NumberValue();
  float rotateZ = info[7]->NumberValue();

  float angle = info[8]->NumberValue();

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  glRotatef(angle, rotateX, rotateY, rotateZ);
  glTranslatef(translateX, translateY, translateZ);

  glBegin(GL_TRIANGLES);
  glColor3f(1.f, 0.f, 0.f);
  glVertex3f(-0.6f, -0.4f, 0.f);
  glColor3f(0.f, 1.f, 0.f);
  glVertex3f(0.6f, -0.4f, 0.f);
  glColor3f(0.f, 0.f, 1.f);
  glVertex3f(0.f, 0.6f, 0.f);
  glEnd();
}

NAN_METHOD(testScene) {
  Nan::HandleScope scope;
  int width = info[0]->Uint32Value();
  int height = info[1]->Uint32Value();
  float z = info.Length()>2 ? (float) info[2]->NumberValue() : 0;
  float ratio = width / (float) height;

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);

  glBegin(GL_TRIANGLES);
  glColor3f(1.f, 0.f, 0.f);
  glVertex3f(-0.6f+z, -0.4f, 0.f);
  glColor3f(0.f, 1.f, 0.f);
  glVertex3f(0.6f+z, -0.4f, 0.f);
  glColor3f(0.f, 0.f, 1.f);
  glVertex3f(0.f+z, 0.6f, 0.f);
  glEnd();
}

NAN_METHOD(WindowHint) {
  int target       = info[0]->Uint32Value();
  int hint         = info[1]->Uint32Value();
  glfwWindowHint(target, hint);
}

NAN_METHOD(DefaultWindowHints) {
  glfwDefaultWindowHints();
}

NAN_METHOD(JoystickPresent) {
  int joy = info[0]->Uint32Value();
  bool isPresent = glfwJoystickPresent(joy);
  info.GetReturnValue().Set(JS_BOOL(isPresent));
} */

std::string intToString(int number) {
  std::ostringstream buff;
  buff << number;
  return buff.str();
}

std::string floatToString(float number){
    std::ostringstream buff;
    buff<<number;
    return buff.str();
}

std::string buttonToString(unsigned char c) {
  int number = (int)c;
  return intToString(number);
}

NAN_METHOD(GetJoystickAxes) {
  int joy = info[0]->Uint32Value();
  int count;
  const float *axisValues = glfwGetJoystickAxes(joy, &count);
  std::string response = "";
  for (int i = 0; i < count; i++) {
    response.append(floatToString(axisValues[i]));
    response.append(","); //Separator
  }

  info.GetReturnValue().Set(JS_STR(response.c_str()));
}

NAN_METHOD(GetJoystickButtons) {
  int joy = info[0]->Uint32Value();
  int count = 0;
  const unsigned char* response = glfwGetJoystickButtons(joy, &count);

  std::string strResponse = "";
  for (int i = 0; i < count; i++) {
    strResponse.append(buttonToString(response[i]));
    strResponse.append(",");
  }

  info.GetReturnValue().Set(JS_STR(strResponse.c_str()));
}

NAN_METHOD(GetJoystickName) {
  int joy = info[0]->Uint32Value();
  const char* response = glfwGetJoystickName(joy);
  info.GetReturnValue().Set(JS_STR(response));
}

/* NAN_METHOD(glfw_CreateWindow) {
  int width       = info[0]->Uint32Value();
  int height      = info[1]->Uint32Value();
  String::Utf8Value str(info[2]->ToString());
  int monitor_idx = info[3]->Uint32Value();

  GLFWwindow* window = NULL;
  GLFWmonitor **monitors = NULL, *monitor = NULL;
  int monitor_count;
  if(info.Length() >= 4 && monitor_idx >= 0){
    monitors = glfwGetMonitors(&monitor_count);
    if(monitor_idx >= monitor_count){
      return Nan::ThrowError("Invalid monitor");
    }
    monitor = monitors[monitor_idx];
  }

  window = glfwCreateWindow(width, height, *str, monitor, NULL);
  if(!window) {
    // can't create window, throw error
    return Nan::ThrowError("Can't create GLFW window");
  }

  glfwMakeContextCurrent(window);

  // make sure cursor is always shown
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  GLenum err = glewInit();
  if (err)
  {
    // Problem: glewInit failed, something is seriously wrong.
    std::string msg="Can't init GLEW (glew error ";
    msg+=(const char*) glewGetErrorString(err);
    msg+=")";

    fprintf(stderr, "%s", msg.c_str());
    return Nan::ThrowError(msg.c_str());
  }
  // fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

  // Set callback functions
  // glfw_events.Reset( info.This()->Get(JS_STR("events"))->ToObject());

  // window callbacks
  glfwSetWindowPosCallback( window, windowPosCB );
  glfwSetWindowSizeCallback( window, windowSizeCB );
  glfwSetWindowCloseCallback( window, windowCloseCB );
  glfwSetWindowRefreshCallback( window, windowRefreshCB );
  glfwSetWindowFocusCallback( window, windowFocusCB );
  glfwSetWindowIconifyCallback( window, windowIconifyCB );
  glfwSetFramebufferSizeCallback( window, windowFramebufferSizeCB );
  glfwSetDropCallback(window, windowDropCB);

  // input callbacks
  glfwSetKeyCallback( window, keyCB);
  // TODO glfwSetCharCallback(window, TwEventCharGLFW);
  glfwSetMouseButtonCallback( window, mouseButtonCB );
  glfwSetCursorPosCallback( window, cursorPosCB );
  glfwSetCursorEnterCallback( window, cursorEnterCB );
  glfwSetScrollCallback( window, scrollCB );

  info.GetReturnValue().Set(pointerToArray(window));
} */

NAN_METHOD(CreateRenderTarget) {
  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[0]));
  int width = info[1]->Uint32Value();
  int height = info[2]->Uint32Value();
  int samples = info[3]->Uint32Value();
  GLuint sharedColorTex = info[4]->Uint32Value();
  GLuint sharedDepthStencilTex = info[5]->Uint32Value();

  if (gl->activeTexture != WebGLRenderingContext::SystemTextureUnit) {
    glActiveTexture(WebGLRenderingContext::SystemTextureUnit);
  }

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  GLuint depthStencilTex = 0;
  if (!sharedDepthStencilTex) {
    GLuint renderBuffer;
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    if (samples > 1) {
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_STENCIL, width, height);
    } else {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
    }
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,	renderBuffer);
  } else {
    depthStencilTex = sharedDepthStencilTex;

    glBindTexture(GL_TEXTURE_2D, depthStencilTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencilTex, 0);
  }

  GLuint colorTex = 0;
  if (!sharedColorTex) {
    glGenTextures(1, &colorTex);
    if (samples > 1) {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorTex);
      // glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, true);
      // glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0, samples);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorTex, 0);
    } else {
      glBindTexture(GL_TEXTURE_2D, colorTex);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
    }
  } else {
    colorTex = sharedColorTex;

    glBindTexture(GL_TEXTURE_2D, colorTex);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
  }

  Local<Value> result;
  GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (framebufferStatus == GL_FRAMEBUFFER_COMPLETE) {
    Local<Array> array = Array::New(Isolate::GetCurrent(), 3);
    array->Set(0, JS_NUM(fbo));
    array->Set(1, JS_NUM(colorTex));
    array->Set(2, JS_NUM(depthStencilTex));
    result = array;
  } else {
    result = Null(Isolate::GetCurrent());
  }
  info.GetReturnValue().Set(result);

  if (gl->HasFramebufferBinding(GL_FRAMEBUFFER)) {
    glBindFramebuffer(GL_FRAMEBUFFER, gl->GetFramebufferBinding(GL_FRAMEBUFFER));
  }
  if (gl->activeTexture == WebGLRenderingContext::SystemTextureUnit) {
    if (gl->HasTextureBinding(GL_TEXTURE_2D)) {
      glBindTexture(GL_TEXTURE_2D, gl->GetTextureBinding(GL_TEXTURE_2D));
    }
    if (gl->HasTextureBinding(GL_TEXTURE_2D_MULTISAMPLE)) {
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, gl->GetTextureBinding(GL_TEXTURE_2D_MULTISAMPLE));
    }
    if (gl->HasTextureBinding(GL_TEXTURE_CUBE_MAP)) {
      glBindTexture(GL_TEXTURE_CUBE_MAP, gl->GetTextureBinding(GL_TEXTURE_CUBE_MAP));
    }
  } else {
    glActiveTexture(gl->activeTexture);
  }
}

NAN_METHOD(DestroyRenderTarget) {
  if (info[0]->IsNumber() && info[1]->IsNumber()) {
    GLuint fbo = info[0]->Uint32Value();
    GLuint tex = info[1]->Uint32Value();

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
  } else {
    Nan::ThrowError("invalid arguments");
  }
}

/* NAN_METHOD(CreateFramebuffer) {
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  info.GetReturnValue().Set(JS_NUM(fbo));
}

NAN_METHOD(FramebufferTextureLayer) {
  GLuint colorTex = info[0]->Uint32Value();
  GLuint depthTex = info[1]->Uint32Value();
  GLint layer = info[2]->Int32Value();

  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTex, 0, layer);
  glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0, layer);
} */

NAN_METHOD(BlitFrameBuffer) {
  Local<Object> glObj = Local<Object>::Cast(info[0]);
  GLuint fbo1 = info[1]->Uint32Value();
  GLuint fbo2 = info[2]->Uint32Value();
  int sw = info[3]->Uint32Value();
  int sh = info[4]->Uint32Value();
  int dw = info[5]->Uint32Value();
  int dh = info[6]->Uint32Value();
  bool color = info[7]->BooleanValue();
  bool depth = info[8]->BooleanValue();
  bool stencil = info[9]->BooleanValue();

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo1);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo2);

  glBlitFramebuffer(0, 0,
    sw, sh,
    0, 0,
    dw, dh,
    (color ? GL_COLOR_BUFFER_BIT : 0) |
    (depth ? GL_DEPTH_BUFFER_BIT : 0) |
    (stencil ? GL_STENCIL_BUFFER_BIT : 0),
    GL_LINEAR);

  WebGLRenderingContext *gl = ObjectWrap::Unwrap<WebGLRenderingContext>(glObj);
  if (gl->HasFramebufferBinding(GL_FRAMEBUFFER)) {
    glBindFramebuffer(GL_FRAMEBUFFER, gl->GetFramebufferBinding(GL_FRAMEBUFFER));
  }
}

void SetCurrentWindowContext(GLFWwindow *window) {
  if (currentWindow != window) {
    glfwMakeContextCurrent(window);
    currentWindow = window;
  }
}

NAN_METHOD(SetCurrentWindowContext) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  SetCurrentWindowContext(window);
}

NAN_METHOD(DestroyWindow) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwDestroyWindow(window);
}

NAN_METHOD(SetWindowTitle) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  String::Utf8Value str(info[1]->ToString());
  glfwSetWindowTitle(window, *str);
}

NAN_METHOD(GetWindowSize) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int w,h;
  glfwGetWindowSize(window, &w, &h);
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"),JS_INT(w));
  result->Set(JS_STR("height"),JS_INT(h));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(SetWindowSize) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwSetWindowSize(window, info[1]->Uint32Value(), info[2]->Uint32Value());
}

NAN_METHOD(SetWindowPos) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwSetWindowPos(window, info[1]->Uint32Value(),info[2]->Uint32Value());
}

NAN_METHOD(GetWindowPos) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int xpos, ypos;
  glfwGetWindowPos(window, &xpos, &ypos);
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("xpos"),JS_INT(xpos));
  result->Set(JS_STR("ypos"),JS_INT(ypos));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(GetFramebufferSize) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("width"),JS_INT(width));
  result->Set(JS_STR("height"),JS_INT(height));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(IconifyWindow) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwIconifyWindow(window);
}

NAN_METHOD(RestoreWindow) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwRestoreWindow(window);
}

NAN_METHOD(Show) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwShowWindow(window);
}

NAN_METHOD(Hide) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwHideWindow(window);
}

NAN_METHOD(IsVisible) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  bool visible = glfwGetWindowAttrib(window, GLFW_VISIBLE);
  info.GetReturnValue().Set(JS_BOOL(visible));
}

NAN_METHOD(WindowShouldClose) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  info.GetReturnValue().Set(JS_INT(glfwWindowShouldClose(window)));
}

NAN_METHOD(SetWindowShouldClose) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int value=info[1]->Uint32Value();
  glfwSetWindowShouldClose(window, value);
}

NAN_METHOD(GetWindowAttrib) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int attrib=info[1]->Uint32Value();
  info.GetReturnValue().Set(JS_INT(glfwGetWindowAttrib(window, attrib)));
}

NAN_METHOD(SetInputMode) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int mode = info[1]->Int32Value();
  int value = info[2]->Int32Value();
  glfwSetInputMode(window, mode, value);
}

NAN_METHOD(WaitEvents) {
  glfwWaitEvents();
}

/* Input handling */
NAN_METHOD(GetKey) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int key=info[1]->Uint32Value();
  info.GetReturnValue().Set(JS_INT(glfwGetKey(window, key)));
}

NAN_METHOD(GetMouseButton) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int button=info[1]->Uint32Value();
  info.GetReturnValue().Set(JS_INT(glfwGetMouseButton(window, button)));
}

NAN_METHOD(GetCursorPos) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  double x,y;
  glfwGetCursorPos(window, &x, &y);
  Local<Object> result = Nan::New<Object>();
  result->Set(JS_STR("x"),JS_NUM(x));
  result->Set(JS_STR("y"),JS_NUM(y));
  info.GetReturnValue().Set(result);
}

NAN_METHOD(SetCursorPos) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int x=info[1]->NumberValue();
  int y=info[2]->NumberValue();
  glfwSetCursorPos(window, x, y);
}

/* @Module Context handling */
/* NAN_METHOD(MakeContextCurrent) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwMakeContextCurrent(window);
}

NAN_METHOD(GetCurrentContext) {
  GLFWwindow* window=glfwGetCurrentContext();
  info.GetReturnValue().Set(JS_NUM((uint64_t) window));
}

NAN_METHOD(SwapInterval) {
  int interval=info[0]->Int32Value();
  glfwSwapInterval(interval);
} */

/* Extension support */
NAN_METHOD(ExtensionSupported) {
  String::Utf8Value str(info[0]->ToString());
  info.GetReturnValue().Set(JS_BOOL(glfwExtensionSupported(*str)==1));
}

bool glfwInitialized = false;
NAN_METHOD(Create) {
  if (!glfwInitialized) {
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

      glfwSetErrorCallback([](int err, const char *errString) {
        fprintf(stderr, "%s", errString);
      });

      glfwInitialized = true;
    } else {
      return Nan::ThrowError("failed to initialize GLFW");
    }
  }

  unsigned int width = info[0]->Uint32Value();
  unsigned int height = info[1]->Uint32Value();
  bool initialVisible = info[2]->BooleanValue();
  bool hidden = info[3]->BooleanValue();
  GLFWwindow *sharedWindow = info[4]->IsArray() ? (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[4])) : nullptr;
  WebGLRenderingContext *gl = info[5]->IsObject() ? ObjectWrap::Unwrap<WebGLRenderingContext>(Local<Object>::Cast(info[5])) : nullptr;

  glfwWindowHint(GLFW_VISIBLE, initialVisible);

  GLuint framebuffer = 0;
  GLuint framebufferTextures[] = {0, 0};
  bool shared = hidden && sharedWindow != nullptr && gl != nullptr;
  if (shared) {
    SetCurrentWindowContext(sharedWindow);

    glGenFramebuffers(1, &framebuffer);
    glGenTextures(sizeof(framebufferTextures)/sizeof(framebufferTextures[0]), framebufferTextures);
  }

  GLFWwindow *windowHandle = glfwCreateWindow(width, height, "Exokit", nullptr, shared ? sharedWindow : nullptr);

  if (windowHandle) {
    SetCurrentWindowContext(windowHandle);

    GLenum err = glewInit();
    if (!err) {
      glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

      // GLFW.SetWindowTitle(win, "WebGL");

      glfwSwapBuffers(windowHandle);
      glfwSwapInterval(0);

      /* int fbWidth, fbHeight;
      glfwGetFramebufferSize(windowHandle, &fbWidth, &fbHeight);

      int wWidth, wHeight;
      glfwGetWindowSize(windowHandle, &wWidth, &wHeight); */

      /* Local<Object> result = Object::New(Isolate::GetCurrent());
      result->Set(JS_STR("width"), JS_INT(fbWidth));
      result->Set(JS_STR("height"), JS_INT(fbHeight)); */

      // glfw_events.Reset( info.This()->Get(JS_STR("events"))->ToObject());

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

      GLuint vao;
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      Local<Array> result = Nan::New<Array>(3);
      result->Set(0, pointerToArray(windowHandle));
      result->Set(1, JS_NUM(framebuffer));
      result->Set(2, JS_NUM(framebufferTextures[0]));
      result->Set(3, JS_NUM(framebufferTextures[1]));
      info.GetReturnValue().Set(result);
    } else {
      /* Problem: glewInit failed, something is seriously wrong. */
      std::string msg = "Can't init GLEW (glew error ";
      msg += (const char *)glewGetErrorString(err);
      msg += ")";
      Nan::ThrowError(msg.c_str());
    }
  } else {
    // can't create window, throw error
    Nan::ThrowError("Can't create GLFW window");
  }
}

NAN_METHOD(Destroy) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwDestroyWindow(window);
}

NAN_METHOD(SetEventHandler) {
  if (!eventHandler) {
    eventHandler.reset(new Nan::Persistent<Function>());
  }
  (*eventHandler).Reset(Local<Function>::Cast(info[0]));
}

NAN_METHOD(PollEvents) {
  glfwPollEvents();
}

NAN_METHOD(SwapBuffers) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  glfwSwapBuffers(window);
}

NAN_METHOD(SetCursorMode) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  if (info[1]->BooleanValue()) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
}

NAN_METHOD(SetCursorPosition) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  int x = info[1]->Int32Value();
  int y = info[2]->Int32Value();
  glfwSetCursorPos(window, x, y);
}

NAN_METHOD(GetClipboard) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  const char *clipboardContents = glfwGetClipboardString(window);
  if (clipboardContents != nullptr) {
    info.GetReturnValue().Set(JS_STR(clipboardContents));
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(SetClipboard) {
  GLFWwindow *window = (GLFWwindow *)arrayToPointer(Local<Array>::Cast(info[0]));
  String::Utf8Value str(info[0]->ToString());
  glfwSetClipboardString(window, *str);
}

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
  Isolate *isolate = Isolate::GetCurrent();
  v8::EscapableHandleScope scope(isolate);

  Local<Object> target = Object::New(isolate);

  Nan::SetMethod(target, "create", glfw::Create);
  Nan::SetMethod(target, "destroy", glfw::Destroy);
  Nan::SetMethod(target, "show", glfw::Show);
  Nan::SetMethod(target, "hide", glfw::Hide);
  Nan::SetMethod(target, "isVisible", glfw::IsVisible);
  Nan::SetMethod(target, "setWindowTitle", glfw::SetWindowTitle);
  Nan::SetMethod(target, "getWindowSize", glfw::GetWindowSize);
  Nan::SetMethod(target, "setWindowSize", glfw::SetWindowSize);
  Nan::SetMethod(target, "setWindowPos", glfw::SetWindowPos);
  Nan::SetMethod(target, "getWindowPos", glfw::GetWindowPos);
  Nan::SetMethod(target, "getFramebufferSize", glfw::GetFramebufferSize);
  Nan::SetMethod(target, "iconifyWindow", glfw::IconifyWindow);
  Nan::SetMethod(target, "restoreWindow", glfw::RestoreWindow);
  Nan::SetMethod(target, "setEventHandler", glfw::SetEventHandler);
  Nan::SetMethod(target, "pollEvents", glfw::PollEvents);
  Nan::SetMethod(target, "swapBuffers", glfw::SwapBuffers);
  Nan::SetMethod(target, "setCursorMode", glfw::SetCursorMode);
  Nan::SetMethod(target, "setCursorPosition", glfw::SetCursorPosition);
  Nan::SetMethod(target, "getClipboard", glfw::GetClipboard);
  Nan::SetMethod(target, "setClipboard", glfw::SetClipboard);
  Nan::SetMethod(target, "createRenderTarget", glfw::CreateRenderTarget);
  Nan::SetMethod(target, "destroyRenderTarget", glfw::DestroyRenderTarget);
  // Nan::SetMethod(target, "createFramebuffer", glfw::CreateFramebuffer);
  // Nan::SetMethod(target, "framebufferTextureLayer", glfw::FramebufferTextureLayer);
  Nan::SetMethod(target, "blitFrameBuffer", glfw::BlitFrameBuffer);
  Nan::SetMethod(target, "setCurrentWindowContext", glfw::SetCurrentWindowContext);

  return scope.Escape(target);
}
