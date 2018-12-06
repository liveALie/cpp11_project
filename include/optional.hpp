#ifndef _C11TEST_OPTIONAL_HPP_
#define _C11TEST_OPTIONAL_HPP_

#include <type_traits>

//面对一个类型的对象
template<typename T>
class Optional{
    //指定大小和内存对齐大小的一块缓存类型
    using data_t = typename std::aligned_storage<sizeof(T),std::alignment_of<T>::value>::type;
public:
    Optional(){}
    //以T类型对象初始化
    Optional(const T& v)
    {
        Create(v);
    }
    //拷贝构造
    Optional(const Optional& other)
    {
        if(other.IsInit()){
            Assign(other);
        }
    }
    //复制运算符
    Optional& operator=(const Optional& other)
    {
        if(this == &other)
            return *this;
        if(other.IsInit()){
            Assign(other);
        }else{
            Destroy();
        }
        return *this;
    }

    ~Optional()
    {
        Destroy();
    }
    //以参数来重新初始化
    template<typename... Args>
    void Emplace(Args&&... args)
    {
        Destroy();
        Create(std::forward<Args>(args)...);
    }
    //是否已经初始化
    bool IsInit()const
    {
        return has_init_;
    }

    explicit operator bool()const//可以直接if(option){}
    {
        return IsInit();
    }

    //*op
    T const& operator*()const
    {
        if(IsInit()){
            return *((T*)(&data_));
        }
        throw std::logic_error("is not init.");
    }

private:
    //创建，并初始化
    template<typename... Args>
    void Create(Args&&... args)
    {
        new (&data_) T(std::forward<Args>(args)...);
        has_init_ = true;
    }
    //销毁
    void Destroy()
    {
        if(has_init_){
            has_init_ = false;
            ((T*)(&data_))->~T();
        }
    }
    //赋值
    void Assign(const Optional& other)
    {
        if(other.IsInit()){
            Copy(other.data_);
            has_init_ = true;
        }else{
            Destroy();
        }
    }
    //拷贝
    void Copy(const data_t& val)
    {
        Destroy();
        new (&data_) T((*(T*)(&val)));
    }
private:
    bool has_init_ = false;
    data_t data_;

};

#endif // _C11TEST_OPTIONAL_HPP_
