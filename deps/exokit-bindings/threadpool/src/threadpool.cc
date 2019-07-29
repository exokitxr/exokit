#include <threadpool.h>

namespace threadpool {

QueueEntry::QueueEntry(std::function<void()> workFn, std::function<void()> cbFn) : workFn(workFn), cbFn(cbFn) {}
QueueEntry::QueueEntry() {}

ThreadPool::ThreadPool() : live(true) {
  asyncHandle = new uv_async_t();
  asyncHandle->data = this;
  uv_loop_t *loop = windowsystembase::GetEventLoop();
  uv_async_init(loop, asyncHandle, asyncFn);
  uv_sem_init(&sem, 0);
  
  threads.reserve(NUM_THREADS);
  for (int i = 0; i < NUM_THREADS; i++) {
    std::thread *thread = new std::thread([this]() -> void {
      for (;;) {
        uv_sem_wait(&sem);
        
        QueueEntry queueEntry;
        {
          std::lock_guard<std::mutex> lock(mutex);

          if (live) {
            queueEntry = std::move(reqQueue.front());
            reqQueue.pop_front();
          }
        }

        if (queueEntry.workFn) {
          queueEntry.workFn();

          resQueue.push_back(queueEntry.cbFn);
          uv_async_send(asyncHandle);
        } else {
          break;
        }
      }
    });
    threads.push_back(thread);
  }
}

void deleteHandle(uv_handle_t *handle) {
  delete (uv_async_t *)handle;
}
ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(mutex);

    live = false;
  }
  
  for (int i = 0; i < NUM_THREADS; i++) {
    uv_sem_post(&sem);
  }
  for (int i = 0; i < NUM_THREADS; i++) {
    delete threads[i];
  }
  uv_close((uv_handle_t *)asyncHandle, deleteHandle);
  uv_sem_destroy(&sem);
}

void ThreadPool::queueWork(std::function<void()> workFn, std::function<void()> cbFn) {
  {
    std::lock_guard<std::mutex> lock(mutex);

    reqQueue.push_back(QueueEntry(workFn, cbFn));

    uv_sem_post(&sem);
  }
}

void ThreadPool::asyncFn(uv_async_t *handle) {
  ThreadPool *threadPool = (ThreadPool *)handle->data;

  std::function<void()> queueEntry;
  {
    std::lock_guard<std::mutex> lock(threadPool->mutex);

    queueEntry = threadPool->resQueue.front();
    threadPool->resQueue.pop_front();
  }

  queueEntry();
}

thread_local ThreadPool *windowThreadPool = nullptr;
ThreadPool *getWindowThreadPool() {
  if (!windowThreadPool) {
    windowThreadPool = new ThreadPool();
  }
  return windowThreadPool;
}
void destroyWindowThreadPool() {
  if (windowThreadPool) {
    delete windowThreadPool;
    windowThreadPool = nullptr;
  }
}

};