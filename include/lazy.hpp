#ifndef _C11TEST_LAZY_HPP_
#define _C11TEST_LAZY_HPP_

#include <functional>

#include "optional.hpp"
//延迟加载或创建，初始化。
//这个T是最后加载返回的对象。
template<typename T>
class Lazy{
public:
    Lazy(){}
    template<typename Func,typename... Args>
    Lazy(Func func,Args&&... args)
    {
        //通过捕获的方式实现闭包，与存储构造函数，之后再调用是两种不同的方式，实现不同的目的。
        //ioc_container的构造机动性更强，内部实现中，取出构造函数之后，可以多次传不同的参数。
        //这个一个对象只能延迟加载一个类型的对象，参数只能传一次，加载一次。
        func_ = [&func,&args...](){return func(args...);};
    }

    //返回值不能是T&，因为T&不能绑定到const T
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

//这里的Fn为什么用&&，而Lazy的构造函数里的Func不用呢？
template<typename Fn,typename... Args>
Lazy<typename std::result_of<Fn(Args...)>::type> lazy(Fn&& fn,Args&&... args)
{
    return Lazy<typename std::result_of<Fn(Args...)>::type>(std::forward<Fn>(fn),std::forward<Args>(args)...);
}

#endif // _C11TEST_LAZY_HPP_
