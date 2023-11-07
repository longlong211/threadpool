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
	//�����̶߳���
	for (int i = 0; i < initThreadSize_; i++)
	{
		auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadHandler, this));
		//����thread�̶߳��󣬰Ѻ�����thread����
		//threads_.emplace_back(new Thread(std::bind(&ThreadPool::threadHandler,this)));
	
		threads_.emplace_back(std::move(ptr));//�뿽�������ǿ�����ֵ����
	}
	for (int i = 0; i < initThreadSize_; i++)
	{
		threads_[i]->start();//��Ҫִ���̺߳��������ǻ���������Դ��threadpool����ȡ����,ָ���̺߳�����
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
			//�Ȼ�ȡ��
			std::unique_lock<std::mutex> lock(taskQueMtx_);

			//��notempty����
			notEmpty_.wait(lock, [&]()->bool {return taskQue_.size() > 0; });
			//�����������ȡһ���������
			task = taskQue_.front();
			taskQue_.pop();
			taskSize_--;
			//�����ʣ������֪ͨ�����߳�ִ������
			if (taskQue_.size() > 0)
			{
				notEmpty_.notify_all();
			}
			//ȡ��һ�����񣬽���֪ͨ
			notFull_.notify_all();

			//�����ͷ�!!!!!��������{}
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


//����task�������ֵ
void ThreadPool::setTaskQueMaxThreshHold(int threshhold)
{
	taskQueMaxThreshHold_ = threshhold;
}

void ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
	//��ȡ��������task
	std::unique_lock<std::mutex> lock(taskQueMtx_);

	//�߳�ͨ�� �ȴ������п��ࡣ
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
	//���������ύʧ�� 1s.wait�Լ�������ѭ���� wait_for����xxx�룬wait_until

}

void Thread::start()
{
	//�����߳�ִ���̺߳���
	std::thread t(func_);//
	t.detach();//���÷����߳�

}

Thread::Thread(ThreadFunc func)
	:func_(func)
{

}
Thread::~Thread()
{

}