#include "ThreadPool.h"
#include"TaskQueue.h"
#include<string>
#include<string.h>
#include <iostream>
using namespace std;
ThreadPool::ThreadPool(int min, int max)
{
    taskQ = new TaskQueue;
    do
    {
        minNum = min;
        maxNum = max;
        busyNum = 0;
        liveNum = min;    // 和最小个数相等
        
        
        if (taskQ == nullptr)
        {
            cout << "malloc task error"<<endl;
            break;
        }

        //根据线程最大上限给线程数组分类内存
        threadIDs = new pthread_t[max];
        if (threadIDs ==nullptr)
        {
            cout<<"malloc threadIDs fail...\n";
            break;
        }

        memset(threadIDs, 0, sizeof(pthread_t) * max);
        if (pthread_mutex_init(&mutexPool, NULL) != 0 ||
            pthread_cond_init(&notEmpty, NULL) != 0)
        {
            cout<<"mutex or condition init fail...\n";
            break;
        }

     
        
        

        shutdown = false;

        // 创建线程
        pthread_create(&managerID, NULL, manager,this);
        for (int i = 0; i < min; ++i)
        {
            pthread_create(&threadIDs[i], NULL, worker,this);
        }
        return;
    } while (0);

    // 释放资源
    if (threadIDs) delete[]threadIDs;
    if (taskQ) delete taskQ;

}


ThreadPool::~ThreadPool()
{
  

    // 关闭线程池
    shutdown = true;
    // 阻塞回收管理者线程
    pthread_join(managerID, NULL);
    // 唤醒阻塞的消费者线程
    for (int i = 0; i < liveNum; ++i)
    {
        pthread_cond_signal(&notEmpty);
    }
    // 释放堆内存
    if (taskQ)
    {
        delete taskQ;
    }
    if (threadIDs)
    {
        delete[] threadIDs;
    }

    pthread_mutex_destroy(&mutexPool);

    pthread_cond_destroy(&notEmpty);
}


void ThreadPool::threadPoolAdd(Task t)
{

   
    if (shutdown)
    {
        return;
    }
    // 添加任务
    taskQ->addTask(t);

    pthread_cond_signal(&notEmpty);
  
}


int ThreadPool::threadPoolBusyNum()
{
    pthread_mutex_lock(&mutexPool);
    int busyNum = this->busyNum;
    pthread_mutex_unlock(&mutexPool);
    return busyNum;
}


int ThreadPool::threadPoolAliveNum()
{
    pthread_mutex_lock(&mutexPool);
    int liveNum = this->liveNum;
    pthread_mutex_unlock(&mutexPool);
    return liveNum;
}


void* ThreadPool::worker(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);

    while (true)
    {
        pthread_mutex_lock(&pool->mutexPool);
        // 当前任务队列是否为空
        while (pool->taskQ->getTaskNum() == 0 && !pool->shutdown)
        {
            cout << "thread " << to_string(pthread_self()) << " waiting..." << endl;

            // 阻塞工作线程
            pthread_cond_wait(&pool->notEmpty, &pool->mutexPool);

            // 判断是不是要销毁线程
            if (pool->exitNum > 0)
            {
                pool->exitNum--;
                if (pool->liveNum > pool->minNum)
                {
                    pool->liveNum--;
                    pthread_mutex_unlock(&pool->mutexPool);
                    pool->threadExit();
                }
            }
        }

        // 判断线程池是否被关闭了
        if (pool->shutdown)
        {
            pthread_mutex_unlock(&pool->mutexPool);
            pool->threadExit();
        }

        // 从任务队列中取出一个任务
        Task task=pool->taskQ->gettask();
       
        pool->busyNum++;
        pthread_mutex_unlock(&pool->mutexPool);

        cout << "thread" << to_string(pthread_self()) << "start working...\n";
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;

        cout << "thread" << to_string(pthread_self()) << "end working...\n";
        pthread_mutex_lock(&pool->mutexPool);
        pool->busyNum--;
        pthread_mutex_unlock(&pool->mutexPool);
    }
    return nullptr;


}


void* ThreadPool::manager(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while (!pool->shutdown)
    {
        // 每隔5s检测一次
        sleep(5);

        // 取出线程池中任务的数量和当前线程的数量
        pthread_mutex_lock(&pool->mutexPool);
        int queueSize = pool->taskQ->getTaskNum();
        int liveNum = pool->liveNum;
        int busyNum = pool->busyNum;
        pthread_mutex_unlock(&pool->mutexPool);

        

        // 添加线程
        // 任务的个数>存活的线程个数 && 存活的线程数<最大线程数
        if (queueSize > liveNum && liveNum < pool->maxNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            int counter = 0;
            for (int i = 0; i < pool->maxNum && counter < NUMBER
                && pool->liveNum < pool->maxNum; ++i)
            {
                if (pool->threadIDs[i] == 0)
                {
                    pthread_create(&pool->threadIDs[i], NULL, worker, pool);
                    counter++;
                    pool->liveNum++;
                }
            }
            pthread_mutex_unlock(&pool->mutexPool);
        }
        // 销毁线程
        // 忙的线程*2 < 存活的线程数 && 存活的线程>最小线程数
        if (busyNum * 2 < liveNum && liveNum > pool->minNum)
        {
            pthread_mutex_lock(&pool->mutexPool);
            pool->exitNum = NUMBER;
            pthread_mutex_unlock(&pool->mutexPool);
            // 让工作的线程自杀
            for (int i = 0; i < NUMBER; ++i)
            {
                pthread_cond_signal(&pool->notEmpty);
            }
        }
    }
    return nullptr;
}


void ThreadPool::threadExit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < maxNum; ++i)
    {
        if (threadIDs[i] == tid)
        {
            threadIDs[i] = 0;
            cout<<"threadExit() called, "<<to_string(tid)<<"exiting...\n";
            break;
        }
    }
    pthread_exit(NULL);
}
