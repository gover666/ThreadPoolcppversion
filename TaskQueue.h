#pragma once
#include<queue>
#include<pthread.h>

//定义任务结构体
using callback = void(*)(void* arg);
template<typename T>
struct Task
{
	Task()
	{
		function = nullptr;
		arg = nullptr;
	}
	Task(callback f, void* arg)
	{
		function=f;
		this->arg = (int*)arg;
	}
	callback function;
	T* arg;
};

//任务队列
template<typename T>
class TaskQueue
{
public:
	TaskQueue();
	~TaskQueue();

	//添加任务
	void addTask(Task<T> t);
	void addTask(callback f, void* arg);

	//取出一个任务
	Task<T> gettask();

	//获取当前任务数量
	inline size_t getTaskNum()
	{
		return m_TaskQ.size();
	}
private:
	pthread_mutex_t m_mutex;
	std::queue<Task<T>> m_TaskQ;
};

