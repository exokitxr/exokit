#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <errno.h>
#include <sstream>
#include <string>
#include <map>
#include <thread>
#include <v8.h>
#include <ml_logging.h>
#include <ml_lifecycle.h>
#include <ml_privileges.h>

#define LOG_TAG "exokit"
#define application_name LOG_TAG

using namespace v8;

namespace node {
  extern std::map<std::string, void *> dlibs;
  int Start(int argc, char* argv[]);
}
int stdoutfds[2];
int stderrfds[2];

#include "build/libexokit/dlibs.h"

/* struct application_context_t {
  int dummy_value;
};
enum DummyValue {
  STOPPED = 0,
  RUNNING,
  PAUSED,
};

static void onNewInitArg(void* application_context) {
  MLLifecycleInitArgList *args;
  MLLifecycleGetInitArgList(&args);

  ((struct application_context_t*)application_context)->dummy_value = DummyValue::RUNNING;
  ML_LOG_TAG(Info, LOG_TAG, "%s: On new init arg called %x.", application_name, args);
}

static void onStop(void* application_context) {
  ((struct application_context_t*)application_context)->dummy_value = DummyValue::STOPPED;
  ML_LOG_TAG(Info, LOG_TAG, "%s: On stop called.", application_name);
}

static void onPause(void* application_context) {
  ((struct application_context_t*)application_context)->dummy_value = DummyValue::PAUSED;
  ML_LOG_TAG(Info, LOG_TAG, "%s: On pause called.", application_name);
}

static void onResume(void* application_context) {
  ((struct application_context_t*)application_context)->dummy_value = DummyValue::RUNNING;
  ML_LOG_TAG(Info, LOG_TAG, "%s: On resume called.", application_name);
}

static void onUnloadResources(void* application_context) {
  ((struct application_context_t*)application_context)->dummy_value = DummyValue::STOPPED;
  ML_LOG_TAG(Info, LOG_TAG, "%s: On unload resources called.", application_name);
} */

/* extern "C" {
  void node_register_module_exokit(Local<Object> exports, Local<Value> module, Local<Context> context);
  void node_register_module_vm_one(Local<Object> exports, Local<Value> module, Local<Context> context);
  void node_register_module_raw_buffer(Local<Object> exports, Local<Value> module, Local<Context> context);
  void node_register_module_child_process_thread(Local<Object> exports, Local<Value> module, Local<Context> context);
}

inline void registerDlibs(std::map<std::string, void *> &dlibs) {
  dlibs["/package/build/Release/exokit.node"] = (void *)&node_register_module_exokit;
  dlibs["/package/node_modules/vm-one/build/Release/vmOne.node"] = (void *)&node_register_module_vm_one;
  dlibs["/package/node_modules/raw-buffer/build/Release/raw_buffer.node"] = (void *)&node_register_module_raw_buffer;
  dlibs["/package/node_modules/child-process-thread/build/Release/child_process_thread.node"] = (void *)&node_register_module_child_process_thread;
} */

constexpr size_t STDIO_BUF_SIZE = 64 * 1024;
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

