#pragma once

#include "non_copyable.hpp"
#include <utility>

//判断一个类中是否有某成员函数。
//因为Check函数是私有成员，所以可变参数通过类模板参数来传递。
// Check<T>(0),首先匹配第一个Check，如果“std::declval<U>().member(std::declval<Args>()...)”如果调用成功的话
//逗号表达式会返回std::true_type()类型，否则匹配第二个Check()，返回false_type()类型。
#define HAS_MEMBER(member)                                                     \
  template <typename T, typename... Args> struct has_member_##member {         \
  private:                                                                     \
    template <typename U>                                                      \
    static auto Check(int)                                                     \
        -> decltype(std::declval<U>().member(std::declval<Args>()...),         \
                    std::true_type());                                         \
    template <typename U>                                                      \
    static auto Check(...) -> decltype(std::false_type());                     \
                                                                               \
  public:                                                                      \
    static const bool value =                                                  \
        std::is_same<decltype(Check<T>(0)), std::true_type>::value;            \
  };

HAS_MEMBER(Before)
HAS_MEMBER(After)

//因为在几个成员函数共用可变模板参数列表，所以通过类模板参数传递。
// Fun为函数类型，Args为函数的参数类型
//主要面对 函数对象，在调用函数前后调用切面函数。
//这里需要显示指定类模板参数
//问题：若类模板参数也作为函数的参数类型，可以自动推导，不用显示指定也可以吗？不可以，类模板参数必须显示指定。
template <typename Fun, typename... Args> class Aspect : public NonCopyable {
public:
  //这里用了右值引用
  Aspect(Fun &&f) : fun_(std::forward<Fun>(f)) {}

  // Before和after都有的，T为切面类，这里的T都是自动推导的，因为做了函数参数类型
  template <typename T>
  typename std::enable_if<has_member_Before<T, Args...>::value &&
                          has_member_After<T, Args...>::value>::type
  Invoke(Args &&... args, T &&aspect) //这里用于自动推导，所以使用了&&
  {
    aspect.Before(std::forward<Args>(args)...);
    fun_(std::forward<Args>(args)...);
    aspect.After(std::forward<Args>(args)...);
  }
  //有前无后
  template <typename T>
  typename std::enable_if<has_member_Before<T, Args...>::value &&
                          !has_member_After<T, Args...>::value>::type
  Invoke(Args &&... args, T &&aspect) {
    aspect.Before(std::forward<Args>(args)...);
    fun_(std::forward<Args>(args)...);
  }
  //无前有后
  template <typename T>
  typename std::enable_if<!has_member_Before<T, Args...>::value &&
                          has_member_After<T, Args...>::value>::type
  Invoke(Args &&... args, T &&aspect) {
    fun_(std::forward<Args>(args)...);
    aspect.After(std::forward<Args>(args)...);
  }
  //调用所有的切面，函数内部调用自己的成员函数Invoke，自动匹配。
  template <typename Head, typename... Tail>
  void Invoke(Args &&... args, Head &&headAspect, Tail &&... tailAspect) {
    headAspect.Before(std::forward<Args>(args)...);
    Invoke(std::forward<Args>(args)..., std::forward<Tail>(tailAspect)...);
    headAspect.After(std::forward<Args>(args)...);
  }

private:
  Fun fun_;
};

template <typename T> using identify_t = T;

// 1,不作为函数参数类型的模板参数，则不能自动推导出类型，需要显示指定其类型。
// 2,类模板参数需要显示指定类型，除非有默认模板参数。
//在调用此函数时，显示指定了AP
template <typename... AP, typename... Args, typename F>
void Invoke(F &&f, Args &&... args) {
  Aspect<F, Args...> asp(
      std::forward<F>(f)); //先创建待切面的对象，参数为函数对象。
  asp.Invoke(
      std::forward<Args>(args)...,
      identify_t<
          AP>()...); //再调用所有的切面函数，这里根据切面类型，构造了ap对象。
}
