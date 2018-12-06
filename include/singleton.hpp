#ifndef _C11TEST_SINGLETON_HPP_
#define _C11TEST_SINGLETON_HPP_

#include <utility>
#include <stdexcept>

template<class T>
class Singleton
{
public:
    //用来构造T的实例
    template<typename... Args>
    static T* Instance(Args&&... args){
        if(!instance_){
            instance_ = new T(std::forward<Args>(args)...);
        }
        return instance_;
    }
    //获取实例
    static T* GetInstance(){
        if(!instance_){
            throw std::logic_error("the instance is not init,please initialize the instance first.");
        }
        return instance_;
    }
    //销毁单利对象
    static void DestroyInstance(){
        if(instance_){
            delete instance_;
            instance_ = nullptr;
        }
    }

private:
    Singleton();
    virtual ~Singleton();
    Singleton(const Singleton&);
    Singleton& operator=(const Singleton&);
    
private:
    static T* instance_;
};

template<class T> T* Singleton<T>::instance_ = nullptr;

#endif // _C11TEST_SINGLETON_HPP_
