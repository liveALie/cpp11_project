#ifndef _C11TEST_TASK_HPP_
#define _C11TEST_TASK_HPP_

#include <functional>
#include <future>
#include <utility>

template<typename T>
class Task;

template<typename R,typename... Args>
class Task<R(Args...)>{
public:
    typedef R ReturnType;
    Task(std::function<R(Args...)>&& f):fun_(std::move(f)){}
    Task(std::function<const R(Args...)>& f) : fun_(f){}
    ~Task(){}
    void Wait()
    {
        std::async(fun_).wait();
    }

    R Get(Args&&... args)
    {
        return std::async(fun_,std::forward<Args>(args)...).get();
    }

    std::shared_future<R> Run()
    {
        return std::async(fun_);
    }

    //Then函数返回的是一个Task对象
    //auto Then(F&& f)->Task<typename std::result_of<F(R)>::type(Args...)>
    //上一句的函数生命，当R为void时，编译出错，详细原因有待考察。
    template<typename F>
    auto Then(F&& f)->Task<decltype(f(std::declval<R>()))(Args...)>
    {
        typedef typename std::result_of<F(R)>::type Return_type;
        auto func = std::move(fun_);
        //此为构造Task的入参，Then调用完之后调用Get，获取执行后的结果。
        return Task<Return_type(Args...)>([func,&f](Args... args)
        {
            std::future<R> last_future = std::async(func,std::forward<Args>(args)...);
            return std::async(f,last_future.get()).get();
        });
    }
private:
    std::function<R(Args...)> fun_;
};

#endif // _C11TEST_TASK_HPP_
