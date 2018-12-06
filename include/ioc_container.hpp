#ifndef _C11TEST_IOCCONTAINER_HPP_
#define _C11TEST_IOCCONTAINER_HPP_

#include <unordered_map>
#include <functional>
#include <string>
#include <memory>
#include <iostream>

#include "any.hpp"
#include "non_copyable.hpp"
//用any进行类型擦除
class IocContainer : public NonCopyable{
public:
    IocContainer(){}
    ~IocContainer(){}
    //注册构造函数，有依赖对象的构造函数
    template<typename T,typename Depend,typename... Args>
    typename std::enable_if<!std::is_base_of<T,Depend>::value>::type RegisterType(const std::string& key)
    {
        std::function<T*(Args...)> func = [](Args... args){ return new T(new Depend(args...)); };
        RegisterType(key,func);
    }

    //是返回值类型与对象类型是继承关系的构造
    template<typename T,typename Depend,typename... Args>
    typename std::enable_if<std::is_base_of<T,Depend>::value>::type RegisterType(const std::string& key)
    {
        //std::cout << "<is base of>RegisterType is used." << std::endl;
        std::function<T*(Args...)> func = [](Args... args){ return new Depend(args...); };
        //std::cout << "register type:" << typeid(decltype(func)).name() << std::endl;
        //std::cout << "register type:" << typeid(std::function<T*(Args...)>).name() << std::endl;
        RegisterType(key,func);
    }
    //注册构造普通类型的对象的构造函数
    template<typename T,typename... Args>
    void RegisterSimple(const std::string& key)
    {
        std::function<T*(Args...)> func = [](Args... args){ return new T(args...); };
        RegisterType(key,func);
    }
    //以上三个注册构造函数，都包装成T*(Args...)的对象函数，擦除类型为Any类型。

    //解析，去除Any类型的构造函数，转换为真是
    template<typename T,typename... Args>
    T* Resolve(const std::string& key,Args... args)//这里写成了右值引用，造成了运行错误。，因为其他用的都是左值
    {
        auto it = creaters_.find(key);
        if(it == creaters_.end())
            return nullptr;
        Any resolver = it->second;//调用了拷贝构造函数
        //std::cout << "resolve resolver type index name:" << resolver.GetTypeIndexName() << std::endl;
        //根据模板参数，转换为T*（Args...）类型的构造函数
        std::function<T*(Args...)> func = resolver.AnyCast<std::function<T*(Args...)>>();
        //std::cout << "resolve any_cast to:" << typeid(std::function<T*(Args...)>).name() << std::endl;
        return func(args...);
    }
    //返回的智能指针类型，可自动释放
    template<typename T,typename... Args>
    std::shared_ptr<T> ResolveShared(const std::string& key,Args... args)
    {
        T* ptr = Resolve<T,Args...>(key,args...);
        return std::shared_ptr<T>(ptr);
    }

    //注意，以上两个Resolve函数，必须显示指定模板参数，与注册时一致，
    //否则用参数自动推导出来的类型可能和注册时不一致！！！！！！！！！！！！！！！！！！！
    
private:
    //通用的注册构造函数
    void RegisterType(const std::string& key,Any creater)
    {
        //std::cout << "RegisterType type index name:" << creater.GetTypeIndexName() << std::endl;
        if(creaters_.find(key) != creaters_.end())
            throw std::invalid_argument("this key has already exist!");
        creaters_.emplace(key,creater);//调用了Any的拷贝构造函数
    }

private:
    std::unordered_map<std::string,Any> creaters_;
};

#endif // _C11TEST_IOCCONTAINER_HPP_