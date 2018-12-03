#ifndef _C11TEST_ASPECT_HPP_
#define _C11TEST_ASPECT_HPP_

#include <utility>
#include "non_copyable.hpp"


#define HAS_MEMBER(member) template<typename T, typename... Args>struct has_member_##member\
{\
private:\
template<typename U> static auto Check(int) -> decltype(std::declval<U>().member(std::declval<Args>()...), std::true_type()); \
template<typename U> static auto Check(...) -> decltype(std::false_type()); \
public:\
static const bool value = std::is_same<decltype(Check<T>(0)),std::true_type>::value; \
}; \

//因为Check函数是私有成员，所以可变参数通过类模板参数来传递。
/*#define HAS_MEMBER(member)\
template<typename T, typename... Args>struct has_member_##member\
{\
private:\
    template<typename U> static auto Check(int) -> decltype(std::declval<U>().member(std::declval<Args>()...), std::true_type()); \
    template<typename U> static auto Check(...) -> decltype(std::false_type()); \
public:\
static const bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value; \
}; \*/

HAS_MEMBER(Before)
HAS_MEMBER(After)

template<typename Fun,typename... Args>//因为在几个成员函数共用可变模板参数列表，所以通过类模板参数传递。
class Aspect : public NonCopyable{
public:
    Aspect(Fun&& f)
        :fun_(std::forward<Fun>(f))
    {
    }

    template<typename T>
    typename std::enable_if<has_member_Before<T,Args...>::value
        && has_member_After<T,Args...>::value>::type Invoke(Args... args,T&& aspect)
    {
        aspect.Before(std::forward<Args>(args)...);
        fun_(std::forward<Args>(args)...);
        aspect.After(std::forward<Args>(args)...);
    }

    template<typename T>
    typename std::enable_if<has_member_Before<T,Args...>::value
        && !has_member_After<T,Args...>::value>::type Invoke(Args... args,T&& aspect)
    {
        aspect.Before(std::forward<Args>(args)...);
        fun_(std::forward<Args>(args)...);
    }

    template<typename T>
    typename std::enable_if<!has_member_Before<T,Args...>::value
        && has_member_After<T,Args...>::value>::type Invoke(Args... args,T&& aspect)
    {
        fun_(std::forward<Args>(args)...);
        aspect.After(std::forward<Args>(args)...);
    }

    template<typename Head,typename... Tail>
    void Invoke(Args... args,Head&& headAspect,Tail&&... tailAspect)
    {
        headAspect.Before(std::forward<Args>(args)...);
        Invoke(std::forward<Args>(args)...,std::forward<Tail>(tailAspect)...);
        headAspect.After(std::forward<Args>(args)...);
    }

private:
    Fun fun_;
};

template<typename T>
using identify_t = T;

template<typename... AP,typename... Args,typename F>
void Invoke(F&& f,Args... args)
{
    Aspect<F,Args...> asp(std::forward<F>(f));
    asp.Invoke(std::forward<Args>(args)...,identify_t<AP>()...);
}

#endif // _C11TEST_ASPECT_HPP_