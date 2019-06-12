#pragma once

#include <iostream>
#include <memory>
#include <typeindex>
#include <utility>

struct Any {
public:
  Any(void) : type_index_(std::type_index(typeid(void))) {}

  //两种拷贝构造函数
  Any(const Any &that) : ptr_(that.Clone()), type_index_(that.type_index_) {}

  Any(Any &&that) : ptr_(std::move(that.ptr_)), type_index_(that.type_index_) {}

  //为了区分Any和U类型
  template <
      typename U,
      class = typename std::enable_if<
          !std::is_same<typename std::decay<U>::type, Any>::value, U>::type>
  Any(U &&value)
      : ptr_(new Derived<typename std::decay<U>::type>(std::forward<U>(value))),
        type_index_(std::type_index(typeid(typename std::decay<U>::type))) {}

  bool IsNull() const { return !bool(ptr_); }

  string GetTypeIndexName() { return type_index_.name(); }

  template <class U> bool Is() const {
    return type_index_ == std::type_index(typeid(U));
  }

  //这个函数调用没有参数，所以必须指定函数模板参数。
  template <typename U> U &AnyCast() {
    if (!Is<U>()) {
      std::cout << "can not cast " << type_index_.name() << " to "
                << typeid(U).name() << std::endl;
      throw std::bad_cast();
    }
    auto derived = dynamic_cast<Derived<U> *>(ptr_.get());
    return derived->value_;
  }

  //重载赋值
  Any &operator=(const Any &a) {
    // std::cout << "operator= is used." << std::endl;
    if (this == &a)
      return *this;
    ptr_ = a.Clone(); //这里怎么可以直接赋值呢,可以std::unique_ptr<T> a; a =
                      //std::unique_ptr<T>(new T);
    type_index_ = a.type_index_;
    // std::cout << "Any& operator=(const Any& a) is used." << std::endl;
    return *this;
  }

private:
  struct Base;
  typedef std::unique_ptr<Base> BasePtr;

  //这是一个通用的基类，可以说是接口
  struct Base {
    virtual ~Base(){};
    virtual BasePtr Clone() const = 0;
  };

  //此模板类继承Base，可以实现任何类型。
  template <typename T> struct Derived : Base {
    //此处用模板类型推导，所以形参用&&,使用参数时用std::forward完美转发。
    template <typename U>
    Derived(U &&value)
        : value_(std::forward<U>(
              value)) //这个U类型是value_的构造参数，或者拷贝构造参数
    {}
    //以自己的value为参数，构造一个新的baseptr
    BasePtr Clone() const {
      return BasePtr(new Derived<T>(
          value_)); //创建了一个新的指针，与被克隆的是两个只能指针。
    }

    T value_;
  };

  //主要是value的克隆。
  BasePtr Clone() const {
    if (ptr_) {
      return ptr_->Clone();
    }
    return nullptr;
  }

private:
  BasePtr ptr_;
  std::type_index type_index_;
};