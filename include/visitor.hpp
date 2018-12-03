#ifndef _C11TEST_VISITOR_HPP_
#define _C11TEST_VISITOR_HPP_

//////////为什么写三个visitor？？ 这是可变参数模板类的用法  回头看书！！
//这是可变参数模板类的解参数包的方式，1、递归 + 特化，2、特化 + 继承。这属于第二种方式。
template<typename... Types>
struct Visitor;

template<typename T,typename... Types>
struct Visitor<T,Types...> : Visitor<Types...>{
    using Visitor<Types...>::Visit;
    virtual void Visit(const T&) = 0;
};

template<typename T>
struct Visitor<T>{
    virtual void Visit(const T& t) = 0;
};

#endif // _C11TEST_VISITOR_HPP_