int main(int argc, char **argv) {
  if (argc > 1) {
    return node::Start(argc, argv);
  }

  registerDlibs(node::dlibs);

  MLResult result = MLPrivilegesStartup();
  if (result != MLResult_Ok) {
    ML_LOG(Info, "failed to start privilege system %x", result);
  }
  const size_t numPrivileges = sizeof(privileges) / sizeof(privileges[0]);
  for (size_t i = 0; i < numPrivileges; i++) {
    const MLPrivilegeID &privilege = privileges[i];
    MLResult result = MLPrivilegesCheckPrivilege(privilege);
    if (result != MLPrivilegesResult_Granted) {
      const char *s = MLPrivilegesGetResultString(result);
      ML_LOG(Info, "did not have privilege %u: %u %s", privilege, result, s);

      MLResult result = MLPrivilegesRequestPrivilege(privilege);
      if (result != MLPrivilegesResult_Granted) {
        const char *s = MLPrivilegesGetResultString(result);
        ML_LOG(Info, "failed to get privilege %u: %u %s", privilege, result, s);
      }
    }
  }

  /* {
    MLLifecycleCallbacks lifecycle_callbacks = {};
    lifecycle_callbacks.on_new_initarg = onNewInitArg;
    lifecycle_callbacks.on_stop = onStop;
    lifecycle_callbacks.on_pause = onPause;
    lifecycle_callbacks.on_resume = onResume;
    lifecycle_callbacks.on_unload_resources = onUnloadResources;
    application_context_t application_context;
    MLResult lifecycle_status = MLLifecycleInit(&lifecycle_callbacks, (void*)&application_context);

    MLResult result = MLLifecycleSetReadyIndication();
    if (result == MLResult_Ok) {
      ML_LOG(Info, "lifecycle ready!");
    } else {
      ML_LOG(Error, "failed to indicate lifecycle ready: %u", result);
    }
  }

  {
    ML_LOG(Info, "------------------------------test query");

    const char *host = "google.com";
    struct addrinfo hints, *res;
    int errcode;
    char addrstr[100];
    void *ptr;

    memset (&hints, 0, sizeof (hints));
    // hints.ai_flags = AI_DEFAULT;
    // hints.ai_family = PF_UNSPEC;
    // hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo(host, NULL, &hints, &res);
    ML_LOG (Info, "Host: %s %x %s\n", host, errcode, gai_strerror(errcode));
    if (errcode == 0) {
      while (res)
        {
          inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);

          switch (res->ai_family)
            {
            case AF_INET:
              ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
              break;
            case AF_INET6:
              ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
              break;
            }
          inet_ntop (res->ai_family, ptr, addrstr, 100);
          ML_LOG(Info, "IPv%d address: %s (%s)", res->ai_family == PF_INET6 ? 6 : 4, addrstr, res->ai_canonname);
          res = res->ai_next;
        }
    } else {
      ML_LOG(Info, "failed to getaddrinfo %x", errcode);
    }
  }

  ML_LOG(Info, "sleeping 1");

  sleep(1000);

  ML_LOG(Info, "sleeping 2"); */

  pipe(stdoutfds);
  pipe(stderrfds);

  int pid = fork();
  if (pid != 0) { // parent
    dup2(stdoutfds[1], 1);
    close(stdoutfds[0]);
    dup2(stderrfds[1], 2);
    close(stderrfds[0]);

    std::atexit([]() -> void {
      close(stdoutfds[1]);
      close(stderrfds[1]);
    });

    const char *argsEnv = getenv("ARGS");
    if (argsEnv) {
      
      char args[4096];
      strncpy(args, argsEnv, sizeof(args));

      char *argv[64];
      size_t argc = 0;

      const char *nodeString = "node";
      const char *dotString = ".";
      const char *jsString = "examples/realityprojection_ml.html";
      char argsString[4096];
      int i = 0;

      char *nodeArg = argsString + i;
      strncpy(nodeArg, nodeString, sizeof(argsString) - i);
      i += strlen(nodeString) + 1;

      char *dotArg = argsString + i;
      strncpy(dotArg, dotString, sizeof(argsString) - i);
      i += strlen(dotString) + 1;

      char *jsArg = argsString + i;
      strncpy(jsArg, jsString, sizeof(argsString) - i);
      i += strlen(jsString) + 1;


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

      return node::Start(argc, argv);
    } else {
      const char *nodeString = "node";
      const char *dotString = ".";
      const char *jsString = "examples/hello_ml.html";
      char argsString[4096];
      int i = 0;

      char *nodeArg = argsString + i;
      strncpy(nodeArg, nodeString, sizeof(argsString) - i);
      i += strlen(nodeString) + 1;

      char *dotArg = argsString + i;
      strncpy(dotArg, dotString, sizeof(argsString) - i);
      i += strlen(dotString) + 1;

      char *jsArg = argsString + i;
      strncpy(jsArg, jsString, sizeof(argsString) - i);
      i += strlen(jsString) + 1;

      char *argv[] = {nodeArg, dotArg, jsArg};
      size_t argc = sizeof(argv) / sizeof(argv[0]);

      return node::Start(argc, argv);
    }
  } else { // child
    ML_LOG_TAG(Info, LOG_TAG, "---------------------exokit start 1");

    close(stdoutfds[1]);
    close(stderrfds[1]);

    std::thread stdoutThread([]() -> void {
      int fd = stdoutfds[0];

      char buf[STDIO_BUF_SIZE + 1];
      ssize_t i = 0;
      ssize_t lineStart = 0;
      for (;;) {
        ssize_t size = read(fd, buf + i, STDIO_BUF_SIZE - i);
        // ML_LOG(Info, "=============read result %x %x %x", fd, size, errno);
        if (size > 0) {
          for (ssize_t j = i; j < i + size; j++) {
            if (buf[j] == '\n') {
              buf[j] = 0;
              ML_LOG_TAG(Info, LOG_TAG, "%s", buf + lineStart);

              lineStart = j + 1;
            }
          }

          i += size;

          if (i >= STDIO_BUF_SIZE) {
            ssize_t lineLength = i - lineStart;
            memcpy(buf, buf + lineStart, lineLength);
            i = lineLength;
            lineStart = 0;
          }
        } else {
          if (i > 0) {
            buf[i] = 0;
            ML_LOG_TAG(Info, LOG_TAG, "%s", buf);
          }
          break;
        }
      }
    });
    std::thread stderrThread([]() -> void {
      int fd = stderrfds[0];

      char buf[STDIO_BUF_SIZE + 1];
      ssize_t i = 0;
      ssize_t lineStart = 0;
      for (;;) {
        ssize_t size = read(fd, buf + i, STDIO_BUF_SIZE - i);
        // ML_LOG(Info, "=============read result %x %x %x", fd, size, errno);
        if (size > 0) {
          for (ssize_t j = i; j < i + size; j++) {
            if (buf[j] == '\n') {
              buf[j] = 0;
              ML_LOG_TAG(Info, LOG_TAG, "%s", buf + lineStart);

              lineStart = j + 1;
            }
          }

          i += size;

          if (i >= STDIO_BUF_SIZE) {
            ssize_t lineLength = i - lineStart;
            memcpy(buf, buf + lineStart, lineLength);
            i = lineLength;
            lineStart = 0;
          }
        } else {
          if (i > 0) {
            buf[i] = 0;
            ML_LOG_TAG(Info, LOG_TAG, "%s", buf);
          }
          break;
        }
      }
    });

    stdoutThread.join();
    stderrThread.join();

    ML_LOG_TAG(Info, LOG_TAG, "---------------------exokit end");

    return 0;
  }
}
