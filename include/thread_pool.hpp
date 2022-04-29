#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <thread>

#include "sync_queue.hpp"

class ThreadPool {
 public:
  using Task = std::function<void()>;
  explicit ThreadPool(int32_t num_threads = std::thread::hardware_concurrency(),
                      int32_t max_queue_size = 100)
      : thread_group_(num_threads), queue_(max_queue_size) {
    Start(num_threads);
  }

      ~ThreadPool() { Stop(); }

      void Stop() {
        std::call_once(flag_, [this] { StopThreadPool(); });
      }

      void AddTask(Task &&task) { queue_.Put(std::forward<Task>(task)); }

      void AddTask(const Task &task) { queue_.Put(task); }

     private:
      void Start(int32_t num_threads) {
        running_ = true;
        for (int32_t i = 0; i < num_threads; ++i) {
          thread_group_[i] = std::thread(&ThreadPool::RunThread, this);
        }
      }

      void RunThread() {
        while (running_) {
          std::list<Task> list;
          queue_.Take(list);
          for (auto &task : list) {
            if (!running_) {
              return;
            }
            task();
          }
        }
      }

      void StopThreadPool() {
        running_ = false;
        queue_.Stop();

        for (auto &thread : thread_group_) {
          if (thread.joinable()) {
            thread.join();
          }
        }
        thread_group_.clear();
      }

     private:
      std::vector<std::thread> thread_group_;
      SyncQueue<Task> queue_;
      atomic_bool running_;
      std::once_flag flag_;
};