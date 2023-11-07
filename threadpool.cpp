#include"threadpool.h"
#include <functional>
#include<thread>
#include<iostream>
const int TASK_MAX_THRESHHOLD = 1024;
ThreadPool::ThreadPool()
	:initThreadSize_(4)
	, taskSize_(0)
	, taskQueMaxThreshHold_(TASK_MAX_THRESHHOLD)
	, poolMode_(PoolMode::MODE_FIXED)
{

}
ThreadPool::~ThreadPool()
{

}
void ThreadPool::start(int initThreadSize)
{
	initThreadSize_ = initThreadSize;
	//创建线程对象
	for (int i = 0; i < initThreadSize_; i++)
	{
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadHandler, this));
		//创建thread线程对象，把函数给thread对象
		//threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadHandler,this)));
	
		threads_.emplace_back(std::move(ptr));//想拷贝，但是可以右值拷贝
	}
	for (int i = 0; i < initThreadSize_; i++)
	{
		threads_[i]->start();//需要执行线程函数，但是互斥锁，资源在threadpool里面取任务,指定线程函数。
	}

}

void ThreadPool::threadHandler()
{
	//std::cout << "begin threadFunc tid: " << std::this_thread::get_id() << std::endl;
	//std::cout << "end threadFunc tid: " << std::this_thread::get_id() << std::endl;

	for (;;)
	{
		std::shared_ptr<Task> task;
		{
			//先获取锁
			std::unique_lock<std::mutex> lock(taskQueMtx_);

			//等notempty条件
			notEmpty_.wait(lock, [&]()->bool {return taskQue_.size() > 0; });
			//从任务队列种取一个任务出来
			task = taskQue_.front();
			taskQue_.pop();
			taskSize_--;
			//如果有剩余任务，通知其他线程执行任务
			if (taskQue_.size() > 0)
			{
				notEmpty_.notify_all();
			}
			//取出一个任务，进行通知
			notFull_.notify_all();

			//把锁释放!!!!!加作用域。{}
		}
		if (task != nullptr)
		{
			task->run();
		}
	}

}

void ThreadPool::setMode(PoolMode mode)
{
	poolMode_ = mode;
}
//void ThreadPool::setInitThreadSize(int size)
//{
//	initThreadSize_ = size;
//} 


//设置task任务的阈值
void ThreadPool::setTaskQueMaxThreshHold(int threshhold)
{
	taskQueMaxThreshHold_ = threshhold;
}

void ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
	//获取锁，生产task
	std::unique_lock<std::mutex> lock(taskQueMtx_);

	//线程通信 等待队列有空余。
	//while(taskQue_.size()== taskQueMaxThreshHold_)
	//{
	//notFull_.wait(lock);
	//}
	if (!notFull_.wait_for(lock, std::chrono::seconds(1), [&]()->bool {return taskQue_.size() < (size_t)taskQueMaxThreshHold_; }))
	{
		std::cerr<<"task queue is full,submit task fail;"<<std::endl;
			return;
	}

	//notFull_.wait(lock, [&]()->bool {return taskQue_.size() < taskQueMaxThreshHold_; });
	taskQue_.emplace(sp);
	taskSize_++;
	notEmpty_.notify_all();
	//处理任务提交失败 1s.wait自己加条件循环。 wait_for最多等xxx秒，wait_until

}

void Thread::start()
{
	//创建线程执行线程函数
	std::thread t(func_);//
	t.detach();//设置分离线程

}

Thread::Thread(ThreadFunc func)
	:func_(func)
{

}
Thread::~Thread()
{

}