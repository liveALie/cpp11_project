#include <iostream>
#include <functional>
#include <vector>
#include <numeric>

#include "singleton.hpp"
#include "events.hpp"
#include "classes.hpp"
#include "subject.hpp"
#include "command.hpp"
#include "object_pool.hpp"

#include "thread_pool.hpp"
#include "aspect.hpp"
#include "ioc_container.hpp"
#include "message_bus.hpp"
#include "variant.hpp"
#include "task.hpp"
#include "task_group.hpp"
#include "optional.hpp"

using namespace std;

void print(int a,int b)
{
    std::cout << a << "," << b << std::endl;
}

int add_one(int n)
{
    return n + 1;
}

void test_singleton()
{
    string str = "bb";
    Singleton<A>::Instance(str);
    Singleton<B>::Instance(std::move(str));
    Singleton<C>::Instance(1, 3.14);

    Singleton<C>::GetInstance()->Fun();

    Singleton<A>::DestroyInstance();
    Singleton<B>::DestroyInstance();
    Singleton<C>::DestroyInstance();
    //////////////////////以上为单列类测试
}

void test_observer()
{
     using Func_type = std::function<void(int,int)>;
    Events<Func_type> myevents;

    auto key = myevents.Connect(print);
    D d;
    myevents.Connect([&d](int a,int b){
        d.a = a;
        d.b = b;
    }); 

    Func_type f = std::bind(&D::print,&d,std::placeholders::_1,std::placeholders::_2);
    myevents.Connect(f);

    int a = 1, b = 2;
    myevents.Notify(a,b);
    myevents.Disconnect(key);
    //////////////////////////////////以上为观察者模式
}

void test_visitor()
{
    PrintVisitor vis;
    SubA sub_a;
    sub_a.val = 8.97;
    SubB sub_b;
    sub_b.val = 8;

    Base* base = &sub_a;
    base->Accept(vis);
    base = &sub_b;
    base->Accept(vis);
    ////////////////////////////////以上为访问者模式测试
}

void test_command()
{
    CommCommand<int> cmd;
    cmd.Wrap(add_one, 0);
    cmd.Wrap([](int n){ return n + 1;}, 1);
    // cmd.Wrap(bloop);
    // cmd.Wrap(bloop,4);
    E t = { 10 };
    int x = 3;
    cmd.Wrap(&E::triple0,&t);
    cmd.Wrap(&E::triple,&t,x);
    cmd.Wrap(&E::triple,&t,3);
    cmd.Wrap(&E::triple2,&t,5);

    auto r = cmd.Excecute();
    std::cout << "cmd.Excecute result:" << r << std::endl;
    ////////////////////////////////以上为命令模式的测试。
}

void print(std::shared_ptr<BigObject> p,const string& str)
{
    if(p)
    {
        p->print(str);
    }
}

void test_object_pool()
{
    std::cout << "test1" << std::endl;
    ObjectPool<BigObject> pool;
    std::cout << "test2" << std::endl;
    pool.Init(2);
    std::cout << "test3" << std::endl;

    {
        auto p = pool.Get();
        std::cout << "test4" << std::endl;
        print(p,"p");
        auto p2 = pool.Get();
        print(p,"p2");
    }
    auto p = pool.Get();
    auto p2 = pool.Get();

    print(p,"p");
    print(p2,"p2");
    ObjectPool<BigObject> pool2;
    pool2.Init(2,1);
    auto p4 = pool2.Get<int>();
    print(p4,"p4");
    std::cout << "test5" << std::endl;
    pool2.Init(2,3,6);
    auto p5 = pool2.Get<int,int>();
    print(p5,"p5");
    /////////////////这里阻塞是为什么？
}

void Ht(int a)
{
    cout << "real HT function:" << a << endl;
}

void test_aspect()
{
    Invoke<AA,BB>(&Ht,3);
}

void test_task()
{
    Task<int()> t([]{return 32;});
    auto r1 = t.Then([](int result){ std::cout << result << std::endl; return result + 3;}).Then([](int 
        result){std::cout << result << std::endl; return result + 3;}).Get();
    std::cout << r1 << std::endl;

    //此处不能用void，原因？？
    Task<void()> t1([](){std::cout << "hello world!" << std::endl;});
    auto r2 = t1.Run();
    r2.get();
}

void test_async()
{
    auto f = std::async([](){std::cout << "test async!" << std::endl;});
    std::cout << typeid(decltype(f)).name();
    f.get();
}

