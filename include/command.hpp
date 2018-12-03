#ifndef _C11TEST_COMMAND_HPP_
#define _C11TEST_COMMAND_HPP_

#include <functional>
#include <type_traits>

template<typename R = void>
struct CommCommand{
public:
    template<class F,class... Args, class = typename std::enable_if<
        !std::is_member_function_pointer<F>::value>::type>
    void Wrap(F&& f,Args... args){
        fun_ = [&](){ return f(args...); };
    }

    template<class C,class... DArgs, class P,class... Args>
    void Wrap(R(C::*f)(DArgs...)const,P&& p,Args&&... args){
        fun_ = [&,f](){ return (*p.*f)(args...); };
    }

    template<class C,class... DArgs, class P,class... Args>
    void Wrap(R(C::*f)(DArgs...),P&& p,Args&&... args){
        fun_ = [&,f](){ return (*p.*f)(args...); };
    }

    template<class C,class... DArgs, class P,class... Args>
    void Wrap(const R(C::*f)(DArgs...) const,P&& p,Args&&... args){
        fun_ = [&,f](){ return (*p.*f)(args...); };
    }

    R Excecute(){
        return fun_();
    } 
private:
    std::function<R()> fun_;
};

#endif // _C11TEST_COMMAND_HPP_