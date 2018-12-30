#include <browser.h>

#include <v8.h>
#include <node.h>
#include <nan.h>

#include <iostream>

using namespace std;
using namespace v8;
using namespace node;

namespace browser {

// helpers

void QueueOnBrowserThread(std::function<void()> fn) {
  {
    std::lock_guard<std::mutex> lock(browserThreadFnMutex);
    browserThreadFns.push_back(fn);
  }
  
  uv_sem_post(&browserThreadSem);
}

void QueueOnBrowserThreadFront(std::function<void()> fn) {
  {
    std::lock_guard<std::mutex> lock(browserThreadFnMutex);
    browserThreadFns.push_front(fn);
  }
  
  uv_sem_post(&browserThreadSem);
}

void RunOnMainThread(std::function<void()> fn) {
  {
    std::lock_guard<std::mutex> lock(mainThreadFnMutex);
    mainThreadFns.push_back(std::pair<std::function<void()>, bool>(fn, true));
  }

  uv_async_send(&mainThreadAsync);
  uv_sem_wait(&mainThreadSem);
}

void QueueOnMainThread(std::function<void()> fn) {
  {
    std::lock_guard<std::mutex> lock(mainThreadFnMutex);
    mainThreadFns.push_back(std::pair<std::function<void()>, bool>(fn, false));
  }

  uv_async_send(&mainThreadAsync);
}

void MainThreadAsync(uv_async_t *handle) {
  std::deque<std::pair<std::function<void()>, bool>> localMainThreadFns;
  {
    std::lock_guard<std::mutex> lock(mainThreadFnMutex);
    
    localMainThreadFns = std::move(mainThreadFns);
    mainThreadFns.clear();
  }
  
  for (size_t i = 0; i < localMainThreadFns.size(); i++) {
    std::pair<std::function<void()>, bool> &o = localMainThreadFns[i];
    
    o.first();
    if (o.second) {
      uv_sem_post(&mainThreadSem);
    }
  }
}

// variables

bool embeddedInitialized = false;
std::thread browserThread;

uv_sem_t constructSem;
uv_sem_t mainThreadSem;
uv_sem_t browserThreadSem;

std::mutex browserThreadFnMutex;
std::deque<std::function<void()>> browserThreadFns;

uv_async_t mainThreadAsync;
std::mutex mainThreadFnMutex;
std::deque<std::pair<std::function<void()>, bool>> mainThreadFns;

}
