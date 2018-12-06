#ifndef _C11TEST_THREADPOOL_HPP_
#define _C11TEST_THREADPOOL_HPP_

#include <thread>
#include <list>
#include <functional>
#include <memory>
#include <atomic>
#include "sync_queue.hpp"

const int MAX_TASK_COUNT = 100;

//1，创建任务队列
//2，启动线程池
//3，添加任务
//4，停止任务队列
//5，停止线程池并销毁
//维护一个任务队列和一个线程列表
class ThreadPool{
public:
    using Task = std::function<void()>; ///一般的线程池任务是为此类型的。
    ThreadPool(int num_threads = std::thread::hardware_concurrency()) //线程的数量应和硬件的cpu数量相等。
        :queue_(MAX_TASK_COUNT)//创建任务队列
    {
        Start(num_threads); //默认启动
    }

    ~ThreadPool()
    {
        Stop(); ///保证忘记停止时，自动停止。
    }

    void Stop()
    {
        std::call_once(flag_,[this]{ StopThreadPool(); });//停止线程池，保证只调用一次。
    }
    //线程池主要的接口就是对外提供添加任务的接口，重载。
    void AddTask(Task&& task)
    {
        queue_.Put(std::forward<Task>(task));
    }

    void AddTask(const Task& task)
    {
        queue_.Put(task);
    }

private:
    //创建线程池。
    void Start(int num_threads)
    {
        running_ = true;
        for(int i = 0; i < num_threads; ++i)
        {
            //线程池中的线程对象用shared_ptr来管理
            thread_group_.push_back(std::make_shared<std::thread>(std::bind(&ThreadPool::RunThread,this)));
        } 
    }
    //每个线程执行的任务，work
    void RunThread()
    {
        while(running_)
        {
            std::list<Task> list;//任务队列提供用一个列表批量获取任务的接口。
            queue_.Take(list);
            for(auto& task : list)//执行task
            {
                if(!running_)
                {
                    return;
                }
                task();
            }
        }
    }
    //停止线程池
    void StopThreadPool()
    {
        queue_.Stop();//停止任务队列的添加和获取
        running_ = false;//标志位置为停止标志

        for(auto& thread : thread_group_)//c++11的基于范围的for循环
        {
            if(thread)
            {
                thread->join();
            }
        }
        thread_group_.clear();
    }

private:
    std::list<std::shared_ptr<std::thread>> thread_group_;//线程池
    SyncQueue<Task> queue_;//任务队列
    atomic_bool running_; //用于标志线程池开始和结束的标志位
    std::once_flag flag_; //用于call_once的标志位

};

#endif // _C11TEST_THREADPOOL_HPP_