#pragma once

#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

using namespace std;
// 1,添加
// 2，取任务，可以取一个，或者取一列
// 3，空或者满的状态查询，和任务大小查询
// 4，停止任务队列
//维护一个任务列表
template <typename T> //此为任务的类型,由线程池指定
class SyncQueue {
public:
  SyncQueue(int maxSize) : max_size_(maxSize), need_stop_(false) {}

  //要提供两个版本的Put
  void Put(const T &x) { Add(x); }

  void Put(T &&x) { Add(std::forward<T>(x)); }
  //提供两个版本的获取
  //一次性取完
  void Take(std::list<T> &list) {
    std::unique_lock<std::mutex> locker(mutex_);
    not_empty_.wait(locker, [this]() { return need_stop_ || NotEmpty(); });
    if (need_stop_) {
      return;
    }
    list = std::move(queue_);
    not_full_.notify_one();
  }
  //一次取一个任务
  void Take(T &t) {
    //此处用unique_ptr
    std::unique_lock<std::mutex> locker(mutex_);
    //如果不空或者需要停止，则往下执行，如果空则等待。
    not_empty_.wait(locker, [this] { return need_stop_ || NotEmpty(); });
    if (need_stop_) {
      return;
    }
    t = queue_.front();
    queue_.pop();
    not_full_.notify_one();
  }
  //停止
  void Stop() {
    {
      //先上锁，再置标志位为停止。
      std::lock_guard<std::mutex> locker(mutex_);
      need_stop_ = true;
    }
    not_full_.notify_all();
    not_empty_.notify_all();
  }
  //空
  bool Empty() {
    std::lock_guard<std::mutex> locker(mutex_);
    return queue_.empty();
  }
  //满
  bool Full() {
    std::lock_guard<std::mutex> locker(mutex_);
    return queue_.size() == max_size_;
  }
  //大小
  size_t Size() {
    std::lock_guard<std::mutex> locker(mutex_);
    return queue_.size();
  }
  //计数
  int Count() { return queue_.size(); }

private:
  bool NotFull() const {
    bool full = (queue_.size() >= max_size_);
    if (full) {
      cout << "storage is full" << endl;
    }
    return !full;
  }

  bool NotEmpty() const {
    bool empty = queue_.empty();
    if (empty) {
      cout << "storage memory is empty,thread id:" << std::this_thread::get_id()
           << endl;
    }
    return !empty;
  }
  //为两个版本的添加接口，提供一个通用的内部接口
  template <typename F> void Add(F &&x) {
    // unique_lock和lock_guard的用法区别：前者和条件变量结合，后者只是上锁。
    std::unique_lock<std::mutex> locker(mutex_);
    //用unique_lock 和 condition_variable结合使用的。
    //反过来理解，wait not
    // full，若not_full或停止了，往下执行，否则，unique_lock会锁住，等到没满为止。
    not_full_.wait(locker, [this]() { return need_stop_ || NotFull(); });
    if (need_stop_) {
      return;
    }
    queue_.push_back(std::forward<F>(x));
    not_empty_.notify_one();
  }

private:
  std::list<T> queue_;                //队列
  std::mutex mutex_;                  //锁
  std::condition_variable not_empty_; //非空的条件变量
  std::condition_variable not_full_;  //非满的条件变量
  size_t max_size_;                   //最大任务数
  bool need_stop_;                    //停止的标志位
};