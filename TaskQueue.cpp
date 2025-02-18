#include "TaskQueue.h"
template<typename T>
TaskQueue<T>::TaskQueue()
{
	pthread_mutex_init(&m_mutex,NULL);
}

template<typename T>
TaskQueue<T>::~TaskQueue()
{
	pthread_mutex_destroy(&m_mutex);
}

template<typename T>
void TaskQueue<T>::addTask(Task<T> t)
{
	pthread_mutex_lock(&m_mutex);
	m_TaskQ.push(t);
	pthread_mutex_unlock(&m_mutex);
}

template<typename T>
void TaskQueue<T>::addTask(callback f, void* arg)
{
	pthread_mutex_lock(&m_mutex);
	Task<T> task;
	task.function = f;
	task.arg = arg;
	m_TaskQ.push(task);
	pthread_mutex_unlock(&m_mutex);
}

template<typename T>
Task<T> TaskQueue<T>::gettask()
{
	Task<T> t;
	pthread_mutex_lock(&m_mutex);
	if (m_TaskQ.size() > 0)
	{
		t= m_TaskQ.front();
		m_TaskQ.pop();
	}
	pthread_mutex_unlock(&m_mutex);
	return t;
}

