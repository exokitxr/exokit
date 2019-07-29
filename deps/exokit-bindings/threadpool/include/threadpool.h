#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <thread>
#include <mutex>
#include <functional>

#include <v8.h>
#include <nan.h>

#include <windowsystem.h>

using namespace v8;

#define NUM_THREADS 8

namespace threadpool {

class QueueEntry {
public:
  QueueEntry(std::function<void()> workFn, std::function<void()> cbFn);
  QueueEntry();

  std::function<void()> workFn;
  std::function<void()> cbFn;
};

class ThreadPool {
public:
  ThreadPool();
  ~ThreadPool();

  void queueWork(std::function<void()> workFn = []() -> void {}, std::function<void()> cbFn = []() -> void {});
  static void ThreadPool::asyncFn(uv_async_t *handle);

// protected:
  std::vector<std::thread *> threads;
  std::deque<QueueEntry> reqQueue;
  std::deque<std::function<void()>> resQueue;
  uv_async_t *asyncHandle;
  std::mutex mutex;
  uv_sem_t sem;
  bool live;
};

extern thread_local ThreadPool *windowThreadPool;
ThreadPool *getWindowThreadPool();
void destroyWindowThreadPool();

};

#endif