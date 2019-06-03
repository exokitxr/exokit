#include <unistd.h>
#include <iostream>
#include <string>
// #include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <v8.h>
#include <exout>
#ifdef ANDROID
#include <android/log.h>
#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#endif
#ifdef LUMIN
#include <ml_logging.h>
#include <ml_lifecycle.h>
#include <ml_privileges.h>
#endif

using namespace v8;

namespace node {
  extern std::map<std::string, std::pair<void *, bool>> dlibs;
  int Start(int argc, char* argv[]);
}

#include "build/libexokit/dlibs.h"

std::vector<const char *> getDefaultArguments() {
  const char *launchUrl;
  if (access("/package/app/index.html", F_OK) != -1) {
    launchUrl = "/package/app/index.html";
  } else {
    launchUrl = "/package/examples/realitytabs.html";
  }

  std::vector<const char *> result = {
    "node",
#ifndef ANDROID
    ".",
#else
    "/package",
#endif
    launchUrl,
  };
  return result;
}

#ifdef ANDROID

struct android_app *androidApp;
JNIEnv *androidJniEnv;
// std::vector<std::string> androidArgs;

typedef struct AssetStatStruct {
  char name[256];
  uint32_t key;
  uint32_t type;
  uint32_t parentKey;
  size_t size;
} AssetStat;

extern "C" {
void initAssetManager(AAssetManager *am);
}

JNIEnv *jniGetEnv(JavaVM *vm) {
  JNIEnv *env;
  if (vm == 0) {
    __android_log_print(ANDROID_LOG_ERROR, "exokit", "Invalid global Java VM");
    return 0;
  }

  int status;
  status = vm->GetEnv((void **) &env, JNI_VERSION_1_6);
  if (status < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "exokit", "Failed to get JNI environment, trying to attach thread");
    // Try to attach native thread to JVM:
    status = vm->AttachCurrentThread(&env, 0);
    if (status < 0) {
      __android_log_print(ANDROID_LOG_ERROR, "exokit", "Failed to attach current thread to JVM");
      return 0;
    }
  }

  return env;
}
jobject jniGetContext(JNIEnv *env) {

  jclass activityThread = env->FindClass("android/app/ActivityThread");
  jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread",
                                                           "()Landroid/app/ActivityThread;");
  jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);

  jmethodID getApplication = env->GetMethodID(activityThread, "getApplication",
                                              "()Landroid/app/Application;");
  jobject context = env->CallObjectMethod(at, getApplication);
  return context;
}


void jniOnload(JavaVM *vm) {
  __android_log_print(ANDROID_LOG_INFO, "exokit", "on load 1");

  // Uses Java VM and application context to obtain Java asset manager
  // and then uses NDK AAssetManager_fromJava()
  // Note: In Android emulator, this seems to only work in main thread!
  JNIEnv *env = jniGetEnv(vm);
  __android_log_print(ANDROID_LOG_ERROR, "exokit", "on load 2 %lx", (unsigned long) env);
  jobject context = jniGetContext(env);
  __android_log_print(ANDROID_LOG_ERROR, "exokit", "on load 3 %lx", (unsigned long) context);
  jmethodID methodGetAssets =
      env->GetMethodID(env->GetObjectClass(context),
                       "getAssets",
                       "()Landroid/content/res/AssetManager;");
  if (methodGetAssets == 0) {
    __android_log_print(ANDROID_LOG_ERROR, "exokit", "Could not get getAssets method");
    return;
  }
  jobject localAssetManager =
      env->CallObjectMethod(context,
                            methodGetAssets);
  if (localAssetManager == 0) {
    __android_log_print(ANDROID_LOG_ERROR, "exokit", "Could not get local Java Asset Manager");
    return;
  }
  jobject globalAssetManager = env->NewGlobalRef(localAssetManager);
  if (globalAssetManager == 0) {
    __android_log_print(ANDROID_LOG_ERROR, "exokit", "Could not get global Java Asset Manager");
    return;
  }

  AAssetManager *am = AAssetManager_fromJava(env, globalAssetManager);
  __android_log_print(ANDROID_LOG_INFO, "exokit", "Got Java Asset Manager 1 %lx %lx", (unsigned long)am, (unsigned long)initAssetManager);
  initAssetManager(am);
  __android_log_print(ANDROID_LOG_INFO, "exokit", "Got Java Asset Manager 2 %lx %lx", (unsigned long)am, (unsigned long)initAssetManager);

  {
    jclass exokitWebViewClass = env->FindClass("com/webmr/ExokitWebView");
    // jmethodID constructor = env->GetMethodID(exokitWebViewClass, "<init>", "(Landroid/content/Context;)V");
    // jobject exokitWebView = env->NewObject(exokitWebViewClass, constructor, context);
    jmethodID makeFnId = env->GetStaticMethodID(exokitWebViewClass, "make", "(Landroid/app/Activity;Landroid/content/Context;I)Lcom/webmr/ExokitWebView;");
    jint colorTex = 0;
    jobject exokitWebView = env->CallStaticObjectMethod(exokitWebViewClass, makeFnId, androidApp->activity->clazz, context, colorTex);
    (void)exokitWebView; // XXX
  }

  androidJniEnv = env;
  __android_log_print(ANDROID_LOG_INFO, "exokit", "Got JNI Env %lx", (unsigned long)androidJniEnv);

  {
    jobject activity = androidApp->activity->clazz;
    jmethodID get_intent_method = env->GetMethodID(env->GetObjectClass(activity), "getIntent", "()Landroid/content/Intent;");
    jobject intent = env->CallObjectMethod(activity, get_intent_method);
    jmethodID get_string_extra_method = env->GetMethodID(env->GetObjectClass(intent), "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");
    jvalue get_string_extra_args;
    get_string_extra_args.l = env->NewStringUTF("ARGS");
    jstring extra_str = static_cast<jstring>(env->CallObjectMethodA(intent, get_string_extra_method, &get_string_extra_args));

    if (extra_str) {
      const char* extra_utf = env->GetStringUTFChars(extra_str, nullptr);
      exout << "Set ARGS env " << extra_utf << std::endl;
      int setenvResult = setenv("ARGS", extra_utf, 1);
      if (setenvResult != 0) {
        exout << "failed to set ARGS env " << setenvResult << " " << errno << std::endl;
      }
      env->ReleaseStringUTFChars(extra_str, extra_utf);
      env->DeleteLocalRef(extra_str);
    }

    env->DeleteLocalRef(get_string_extra_args.l);
    env->DeleteLocalRef(intent);
  }
}

