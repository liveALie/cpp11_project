#ifndef _C11TEST_LAZY_HPP_
#define _C11TEST_LAZY_HPP_

#include <functional>

#include "optional.hpp"

template<typename T>
class Lazy{
public:
    Lazy(){}
    template<typename Func,typename... Args>
    Lazy(Func func,Args&&... args)
    {
        func_ = [&func,&args...](){return func(args...);};
    }

    const T& Value()
    {
        if(!value_){
            value_ = func_();
        }
        return *value_;
    }

    bool IsValueCreated()const
    {
        return value_.IsInit();
    }

private:
    std::function<T()> func_;
    Optional<T> value_;
};

template<typename Fn,typename... Args>
Lazy<typename std::result_of<Fn(Args...)>::type> lazy(Fn&& fn,Args... args)
{
    return Lazy<typename std::result_of<Fn(Args...)>::type>(std::forward<Fn>(fn),std::forward<Args>(args)...);
}

#endif // _C11TEST_LAZY_HPP_
