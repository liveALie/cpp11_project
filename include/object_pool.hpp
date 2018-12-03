#ifndef _C11TEST_OBJECTPOOL_HPP_
#define _C11TEST_OBJECTPOOL_HPP_

#include <string>
#include <functional>
#include <memory>
#include <map>
#include <iostream>

#include "non_copyable.hpp"

using namespace std;

const int MAX_OBJECT_NUM = 10;

template<class T>
class ObjectPool : NonCopyable {
    template<typename... Args>
    using Constructor = std::function<shared_ptr<T>(Args...)>;
public:
    //对象池的初始化，可变参数模板函数
    template<typename... Args>
    void Init(int size, Args&&... args){
        std::cout << "pool1" << std::endl;
        if(size <=0 || size > MAX_OBJECT_NUM){
            throw logic_error("object num out of range.");
        }
        std::cout << "pool2" << std::endl;
        auto constructName = typeid(Constructor<Args...>).name();
        std::cout << "pool3 constructName" << constructName << std::endl;
        for(int i = 0; i < size; ++i){
            std::cout << "pool4" << std::endl;
            object_map_.emplace(constructName,shared_ptr<T>(new T(std::forward<Args>(args)...),
                [this,constructName](T* p){
                    std::cout << "shanchuqi " << constructName << std::endl;
                    object_map_.emplace(std::move(constructName),shared_ptr<T>(p));
                }
            ));
        }
    }

    //取对象
    template<class... Args>
    shared_ptr<T> Get(){
        std::cout << "get1" << std::endl;
        string constructName = typeid(Constructor<Args...>).name();
        std::cout << "get2 constructName " << constructName << std::endl;
        auto range = object_map_.equal_range(constructName); ////////equal_range的用法。
        std::cout << "get3" << std::endl;
        for(auto it = range.first; it != range.second; ++it){
            auto ptr = it->second;
            std::cout << "get4" << std::endl;
            object_map_.erase(it);
            std::cout << "get5" << std::endl;
            return ptr;
        }
        std::cout << "return nullptr" << std::endl;
        return nullptr;
    }
private:
    multimap<string,std::shared_ptr<T>> object_map_;
};

#endif // _C11TEST_OBJECTPOOL_HPP_