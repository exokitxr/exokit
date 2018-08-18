#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <cstdlib>
#include <dlfcn.h>
#include <errno.h>
#include <sstream>
#include <thread>
#include <ml_logging.h>
#include <ml_lifecycle.h>

#define LOG_TAG "exokit"
#define application_name LOG_TAG

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

namespace node {
  int Start(int argc, char* argv[]);
}

int stdoutfds[2];
int stderrfds[2];

int main() {
  pipe(stdoutfds);
  pipe(stderrfds);
  
  ML_LOG(Info, "start main 1");

  void *handle = dlopen(NULL, RTLD_LAZY);
  void *address1 = dlsym(handle, "_ZN4node10StreamBase15CreateWriteWrapEN2v85LocalINS1_6ObjectEEE");
  void *address2 = dlsym(handle, "_ZN14SkImage_RasterD0Ev");
  void *address3 = dlsym(handle, "_Z10makeWindowv");
  
  ML_LOG(Info, "start main 2 %x %x %x %x", handle, address1, address2, address3);

  int pid = fork();
  if (pid == 0) {
    dup2(stdoutfds[1], 1);
    close(stdoutfds[0]);
    dup2(stderrfds[1], 2);
    close(stderrfds[0]);

    MLLifecycleCallbacks lifecycle_callbacks = {};
    lifecycle_callbacks.on_new_initarg = onNewInitArg;
    lifecycle_callbacks.on_stop = onStop;
    lifecycle_callbacks.on_pause = onPause;
    lifecycle_callbacks.on_resume = onResume;
    lifecycle_callbacks.on_unload_resources = onUnloadResources;
    application_context_t application_context;
    MLResult lifecycle_status = MLLifecycleInit(&lifecycle_callbacks, (void*)&application_context);

    // ML_LOG(Info, "start main 2");

    /* char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    ML_LOG(Info, "start main 2.1 %s", cwd); */


    // dup2(stdoutfds[1], 2);

    /* stdoutfd = open("/tmp/stdout.txt", O_RDWR | O_TRUNC | O_CREAT);
    ML_LOG(Info, "start main 3 %x %x", stdoutfd, errno);
    dup2(stdoutfd, 1);

    ML_LOG(Info, "start main 4");

    stderrfd = open("/tmp/stderr.txt", O_RDWR | O_TRUNC | O_CREAT);
    ML_LOG(Info, "start main 5 %x", stdoutfd);
    dup2(stderrfd, 2); */

    // printf("lol\n");
    /* dprintf(stdoutfds[1], "lol 1\n");
    dprintf(1, "lol 2\n");
    printf("lol 3\n"); */

    std::atexit([]() -> void {
      close(stdoutfds[1]);
      close(stderrfds[1]);
    });

    {
      const char *nodeString = "node";
      const char *dotString = ".";
      const char *jsString = "examples/hello_xr.html";
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
      node::Start(argc, argv);
    }

    // ML_LOG(Info, "start main 7");
  } else {
    ML_LOG_TAG(Info, LOG_TAG, "---------------------exokit start");
    
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
  }

  return 0;
}
