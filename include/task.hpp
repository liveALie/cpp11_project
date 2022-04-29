#pragma once

#include <functional>
#include <future>
#include <utility>

template <typename T>
class Task;

//模板特化
// 1，Wait，无参数则调用这个
// 2，Get，有参数则调用这个。
// 3，Run，获取future则调用这个
// 4,Then最复杂的，以上一个的执行结果作为下一个任务的入参，返回要给Task
template <typename R, typename... Args>
class Task<R(Args...)> {
 public:
  typedef R ReturnType;
  //提供两种狗咱函数
  Task(std::function<R(Args...)> &&f) : fun_(std::move(f)) {}
  Task(const std::function<R(Args...)> &f) : fun_(f) {}
  ~Task() {}
  void Wait() { std::async(fun_).wait(); }

  R Get(Args &&...args) {
    return std::async(fun_, std::forward<Args>(args)...).get();
  }

  std::shared_future<R> Run() { return std::async(fun_); }

  // Then函数返回的是一个Task对象
  // auto Then(F&& f)->Task<typename std::result_of<F(R)>::type(Args...)>
  //上一句的函数生命，当R为void时，编译出错，详细原因有待考察。
  template <typename F>
  auto Then(F &&f) {
    typedef typename std::result_of<F(R)>::type Return_type;
    auto func = std::move(fun_);
    //此为构造Task的入参，Then调用完之后调用Get，获取执行后的结果。由R1(Args...)
    //到 。。。 Rn(Args...)的变换。
    return Task<Return_type(Args...)>([func, &f](Args... args) {
      std::future<R> last_future =
          std::async(func, std::forward<Args>(args)...);
      return std::async(f, last_future.get()).get();
    });
  }

 private:
  std::function<R(Args...)> fun_;
};
