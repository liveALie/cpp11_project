#ifndef _C11TEST_MESSAGE_BUS_HPP_
#define _C11TEST_MESSAGE_BUS_HPP_

#include <string>
#include <map>
#include <functional>

#include "any.hpp"
#include "function_traits.hpp"
#include "non_copyable.hpp"

class MessageBus : NonCopyable{
public:
    template<typename F>
    void Attach(F&& f,const std::string& topic = "")
    {
        auto func = to_function(std::forward<F>(f));
        Add(topic,std::move(func));
    }

    template<typename R>
    void SendReq(const std::string& topic)
    {
        using function_type = std::function<R()>;
        std::string msg_type = topic + typeid(function_type).name();
        auto range = map_.equal_range(msg_type);
        for(Iterater it = range.first; it != range.second; ++it)
        {
            auto f = it->second.AnyCast<function_type>;
            f();
        }
    }

    template<typename R,typename... Args>
    void SendReq(Args&&... args,const std::string& topic = "")
    {
        using function_type = std::function<R(Args...)>;
        std::string msg_type = topic + typeid(function_type).name();
        auto range = map_.equal_range(msg_type);
        for(Iterater it = range.first; it != range.second; ++it)
        {
            auto f = it->second.AnyCast<function_type>;
            f(std::forward<Args>(args)...);
        }
    }
    
    template<typename R,typename... Args>
    void Remove(const std::string& topic = "")
    {
        using function_type = std::function<R(Args...)>;
        std::string msg_type = topic + typeid(function_type).name();
        auto range = map_.equal_range(msg_type);
        map_.erase(range.first,range.second);
    }

private:
    template<typename Function>
    void Add(const std::string& topic,Function&& f)
    {
        std::string msg_type = topic + typeid(Function).name();
        map_.emplace(msg_type,std::forward<Function>(f));
    }
private:
    std::multimap<std::string,Any> map_;
    typedef std::multimap<std::string,Any>::iterator Iterater;
};

#endif // _C11TEST_MESSAGE_BUS_HPP_