#define LOG_INFO(f, s) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, (f), (s))
#define LOG_ERROR(f, s) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, (f), (s))

#endif

#ifdef LUMIN

#define LOG_INFO(f, s) ML_LOG_TAG(Info, LOG_TAG, (f), (s))
#define LOG_ERROR(f, s) ML_LOG_TAG(Error, LOG_TAG, (f), (s))

#endif

const char *LOG_TAG = "exokit";
constexpr ssize_t STDIO_BUF_SIZE = 64 * 1024;

void pumpStdout(int fd) {
  char buf[STDIO_BUF_SIZE + 1];
  ssize_t i = 0;
  ssize_t lineStart = 0;

  for (;;) {
    ssize_t size = read(fd, buf + i, sizeof(buf) - i);

    if (size > 0) {
      for (ssize_t j = i; j < i + size; j++) {
        if (buf[j] == '\n') {
          buf[j] = 0;
          LOG_INFO("%s", buf + lineStart);

          lineStart = j + 1;
        }
      }

      i += size;

      if (i >= sizeof(buf)) {
        ssize_t lineLength = i - lineStart;
        memcpy(buf, buf + lineStart, lineLength);
        i = lineLength;
        lineStart = 0;
      }
    } else {
      if (i > 0) {
        buf[i] = 0;
        LOG_INFO("%s", buf);
      }
      break;
    }
  }
}
void pumpStderr(int fd) {
  char buf[STDIO_BUF_SIZE + 1];
  ssize_t i = 0;
  ssize_t lineStart = 0;

  for (;;) {
    ssize_t size = read(fd, buf + i, sizeof(buf) - i);

    if (size > 0) {
      for (ssize_t j = i; j < i + size; j++) {
        if (buf[j] == '\n') {
          buf[j] = 0;
          LOG_ERROR("%s", buf + lineStart);

          lineStart = j + 1;
        }
      }

      i += size;

      if (i >= sizeof(buf)) {
        ssize_t lineLength = i - lineStart;
        memcpy(buf, buf + lineStart, lineLength);
        i = lineLength;
        lineStart = 0;
      }
    } else {
      if (i > 0) {
        buf[i] = 0;
        LOG_ERROR("%s", buf);
      }
      break;
    }
  }
}

