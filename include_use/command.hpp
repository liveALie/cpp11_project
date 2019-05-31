#ifndef _C11TEST_COMMAND_HPP_
#define _C11TEST_COMMAND_HPP_

#include <functional>
#include <type_traits>
// command需要指定返回类型，默认为void
template <typename R = void> struct CommCommand {
public:
  //包装可调用对象
  template <class F, class... Args,
            class = typename std::enable_if<
                !std::is_member_function_pointer<F>::value>::type>
  void Wrap(F &&f, Args &&... args) {
    fun_ = [&]() { return f(args...); }; //为什么这里不用std::forward呢？
  }
  //包装类的const成员函数指针
  template <class C, class... DArgs, class P, class... Args>
  void Wrap(R (C::*f)(DArgs...) const, P &&p, Args &&... args) {
    fun_ = [&, f]() { return (*p.*f)(args...); };
  }
  //包装类的非const成员函数指针
  template <class C, class... DArgs, class P, class... Args>
  void Wrap(R (C::*f)(DArgs...), P &&p, Args &&... args) { //成员函数指针的类型
    fun_ = [&, f]() { return (*p.*f)(args...); }; //调用成员函数指针
  }
  //包装类的const成员，返回const的函数指针，没有此成员函数
  //这个函数是成立的，因为返回类型为R，而不是R&。不能R&绑定到const R上。
  template <class C, class... DArgs, class P, class... Args>
  void Wrap(const R (C::*f)(DArgs...) const, P &&p, Args &&... args) {
    fun_ = [&, f]() { return (*p.*f)(args...); };
  }

  R Excecute() { return fun_(); }

private:
  std::function<R()> fun_;
};

#endif // _C11TEST_COMMAND_HPP_