void test_taskgroup()
{
    TaskGroup g;
    g.Run([]{return 1;},[]{std::cout << "ok1" << std::endl;},[]{ std::cout << "ok2" << std::endl;});
    g.Wait();
}

void print_thread()
{
    std::cout << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

void test_whenall()
{
    vector<Task<int()>> v = {
        Task<int()>([]{print_thread();std::this_thread::sleep_for(std::chrono::seconds(1));return 1;}),
        Task<int()>([]{print_thread();return 2;}),
        Task<int()>([]{print_thread();return 3;}),
        Task<int()>([]{print_thread();return 4;})
    };

    std::cout << "when all" << std::endl;
    WhenAll(v).Then([](vector<int> results){
        std::cout << "sum:" << accumulate(begin(results),end(results),0) << std::endl;
    }).Wait();
}

void test_whenany()
{
    vector<Task<int()>> v = {
        Task<int()>([]{print_thread();std::this_thread::sleep_for(std::chrono::seconds(1));return 1;}),
        Task<int()>([]{print_thread();return 2;}),
        Task<int()>([]{print_thread();return 3;}),
        Task<int()>([]{print_thread();return 4;})
    };

    std::cout << "when any" << std::endl;
    WhenAny(v).Then([](const std::pair<int,int>& result){
        std::cout << "index:" << result.first << " result:" << result.second << std::endl;
        return result.second;
    }).Then([](int result){
        std::cout << "any result:" << result << std::endl;
    }).Wait();
    
}

bool check_prime(int x)
{
    for(int i = 2; i < x; ++i)
    {
        if(x % i == 0)
            return false;
        return true;
    }
    return false;
}

void test_parallelfor()
{
    vector<int> v;
    const int size = 1000;
    for(int i = 0; i < size; ++i)
    {
        v.push_back(i + 1);
    }
    auto begin = v.begin();
    auto end = v.end();
    ParallelForeach<vector<int>::iterator>(begin,end,check_prime);
}

void test_parallelsum()
{
    vector<int> v;
    const int size = 100000000;
    for(int i = 0; i < size; ++i)
    {
        v.push_back(i + 1);
    }
    int i = 0;
    auto r = ParallelReduce(v,i,[](const vector<int>::iterator& begin,const vector<int>::iterator& end,int val){
        return std::accumulate(begin,end,val);
    });
    std::cout << "test_parallelsum result:" << r << std::endl;
}

struct TestClass{
public:
    TestClass(int& val,int* v):val_(val),v_(v){};
    void operator()()const{
        std::cout << "testclass val_:" << ++val_ << std::endl;
        std::cout << "testclass v_:" << ++*v_ << std::endl;
    }
private:
    int& val_;
    int* v_;
};

void test_lambda()
{
    int i = 1;
    int j = 3;
    int k = 5;
    auto f = [i,&j](){ 
        ++j;
        std::cout << "i:" << i << " j:" << j << std::endl; 
    };
    f();

    TestClass tc(i,&k);
    tc();
}

void test_optional()
{
    Optional<std::string> a("ok");
    Optional<std::string> b("ok");
    Optional<std::string> c("aa");
    std::cout << "test_optional original c:" << *c << std::endl;
    c = a;
    std::cout << "test_optional new c:" << *c << std::endl;

    Optional<MyStruct> op;
    op.Emplace(1,2);
    if(op){
        std::cout << "test optional op a:" << (*op).a_ << ",b:" << (*op).b_ << std::endl;
    }
}

int Foo(int x)
{
    return x * 2;
}

void test_lazy()
{
    int y = 4;
    auto lazyer1 = lazy(Foo,y);
    std::cout << "test lazyer1:" << lazyer1.Value() << std::endl;

    Lazy<int> lazyer2 = lazy([]{return 12;});
    std::cout << "test lazyer2:" << lazyer2.Value() << std::endl;
    MyStruct t;
    t.Load();
}

int main(int argc,char* argv[])
{
    std::cout << "hello world." << std::endl;

    test_singleton();
    test_observer();
    test_visitor();
    test_command();
    //test_object_pool();
    test_aspect();
    test_task();
    test_async();
    test_taskgroup();
    test_whenall();
    test_whenany();
    test_parallelfor();
    test_parallelsum();
    test_lambda();
    test_optional();
    test_lazy();
    std::cout << "test over!" << std::endl;
    return 0;
}