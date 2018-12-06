#ifndef _C11TEST_CLASSES_HPP_
#define _C11TEST_CLASSES_HPP_

#include <iostream>
#include <string>
#include <memory>

#include "lazy.hpp"

using namespace std;

class A{
public:
    A(const string&){ cout << "lvalue" << endl; }
    A(string&&){ cout << "rvalue" << endl; }
};

class B{
public:
    B(const string&){ cout << "lvalue" << endl; }
    B(string&&){ cout << "rvalue" << endl; }
};

class C{
public:
    C(int x,double y){}
    void Fun(){ cout << "test" << endl; }
};
/////////////////////////////以上用于单例类的测试

struct D{
    int a,b;
    void print(int a,int b){cout << a << "," << b << endl;}
};
///////////////////////////////以上用于观察者模式测试类

struct E{
    int a;
    int operator()(){ return a; }
    int value(){return a;}
    //int operator()(int n){ return a + n; }
    int triple0(){ return a * 3; }
    int triple(int n){ return a * 3 + n; }
    int triple1()const{ return a * 3; }
    const int triple2(int n)const { return a *3 + n; }
    void triple3(){ cout << "" << endl; }
};
///////////////////////////////////以上用于命令模式的测试

struct BigObject{
    BigObject(){}
    BigObject(int a){}
    BigObject(const int& a,const int& b){}
    void print(const string& str){
        cout << str << endl;
    }
};
//////////////////////////////以上用于对象池的测试。

struct AA{
    void Before(int i)
    {
        cout << "Before from AA" << i << endl;
    }
    void After(int i)
    {
        cout << "After from AA" << i << endl;
    }
};

struct BB{
    void Before(int i)
    {
        cout << "Before from BB" << i << endl;
    }
    void After(int i)
    {
        cout << "After from BB" << i << endl;
    }
};

struct CC{
    void Before()
    {
        cout << "Before from CC" << endl;
    }
    void After()
    {
        cout << "After from CC" << endl;
    }
};

struct DD{
    void Before()
    {
        cout << "Before from DD" << endl;
    }
    void After()
    {
        cout << "After from DD" << endl;
    }
};
/////////////以上测试切面

struct BigObject2{
    BigObject2(){
        std::cout << "lazy load big object2!" << std::endl;
    }
};
/////////////lazy

struct MyStruct{
    MyStruct()
        :a_(0),b_(0)
    { 
        obj_ = lazy([]{
            return make_shared<BigObject2>();
        });
    }
    MyStruct(int a,int b):a_(a),b_(b){}
    void Load()
    {
        obj_.Value();
    }

    int a_;
    int b_;
    Lazy<std::shared_ptr<BigObject2>> obj_;
};
///////////////以上测试optional





#endif //_C11TEST_CLASSES_HPP_
