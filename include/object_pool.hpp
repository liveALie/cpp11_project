#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "non_copyable.hpp"

using namespace std;

const int MAX_OBJECT_NUM = 10;

template <class T> class ObjectPool : NonCopyable {
  //定义了一个模板别名
  template <typename... Args>
  using Constructor = std::function<shared_ptr<T>(Args...)>;

public:
  ObjectPool() : is_desctructing_(false) {}
  ~ObjectPool() { is_desctructing_ = true; }

  //对象池的初始化，可变参数模板函数
  //最好显示指定参数类型
  template <typename... Args> void Init(int size, Args &&... args) {
    if (size <= 0 || size > MAX_OBJECT_NUM) {
      throw logic_error("object num out of range.");
    }
    //以构造函数的类型标识为key值，存储所有创建的shared_ptr对象
    //函数类型为shared_ptr<T>(Args...)
    auto constructName = typeid(Constructor<Args...>).name();
    for (int i = 0; i < size; ++i) {
      object_map_.emplace(constructName,
                          shared_ptr<T>(new T(std::forward<Args>(args)...),
                                        [this, constructName](T *p) {
                                          if (!this->is_desctructing_) {
                                            object_map_.emplace(
                                                std::move(constructName),
                                                shared_ptr<T>(p));
                                          } else {
                                            delete p;
                                          }

                                        }));
    }
  }

  //取对象
  template <class... Args> shared_ptr<T> Get() {
    string constructName = typeid(Constructor<Args...>).name();
    auto range =
        object_map_.equal_range(constructName); ////////equal_range的用法。
    for (auto it = range.first; it != range.second; ++it) {
      auto ptr = it->second;
      object_map_.erase(it);
      return ptr;
    }
    return nullptr;
  }

private:
  bool is_desctructing_;
  multimap<string, std::shared_ptr<T>> object_map_;
};