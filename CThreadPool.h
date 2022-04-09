// author : zhoukang
// date   : 2022-04-08 14:36:30

#ifndef THREADPOOL_CTHREADPOOL_H
#define THREADPOOL_CTHREADPOOL_H

#include "thread"
#include <pthread.h>
#include <vector>
#include <mutex>
#include <condition_variable>

typedef void (*Function)(void*);
struct Task {
    Function function;
    void* argrument;

    Task(Function func = nullptr, void *arg = nullptr) : function(func), argrument(arg)
    {}
};

// 先进先出
template<typename T>
class Queue {
private:
    int capacity;   // 容量
    int size;       // 当前队列元素个数
    int head;       // 队头
    int tail;       // 队尾
    T* elements;
public:

    explicit Queue(int capacity = 10);
    bool Push(T task);
    T Pop();
    void Clear();
    inline bool Empty() {
        return size == 0;
    }
    inline int Size() {
        return size;
    }
    inline int Capacity() {
        return capacity;
    }

    ~Queue() {
        delete []elements;
    };

private:
    void Reset();
};


// 实现固定大小线程池
// 1.工作的线程池大小固定，不需要管理者线程来管理
// 2.一次性生产所有任务
// 3.开启工作
// 4.等待工作结束
// 5.一次性消费所有任务
// 6.生产与消费不交叉。

// 线程池退出
// 1.Release，内部通过pthread_cancel的方式取消子线程
class CThreadPool {
    using TaskQ = Queue<Task>;
private:
    // 池子大小
    int poolSize;
    // 工作线程组id
    std::vector<pthread_t> threadIds;

    // 任务队列
    TaskQ taskQ;
    // 正在工作的线程数
    int busyNum;
    bool start; // 任务启动标记

    // mutex
    std::mutex mutexTask; // 读取任务队列锁
    std::condition_variable condStart; // 任务启动条件变量，添加任务并调用Start()之后，开启任务

    std::mutex mutexEnd; // 任务结束
    std::condition_variable condEnd; // 任务结束条件变量，没有工作线程时，且任务队列为空，任务结束


public:
    explicit CThreadPool(int pool_size, int taskQ_capacity);
    bool AddTask(Function function, void* argc);
    void Start();       // 启动线程
    void Wait();        // 等待任务结束
    void Release();     // 结束线程池

    void PrintThreads();

private:
    static void* Work(void* arg);
    static void CleanupHandler(void* arg);
};



#endif //THREADPOOL_CTHREADPOOL_H
