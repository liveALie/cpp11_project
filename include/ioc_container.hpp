#ifndef _C11TEST_IOCCONTAINER_HPP_
#define _C11TEST_IOCCONTAINER_HPP_

#include <unordered_map>
#include <functional>
#include <string>
#include <memory>

#include "any.hpp"
#include "non_copyable.hpp"

class IocContainer : public NonCopyable{
public:
    IocContainer(){}
    ~IocContainer(){}
    
    template<typename T,typename Depend,typename... Args>
    typename std::enable_if<!std::is_base_of<T,Depend>::value>::type RegisterType(const std::string& key)
    {
        std::function<T*(Args...)> function = [](Args... args){ return new T(new Depend(args...)); };
        RegisterType(key,function);
    }

    template<typename T,typename Depend,typename... Args>
    typename std::enable_if<std::is_base_of<T,Depend>::value>::type RegisterType(const std::string& key)
    {
        std::function<T*(Args...)> function = [](Args... args){ return new Depend(args...); };
        RegisterType(key,function);
    }

    template<typename T,typename... Args>
    void RegisterSimple(const std::string& key)
    {
        std::function<T*(Args...)> function = [](Args... args){ return new T(args...); };
        RegisterType(key,function);
    }

    template<class T,typename... Args>
    T* Resolve(const std::string& key,Args&&... args)
    {
        auto it = creaters_.find(key);
        if(it == creaters_.end())
            return nullptr;
        Any resolver = it->second;
        std::function<T*(Args...)> function = resolver.AnyCast<std::function<T*(Args...)>>();
        return function(args...);
    }

    template<typename T,typename... Args>
    std::shared_ptr<T> ResolveShared(const std::string& key,Args... args)
    {
        T* ptr = Resolve<T>(key,args...);
        return std::shared_ptr<T>(ptr);
    }
    
private:
    void RegisterType(const std::string& key,Any creater)
    {
        if(creaters_.find(key) != creaters_.end())
            throw std::invalid_argument("this key has already exist!");
        creaters_.emplace(key,creater);
    }

private:
    std::unordered_map<std::string,Any> creaters_;
};

#endif // _C11TEST_IOCCONTAINER_HPP_