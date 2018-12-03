#ifndef _C11TEST_FUNCTION_TRAITS_HPP_
#define _C11TEST_FUNCTION_TRAITS_HPP_

#include <functional>
#include <tuple>

//转换为函数指针或std::fucntion
template<typename T>
struct function_traits;
//普通函数
template<typename Ret, typename... Args>
struct function_traits<Ret(Args...)>{
    enum{arity = sizeof...(Args)};
    typedef Ret function_type(Args...);
    typedef Ret return_type;
    using stl_function_type = std::function<function_type>;//这里的function_type？？
    typedef Ret(*pointer)(Args...);

    template<size_t I>
    struct args{
        static_assert( I < arity,"index is out of range,index must less than sizeof Args.");
        using type = typename std::tuple_element<I,std::tuple<Args...>>::type;
    };

    typedef std::tuple<Args...> ArgTupleType;
};
//函数指针
template<typename Ret,typename... Args>
struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)>{};

//std::function
template<typename Ret,typename... Args>
struct function_traits<std::function<Ret(Args...)>> : function_traits<Ret(Args...)>{};

//成员函数
#define FUNCTION_TRAITS(...) \
    template<typename ReturnType,typename C,typename... Args>\
    struct function_traits<ReturnType(C::*)(Args...) __VA_ARGS__> : function_traits<ReturnType(Args...)>{}; \

FUNCTION_TRAITS()
FUNCTION_TRAITS(const)
FUNCTION_TRAITS(volatile)
FUNCTION_TRAITS(const volatile)

//函数对象
template<typename Callable>
struct function_traits : function_traits<decltype(&Callable::operator())>{};

//由参数类型隐式指定
template<typename Function>
typename function_traits<Function>::stl_function_type to_function(const Function& lambda)
{
    return static_cast<typename function_traits<Function>::stl_function_type>(lambda);
}

template<typename Function>
typename function_traits<Function>::stl_function_type to_function(Function&& lambda)
{
    return static_cast<typename function_traits<Function>::stl_function_type>(std::forward<Function>(lambda));
}

template<typename Function>
typename function_traits<Function>::pointer to_function_pointer(const Function& lambda)
{
    return static_cast<typename function_traits<Function>::pointer>(lambda);
}

#endif // _C11TEST_FUNCTION_TRAIT_HPP_