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

struct application_context_t {
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
}

namespace node {
  int Start(int argc, char* argv[]);
}

/* int stdoutfd;
int stderrfd; */

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
      // sleep(1);

      // ML_LOG(Info, "end main 1");

      close(stdoutfds[1]);
      close(stderrfds[1]);

      /* fseek((FILE *)stdoutfd, 0, SEEK_END);
      long stdoutsize = ftell((FILE *)stdoutfd);
      fseek((FILE *)stdoutfd, 0, SEEK_SET);
      char *stdoutString = (char *)malloc(stdoutsize + 1);
      fread(stdoutString, stdoutsize, 1, (FILE *)stdoutfd);
      stdoutString[stdoutsize] = 0;
      ML_LOG(Info, "end main 1 %x %s", stdoutsize, stdoutString);
      free(stdoutString);

      fseek((FILE *)stderrfd, 0, SEEK_END);
      long stderrsize = ftell((FILE *)stderrfd);
      fseek((FILE *)stderrfd, 0, SEEK_SET);
      char *stderrString = (char *)malloc(stderrsize + 1);
      fread(stderrString, stderrsize, 1, (FILE *)stderrfd);
      stderrString[stderrsize] = 0;
      ML_LOG(Warning, "end main 2 %x %s", stderrsize, stderrString);
      free(stderrString); */
    });

    // ML_LOG(Info, "start main 6");

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
      // ML_LOG(Info, "start stdout thread");
      char buf[128];
      ssize_t i = 0;
      for (;;) {
        ssize_t size = read(stdoutfds[0], buf + i, sizeof(buf - i - 1));
        // ML_LOG(Info, "=============read result %x %x %x", stdoutfds[0], size, errno);
        if (size > 0) {
          i += size;

          if (i >= (sizeof(buf) - 1)) {
            buf[i] = 0;
            ML_LOG_TAG(Info, LOG_TAG, "%s", buf);

            i = 0;
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
      // ML_LOG(Info, "start stderr thread");
      char buf[128];
      ssize_t i = 0;
      for (;;) {
        ssize_t size = read(stderrfds[0], buf + i, sizeof(buf - i - 1));
        // ML_LOG(Info, "=============read result %x %x %x", stdoutfds[0], size, errno);
        if (size > 0) {
          i += size;

          if (i >= (sizeof(buf) - 1)) {
            buf[i] = 0;
            ML_LOG_TAG(Warning, LOG_TAG, "%s", buf);

            i = 0;
          }
        } else {
          if (i > 0) {
            buf[i] = 0;
            ML_LOG_TAG(Warning, LOG_TAG, "%s", buf);
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
