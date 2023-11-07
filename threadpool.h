#ifndef THREADPOOL_H
#define THREADPOOL_H
#include<vector>
#include<queue>
#include<memory>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<functional>

//����������
class Task
{
public:
	virtual void run() = 0;//�û��Զ����������ͣ���дrun��ʵ���Զ�������
};
enum PoolMode
{
	MODE_FIXED,//�̶��������߳�
	MODE_CACHED,//�߳̿��Զ�̬����//class enum ��ֹ�����ͻ
};
//�߳�����
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
	//����task�������ֵ
	void setTaskQueMaxThreshHold(int threshhold);

	void submitTask(std::shared_ptr<Task> sp);
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
private:
	//�����̺߳�����
	void threadHandler();
private:
	std::vector<std::unique_ptr<Thread>> threads_;//�߳��б�
	size_t initThreadSize_;//��ʼ�߳�������size_t -- unsigned int
	std::queue<std::shared_ptr<Task>> taskQue_; //ָ������òſ���ʹ�ö�̬���û����ܴ�����ʱ����
	std::atomic_int taskSize_;//���������
	int taskQueMaxThreshHold_;//�����������������ֵ

	std::mutex taskQueMtx_;
	std::condition_variable notFull_;
	std::condition_variable notEmpty_;
	PoolMode poolMode_;
};
#endif // !THREADPOOL_H
