#ifndef _C11TEST_EVENTS_HPP_
#define _C11TEST_EVENTS_HPP_

#include <map>

#include "non_copyable.hpp"

template<class Func> //观察者类型为函数类型
class Events : public NonCopyable{
public:
    Events(){}
    ~Events(){}
    //此为类模板参数，不支持左值右值的自动推导，必须定义左值和右值的两个函数。
    int Connect(Func&& f){
        return Assign(f);
    }

    int Connect(const Func& f){
        return Assign(f);
    }

    void Disconnect(int key){
        connections_.erase(key); //这里可以直接删除key吗？可以 ，windows没问题。
    }

    template<class... Args> //模板参数列表，消除重复代码。
    void Notify(Args&&... args){
        for(auto& it : connections_){
            it.second(std::forward<Args>(args)...);
        }
    }

private:
    template<class F> //为什么这里也要定义一个函数模板，为了自动推导类型
    int Assign(F&& f){
        int k = observer_id_++;
        connections_.emplace(k,std::forward<F>(f));
        return k;
    }

private:
    int observer_id_ = 0;
    std::map<int, Func> connections_;
};

#endif // _C11TEST_EVENTS_HPP_