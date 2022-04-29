#pragma once

#include <functional>
#include <map>
#include <string>

#include "any.hpp"
#include "function_traits.hpp"
#include "non_copyable.hpp"

class MessageBus : NonCopyable {
public:
  //注册要接受的topic和消息接收器
  template <typename F> void Attach(F &&f, const std::string &topic = "") {
    //转换成统一的消息接收者类型。std::function<T>包装器类型。
    auto func = to_function(std::forward<F>(f));
    Add(topic, std::move(func));
  }
  //发送消息，通知消息，无参的，接收器的返回类型为R
  template <typename R> void SendReq(const std::string &topic) {
    using function_type = std::function<R()>;
    std::string msg_type = topic + typeid(function_type).name();
    auto range = map_.equal_range(msg_type);
    for (Iterater it = range.first; it != range.second; ++it) {
      auto f = it->second.AnyCast<function_type>;
      f();
    }
  }

  //接收器的返回类型为R，参数类型为Args
  template <typename R, typename... Args>
  void SendReq(Args &&... args, const std::string &topic = "") {
    using function_type = std::function<R(Args...)>;
    std::string msg_type = topic + typeid(function_type).name();
    auto range = map_.equal_range(msg_type);
    for (Iterater it = range.first; it != range.second; ++it) {
      auto f = it->second.AnyCast<function_type>;
      f(std::forward<Args>(args)...);
    }
  }

  //删除订阅者，批量删除
  template <typename R, typename... Args>
  void Remove(const std::string &topic = "") {
    using function_type = std::function<R(Args...)>;
    std::string msg_type = topic + typeid(function_type).name();
    auto range = map_.equal_range(msg_type);
    map_.erase(range.first, range.second); //这不是把所有订阅者都删了吗？
  }

private:
  template <typename Function>
  void Add(const std::string &topic, Function &&f) {
    // topic + 类型签名 为key值。
    std::string msg_type = topic + typeid(Function).name();
    map_.emplace(msg_type, std::forward<Function>(f));
  }

private:
  //用Any类型擦除，用multimap
  std::multimap<std::string, Any> map_;
  typedef std::multimap<std::string, Any>::iterator Iterater;
};