#ifdef ANDROID
void android_main(struct android_app *app) {
#endif
#ifdef LUMIN
int main(int argc, char **argv) {
#endif

  /* // exec self
  if (argc > 1) {
    int result = node::Start(argc, argv);
#ifdef ANDROID
    return;
#endif
#ifdef LUMIN
    return result;
#endif
  } */

  // pipe stdio
  std::thread stdoutReaderThread;
  std::thread stderrReaderThread;
  {
    int stdoutfds[2];
    int stderrfds[2];
    pipe(stdoutfds);
    pipe(stderrfds);

    // if (stdoutfds[1] != 1) {
    close(1);
    dup2(stdoutfds[1], 1);
    // }
    // if (stderrfds[1] != 2) {
    close(2);
    dup2(stderrfds[1], 2);
    // }

    // read stdout/stderr in threads
    int stdoutReadFd = stdoutfds[0];
    stdoutReaderThread = std::thread([stdoutReadFd]() -> void { pumpStdout(stdoutReadFd); });
    int stderrReadFd = stderrfds[0];
    stderrReaderThread = std::thread([stderrReadFd]() -> void { pumpStderr(stderrReadFd); });
  }

  // platform startup
#ifdef LUMIN
  {
    MLResult result = MLPrivilegesStartup();
    if (result != MLResult_Ok) {
      exout << "failed to start privilege system " << result << std::endl;
    }
    const MLPrivilegeID privileges[] = {
      MLPrivilegeID_LowLatencyLightwear,
      MLPrivilegeID_WorldReconstruction,
      MLPrivilegeID_Occlusion,
      MLPrivilegeID_ControllerPose,
      MLPrivilegeID_CameraCapture,
      MLPrivilegeID_AudioCaptureMic,
      MLPrivilegeID_VoiceInput,
      MLPrivilegeID_AudioRecognizer,
      MLPrivilegeID_Internet,
      MLPrivilegeID_LocalAreaNetwork,
      MLPrivilegeID_BackgroundDownload,
      MLPrivilegeID_BackgroundUpload,
      MLPrivilegeID_PwFoundObjRead,
      MLPrivilegeID_NormalNotificationsUsage,
    };
    const size_t numPrivileges = sizeof(privileges) / sizeof(privileges[0]);
    for (size_t i = 0; i < numPrivileges; i++) {
      const MLPrivilegeID &privilege = privileges[i];
      MLResult result = MLPrivilegesCheckPrivilege(privilege);
      if (result != MLPrivilegesResult_Granted) {
        const char *s = MLPrivilegesGetResultString(result);
        exout << "did not have privilege " << privilege << " " << result << " " << s << std::endl;

        MLResult result = MLPrivilegesRequestPrivilege(privilege);
        if (result != MLPrivilegesResult_Granted) {
          const char *s = MLPrivilegesGetResultString(result);
          exout << "failed to get privilege " << privilege << " " << result << " " << s << std::endl;
        }
      }
    }
  }
#endif
#ifdef ANDROID
  {
    int setenvResult = setenv("HOME", app->activity->internalDataPath, 1);
    if (setenvResult != 0) {
      exout << "failed to set HOME env " << setenvResult << " " << errno << " " << std::endl;
    }

    androidApp = app;
    jniOnload(app->activity->vm);
  }
#endif

  // node startup
  exout << "---------------------exokit start" << std::endl;

  std::atexit([]() -> void {
    exout << "---------------------exokit end" << std::endl;
  });

  registerDlibs(node::dlibs);

  int result;
  const char *argsEnv = getenv("ARGS");
  if (argsEnv) { // get args from launch command
    char args[4096];
    strncpy(args, argsEnv, sizeof(args));

    char *argv[64];
    size_t argc = 0;

    int argStartIndex = 0;
    for (int i = 0;; i++) {
      const char c = args[i];
      if (c == ' ') {
        args[i] = '\0';
        argv[argc] = args + argStartIndex;
        argc++;
        argStartIndex = i + 1;
        continue;
      } else if (c == '\0') {
        argv[argc] = args + argStartIndex;
        argc++;
        break;
      } else {
        continue;
      }
    }

    for (int i = 0; i < argc; i++) {
      exout << "get arg " << i << ": " << argv[i] << std::endl;
    }

    result = node::Start(argc, argv);
  } else { // get default launch args
    // get the requested launch arguments list
    const std::vector<const char *> defaultArguments = getDefaultArguments();

    // construct real argv layout that node expects
    size_t argc = defaultArguments.size();
    std::vector<char *> argv(argc);

    char argsString[4096];
    int offset = 0;
    for (size_t i = 0; i < defaultArguments.size(); i++) {
      const char *srcArgString = defaultArguments[i];
      char *dstArgString = argsString + offset;
      size_t srcArgSize = strlen(srcArgString) + 1;
      if ((offset + srcArgSize) < sizeof(argsString)) {
        memcpy(dstArgString, srcArgString, srcArgSize);

        argv[i] = dstArgString;

        offset += strlen(srcArgString) + 1;
      } else {
        LOG_ERROR("node command line arguments overflow: %d", offset);

        abort();
      }
    }

    result = node::Start(argc, argv.data());
  }

  LOG_INFO("exit code %d", result);

  close(1);
  close(2);
  stdoutReaderThread.join();
  stderrReaderThread.join();

#ifdef LUMIN
  return result;
#endif
}
