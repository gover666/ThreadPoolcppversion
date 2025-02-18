#pragma once
#include<pthread.h>
#include<unistd.h>
#include"TaskQueue.h"
#include"TaskQueue.cpp"


template<typename T>
class ThreadPool
{
public:
    ThreadPool(int min, int max);
    ~ThreadPool();

    //给线程池添加任务
    void threadPoolAdd(Task<T> t);

    // 获取线程池中工作的线程的个数
    int threadPoolBusyNum();

    // 获取线程池中活着的线程的个数
    int threadPoolAliveNum();

    // 工作的线程(消费者线程)任务函数
    static void* worker(void* arg);

    // 管理者线程任务函数
    static void* manager(void* arg);

    // 单个线程退出
    void threadExit();


private:
    TaskQueue<T>* taskQ;               //任务队列
    pthread_t managerID;            // 管理者线程ID
    pthread_t* threadIDs;
    pthread_cond_t notEmpty;        // 工作的线程ID
    pthread_mutex_t mutexPool;      // 锁整个的线程池
    int minNum;                     // 最小线程数量
    int maxNum;                     // 最大线程数量
    int busyNum;                    // 忙的线程的个数
    int liveNum;                    // 存活的线程的个数
    int exitNum;                    // 要销毁的线程个数
    static const int NUMBER = 2;
    bool shutdown=false;                  //判断线程池是否关闭
    
};

