// author : zhoukang
// date   : 2022-04-08 14:36:30
#include "CThreadPool.h"
#include <signal.h>

using unique_lock = std::unique_lock<std::mutex>;

CThreadPool::CThreadPool(int pool_size, int taskQ_capacity) : poolSize(pool_size), busyNum(0), start(false), taskQ(taskQ_capacity)
{
    // 创建线程
    threadIds.reserve(poolSize);
    for (int i = 0; i < poolSize; ++i) {
        pthread_t pid;
        pthread_create(&pid, nullptr, CThreadPool::Work, this);
        threadIds.push_back(pid);
    }
}

bool CThreadPool::AddTask(Function function, void *argc)
{
    return taskQ.Push(Task(function, argc));
}

void *CThreadPool::Work(void *arg)
{
    // 线程模式：分离模式
    pthread_detach(pthread_self());
    //if(pthread_detach(pthread_self())!=0) {
    //    printf("分离失败\n");
    //};
    // 取消方式：立即取消

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);

    pthread_cleanup_push(CleanupHandler, arg);

    CThreadPool *pool = (CThreadPool *) arg;

    while (true) {
        Task task;
        {
            unique_lock lockTask(pool->mutexTask);// 加锁
            while (pool->taskQ.Empty() || !pool->start) {
                //printf("pool->taskQ.Size()=%d, pool->start = %d\n", pool->taskQ.Size(), pool->start);
                pool->condStart.wait(lockTask); // 等待阶段，解锁，避免锁被占用
            }
            //printf("pool->taskQ.Size()=%d, pool->start = %d\n", pool->taskQ.Size(), pool->start);
            task = pool->taskQ.Pop();
            ++pool->busyNum;
        }

        //printf("执行当前任务----\n");
        task.function(task.argrument);
        //printf("结束当前任务----\n");

        {
            unique_lock lockTask(pool->mutexTask);// 加锁
            //printf("pool->taskQ.Size()=%d, pool->start = %d, pool->busyNum = %d\n", pool->taskQ.Size(), pool->start, pool->busyNum-1);
            if (--pool->busyNum == 0 && pool->taskQ.Empty()) {
                pool->start = false;
                pool->condEnd.notify_all();
            }
        }
    }


    // 工作线程处理函数
    // 线程退出
    // 退出清理
    pthread_cleanup_pop(1);
    pthread_exit(0);
}

void CThreadPool::Start()
{
    unique_lock lockTask(mutexTask);
    start = true;
    condStart.notify_all();

    //printf("CThreadPool::Start() called\n");
    //printf("pool->taskQ.Size() = %d, start = %d\n", taskQ.Size(), start);
}

void CThreadPool::Wait()
{
    std::unique_lock<std::mutex> lock(mutexEnd);
    condEnd.wait(lock);
}

void CThreadPool::Release()
{
    for (int i = 0; i < threadIds.size(); ++i) {
        //printf("取消线程pthread_t = %ld\n", threadIds[i]);
        if (pthread_cancel(threadIds[i]) != 0) {
            //printf("取消线程失败\n");
        }
    }
}

void CThreadPool::CleanupHandler(void *arg)
{
    //printf("进入线程清理任务\n");
    CThreadPool* pool = (CThreadPool*)arg;
    for (int i = 0; i < pool->threadIds.size(); ++i) {
        if (pool->threadIds[i] == pthread_self()) {
            pool->threadIds.erase(pool->threadIds.begin() + i);
            //printf("线程pthred_t = %ld退出.\n", pthread_self());
        }
    }
}

void CThreadPool::PrintThreads()
{
    for (int i = 0; i < threadIds.size(); ++i) {
        printf("threadIds[%d] = %ld\n", i, threadIds[i]);
    }
}

CThreadPool &CThreadPool::GetInstance()
{
    static CThreadPool pool;
    return pool;
}

CThreadPool::~CThreadPool()
{
    Release();
}

template<typename T>
Queue<T>::Queue(int c):capacity(c), size(0), head(0), tail(0)
{
    elements = new T[capacity];
}

template<typename T>
void Queue<T>::Clear()
{
    Reset();
}

template<typename T>
void Queue<T>::Reset()
{
    size = 0;
    head = 0;
    tail = 0;
}

template<typename T>
bool Queue<T>::Push(T task)
{
    if (size < capacity) {
        elements[tail] = task;
        tail = (++tail) % capacity;
        ++size;
        return true;
    }
    return false;
}

template<typename T>
T Queue<T>::Pop()
{
    --size;
    T elem = elements[head];
    head = (++head) % capacity;
    return elem;
}
