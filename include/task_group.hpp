#ifndef _C11TEST_TASK_GROUP_HPP_
#define _C11TEST_TASK_GROUP_HPP_

#include <vector>
#include <string>
#include <map>
#include <utility>

#include "non_copyable.hpp"
#include "any.hpp"
#include "variant.hpp"

class TaskGroup : NonCopyable{
    typedef Variant<int,std::string,double,short,unsigned int> RetVariant;
public:
    TaskGroup(){}
    ~TaskGroup(){}

    template<typename R,class = typename std::enable_if<!std::is_same<R,void>::value>::type>
    void Run(Task<R()>&& task)
    {
        group_.emplace(R(),task.Run());
    }

    template<typename R,class = typename std::enable_if<!std::is_same<R,void>::value>::type>
    R Run(Task<R()>& task)
    {
        group_.emplace(R(),task.Run());
    }

    void Run(Task<void()>&& task)
    {
        void_group_.push_back(task.Run());
    }

    template<typename F>
    void Run(F&& f)
    {
        Run(Task<typename std::result_of<F()>::type()>(std::forward<F>(f)));
    }

    template<typename F,typename... Funs>
    void Run(F&& f,Funs... funs)
    {
        Run(std::forward<F>(f));
        Run(std::forward<Funs>(funs)...);
    }

    void Wait()
    {
        for(auto it = group_.begin(); it != group_.end(); ++it)
        {
            auto vrt = it->first;
            vrt.Visit([&](int a){FutureGet<int>(it->second);},
                        [&](double a){FutureGet<double>(it->second);},
                        [&](std::string a){FutureGet<std::string>(it->second);},
                        [&](short a){FutureGet<short>(it->second);},
                        [&](unsigned int a){FutureGet<unsigned int>(it->second);});
        }
        for(auto it = void_group_.begin(); it != void_group_.end(); ++it)
        {
            it->get();
        }
    }

private:
    template<typename T>
    void FutureGet(Any& f)
    {
        f.AnyCast<shared_future<T>>().get();
    }

private:
    multimap<RetVariant,Any> group_;
    vector<std::shared_future<void>> void_group_;
};

//当所有的task执行完。把所有task的future存起来。
//range为容器类型，value_type为task类型，return_type？
template<typename Range>
extern Task<vector<typename Range::value_type::ReturnType>()> WhenAll(Range& range)
{
    typedef typename Range::value_type::ReturnType Return_type;
    auto task = [&range]{
        vector<std::shared_future<Return_type>> fv;
        for(auto& task : range)
        {
            fv.emplace_back(task.Run());
        }
        vector<Return_type> v;
        for(auto& item : fv)
        {
            v.emplace_back(item.get());
        }
        return v;//这是返回的task调用get之后的返回值。
    };
    return Task<vector<Return_type>()>(task);//这是whenall的返回值
}

#endif // _C11TEST_TASK_GROUP_HPP_
