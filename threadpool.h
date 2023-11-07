#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<vector>
#include<queue>
#include<memory>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<functional>

//任务抽象基类
class Task
{
public:
	virtual void run() = 0;//用户自定义任务类型，重写run。实现自定义任务
};
enum PoolMode
{
	MODE_FIXED,//固定数量的线程
	MODE_CACHED,//线程可以动态增长//class enum 防止里面冲突
};
//线程类型
class Thread
{
public:
	using ThreadFunc = std::function<void()>;
	void start();
	Thread(ThreadFunc func);
	~Thread();

private:
	ThreadFunc func_;
};
class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();
	void start(int initThreadSize = 4);
	void setMode(PoolMode mode);
	//设置task任务的阈值
	void setTaskQueMaxThreshHold(int threshhold);

	void submitTask(std::shared_ptr<Task> sp);
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
private:
	//定义线程函数：
	void threadHandler();
private:
	std::vector<std::unique_ptr<Thread>> threads_;//线程列表
	size_t initThreadSize_;//初始线程数量，size_t -- unsigned int
	std::queue<std::shared_ptr<Task>> taskQue_; //指针或引用才可以使用多态。用户可能传入临时对象。
	std::atomic_int taskSize_;//任务的数量
	int taskQueMaxThreshHold_;//任务队列数量上线阈值

	std::mutex taskQueMtx_;
	std::condition_variable notFull_;
	std::condition_variable notEmpty_;
	PoolMode poolMode_;
};
#endif // !THREADPOOL_H
