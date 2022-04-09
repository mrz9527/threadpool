#include <iostream>
#include "CThreadPool.h"
#include <unistd.h>

#define __NR_gettid 186

struct TaskParam {
    int threadId;
    long value;

    TaskParam(int tid, long val) : threadId(tid), value(val)
    {}
};

void func(void* arg) {
    TaskParam* param = (TaskParam*)arg;
    int id = param->threadId;
    long val = param->value;
    delete param;


    for (long i = 0; i < val; ++i) {
        printf("任务编号(%d), getpid() = %d, pthread_self() = %ld, gettid = %ld, loop(%ld)\n", id, getpid(), pthread_self(), (long int)syscall(__NR_gettid), i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    printf("任务编号(%d), getpid() = %d, pthread_self() = %ld, gettid = %ld, 总数(%ld)结束\n\n", id, getpid(), pthread_self(), (long int)syscall(__NR_gettid), val);
}
void demo() {
    printf("主线程(100), getpid() = %d, pthread_self() = %ld, gettid = %ld\n\n", getpid(), pthread_self(), (long int)syscall(__NR_gettid));

    //printf("ps -xH/top -H查看所有线程，按任意键，创建线程池\n");
    //getchar();

    // 初始化线程池
    CThreadPool threadPool(2, 20);
    long arr[12] = {0};
    int len = sizeof(arr)/sizeof(arr[0]);
    for(long i = 0; i<len; ++i) {
        arr[i] = 5 + i;
    }

    threadPool.PrintThreads();

    // 添加任务
    for (int i = 0; i < len; ++i) {
        if(threadPool.AddTask(func, (void*)new TaskParam(i, arr[i])) == false) {
            printf("队列已满\n");
        }
    }

    //printf("ps -xH/top -H查看所有线程，按任意键，开启线程池任务\n");
    //getchar();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    threadPool.Start();
    threadPool.Wait();

    //printf("ps -xH/top -H查看所有线程，按任意键，结束线程池\n");
    //getchar();

    threadPool.Release();

    //printf("ps -xH/top -H查看所有线程，按任意键，结束程序\n");
    //getchar();

    //printf("主线程(100), 结束\n\n");

    // 开始执行任务
}

int main()
{
    std::cout << "Hello, World!" << std::endl;
    demo();
    return 0;
}
