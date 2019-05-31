#ifndef _C11TEST_EVENTS_HPP_
#define _C11TEST_EVENTS_HPP_

#include <map>

#include "non_copyable.hpp"

template <class Func> //观察者类型为函数类型,必须显示指定
class Events : public NonCopyable {
public:
  Events() {}
  ~Events() {}
  //注册观察者。
  //此为类模板参数，不支持左值右值的自动推导，必须定义左值和右值的两个函数。
  //入参为左值和右值的两个connect函数，都会调用Assign，所以Assign需要定义为一个函数模板，让参数类型自动推导
  //问题：为什么不定义一个参数为Func&&就行了呢？因为Func是在类模板参数中指定的。
  int Connect(Func &&f) { //入参之后为左值
    return Assign(f);
  }

  int Connect(const Func &f) { //入参之后为右值
    return Assign(f);
  }

  void Disconnect(int key) {
    connections_.erase(key); //这里可以直接删除key吗？可以 ，windows没问题。
  }

  template <class... Args> //模板参数列表，消除重复代码，可以自动推导
  void Notify(Args &&... args) {
    for (auto &it : connections_) {
      it.second(std::forward<Args>(args)...);
    }
  }

private:
  template <class F> //为什么这里也要定义一个函数模板，为了自动推导类型
  int Assign(F &&f) {
    int k = observer_id_++;
    connections_.emplace(k, std::forward<F>(f));
    return k;
  }

private:
  int observer_id_ = 0;
  std::map<int, Func> connections_;
};

#endif // _C11TEST_EVENTS_HPP_