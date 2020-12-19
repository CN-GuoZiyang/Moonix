#ifndef _THREAD_H
#define _THREAD_H

#include "types.h"
#include "context.h"
#include "consts.h"

typedef struct {
    // 线程上下文存储的地址
    usize contextAddr;
    // 线程栈底地址
    usize kstack;
} Thread;

typedef enum {
    Ready,
    Running,
    Sleeping,
    Exited
} Status;

// 调度器（函数指针）
typedef struct {
    void    (* init)(void);
    void    (* push)(int);
    int     (* pop) (void);
    int     (* tick)(void);
    void    (* exit)(int);
} Scheduler;

// 线程池中存储的线程信息
typedef struct {
    Status status;
    int tid;
    int occupied;
    Thread thread;
} ThreadInfo;

typedef struct {
    ThreadInfo threads[MAX_THREAD];
    Scheduler scheduler;
} ThreadPool;

// 表示一个正在运行的线程
typedef struct {
    int tid;
    Thread thread;
} RunningThread;

typedef struct {
    ThreadPool pool;
    Thread idle;
    RunningThread current;
    int occupied;
} Processor;

// 线程相关函数
void switchThread(Thread *self, Thread *target);

// 线程池相关函数
ThreadPool newThreadPool(Scheduler scheduler);
void addToPool(ThreadPool *pool, Thread thread);
RunningThread acquireFromPool(ThreadPool *pool);
void retrieveToPool(ThreadPool *pool, RunningThread rt);
int tickPool(ThreadPool *pool);
void exitFromPool(ThreadPool *pool, int tid);

// Processor 相关函数
void initCPU(Thread idle, ThreadPool pool);
void addToCPU(Thread thread);
void idleMain();
void tickCPU();
void exitFromCPU(usize code);
void runCPU();

// 调度器相关函数
void schedulerInit();
void schedulerPush(int tid);
int  schedulerPop();
int  schedulerTick();
void schedulerExit(int tid);

#endif