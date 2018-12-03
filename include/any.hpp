#ifndef _C11TEST_ANY_HPP_
#define _C11TEST_ANY_HPP_

#include <memory>
#include <typeindex>
#include <iostream>
#include <utility>

struct Any{
public:
    Any(void)
        :type_index_(std::type_index(typeid(void)))
    {

    }

    Any(const Any& that)
        :ptr_(that.Clone()),type_index_(typeid(that.type_index_))
    {

    }

    Any(Any&& that)
        :ptr_(std::move(that.ptr_)),type_index_(that.type_index_)
    {

    }

    template<typename U,class = typename std::enable_if<!std::is_same<typename std::decay<U>::type,Any>::value,U>::type>
    Any(U&& value)
        :ptr_(new Derived<typename std::decay<U>::type>(std::forward<U>(value))),
        type_index_(std::type_index(typeid(typename std::decay<U>::type)))
    {

    }

    bool IsNull()const
    {
        return !bool(ptr_);
    }

    template<class U>
    bool Is()const
    {
        return type_index_ == std::type_index(typeid(U));
    }

    template<typename U>
    U& AnyCast()
    {
        if(!Is<U>())
        {
            std::cout << "can not cast " << type_index_.name() << "to " << typeid(U).name() << std::endl;
            throw std::bad_cast();
        }
        auto derived = dynamic_cast<Derived<U>*>(ptr_.get());
        return derived->value_;
    }

    Any& operator=(const Any& a)
    {
        if(ptr_ == a.ptr_)
            return *this;
        ptr_ = a.Clone();
        type_index_ = a.type_index_;
        return *this;
    }
private:
    struct Base;
    typedef std::unique_ptr<Base> BasePtr;

    struct Base{
        virtual ~Base(){};
        virtual BasePtr Clone()const = 0;
    };

    template<typename T>
    struct Derived : Base{
        template<typename U>
        Derived(U&& value)
            :value_(std::forward<U>(value))
        {        
        }

        BasePtr Clone()const
        {
            return BasePtr(new Derived<T>(value_));
        }

        T value_;
    };

    BasePtr Clone()const
    {
        if(ptr_)
        {
            return ptr_->Clone();
        }
        return nullptr;
    }

private:
    BasePtr ptr_;
    std::type_index type_index_;
};

#endif // _C11TEST_ANY_HPP_