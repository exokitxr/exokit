#include <unistd.h>
#include <string>
#include <map>
#include <thread>
#include <v8.h>
#include <ml_logging.h>
#include <ml_lifecycle.h>
#include <ml_privileges.h>
#include <exout>

using namespace v8;

namespace node {
  extern std::map<std::string, std::pair<void *, bool>> dlibs;
  int Start(int argc, char* argv[]);
}

#include "build/libexokit/dlibs.h"

const char *LOG_TAG = "exokit";
constexpr ssize_t STDIO_BUF_SIZE = 64 * 1024;

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
          ML_LOG_TAG(Info, LOG_TAG, "%s", buf + lineStart);

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
        ML_LOG_TAG(Info, LOG_TAG, "%s", buf);
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
          ML_LOG_TAG(Error, LOG_TAG, "%s", buf + lineStart);

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
        ML_LOG_TAG(Error, LOG_TAG, "%s", buf);
      }
      break;
    }
  }
}

int main(int argc, char **argv) {
  if (argc > 1) {
    return node::Start(argc, argv);
  }

  registerDlibs(node::dlibs);

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
  std::thread stdoutReaderThread([stdoutReadFd]() -> void { pumpStdout(stdoutReadFd); });
  int stderrReadFd = stderrfds[0];
  std::thread stderrReaderThread([stderrReadFd]() -> void { pumpStderr(stderrReadFd); });

  {
    MLResult result = MLPrivilegesStartup();
    if (result != MLResult_Ok) {
      exout << "failed to start privilege system " << result << std::endl;
    }
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

  exout << "---------------------exokit start" << std::endl;

  std::atexit([]() -> void {
    exout << "---------------------exokit end" << std::endl;
  });

  int result;
  const char *argsEnv = getenv("ARGS");
  if (argsEnv) {
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
  } else {
    const char *jsString;
    if (access("/package/app/index.html", F_OK) != -1) {
      jsString = "/package/app/index.html";
    } else {
      jsString = "examples/realitytabs.html";
    }

    const char *nodeString = "node";
    const char *experimentalWorkerString = "--experimental-worker";
    const char *dotString = ".";
    char argsString[4096];
    int i = 0;

    char *nodeArg = argsString + i;
    strncpy(nodeArg, nodeString, sizeof(argsString) - i);
    i += strlen(nodeString) + 1;

    char *experimentalWorkerArg = argsString + i;
    strncpy(experimentalWorkerArg, experimentalWorkerString, sizeof(argsString) - i);
    i += strlen(experimentalWorkerString) + 1;

    char *dotArg = argsString + i;
    strncpy(dotArg, dotString, sizeof(argsString) - i);
    i += strlen(dotString) + 1;

    char *jsArg = argsString + i;
    strncpy(jsArg, jsString, sizeof(argsString) - i);
    i += strlen(jsString) + 1;

    char *argv[] = {nodeArg, experimentalWorkerArg, dotArg, jsArg};
    size_t argc = sizeof(argv) / sizeof(argv[0]);

    node::Start(argc, argv);
  }

  close(1);
  close(2);
  stdoutReaderThread.join();
  stderrReaderThread.join();

  return result;
}
