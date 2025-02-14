#include "TaskQueue.h"

TaskQueue::TaskQueue()
{
	pthread_mutex_init(&m_mutex,NULL);
}

TaskQueue::~TaskQueue()
{
	pthread_mutex_destroy(&m_mutex);
}
void TaskQueue::addTask(Task& t)
{
	pthread_mutex_lock(&m_mutex);
	m_TaskQ.push(t);
	pthread_mutex_unlock(&m_mutex);
}


void TaskQueue::addTask(callback f, void* arg)
{
	pthread_mutex_lock(&m_mutex);
	Task task;
	task.function = f;
	task.arg = arg;
	m_TaskQ.push(task);
	pthread_mutex_unlock(&m_mutex);
}

Task TaskQueue::gettask()
{
	Task t;
	pthread_mutex_lock(&m_mutex);
	if (m_TaskQ.size() > 0)
	{
		t= m_TaskQ.front();
		m_TaskQ.pop();
	}
	pthread_mutex_unlock(&m_mutex);
	return t;
}

