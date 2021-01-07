#ifndef _THREAD_H
#define _THREAD_H

#include "types.h"
#include "context.h"
#include "consts.h"
#include "condition.h"
#include "file.h"

/* 进程为资源分配的单位，保存线程共享资源 */
typedef struct {
    usize satp;         /* 页表寄存器 */
    File oFile[16];     /* 文件描述符 */
    uint8 fdOccupied[16];   /* 文件描述符是否被占用 */
} Process;

typedef struct {
    usize contextAddr;  /* 线程上下文存储的地址 */
    usize kstack;       /* 线程栈底地址 */
    Process process;    /* 所属进程 */
    int wait;           /* 等待该线程退出的线程的 Tid */
} Thread;

/* 线程状态 */
typedef enum {
    Ready,
    Running,
    Sleeping,
    Exited
} Status;

/* 调度器算法实现 */
typedef struct {
    void    (* init)(void);
    void    (* push)(int);
    int     (* pop) (void);
    int     (* tick)(void);
    void    (* exit)(int);
} Scheduler;

/* 线程池中的线程信息槽 */
typedef struct {
    Status status;
    int tid;
    int occupied;       /* 该槽位是否被占用 */
    Thread thread;
} ThreadInfo;

typedef struct {
    ThreadInfo threads[MAX_THREAD];
    Scheduler scheduler;
} ThreadPool;

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

/* 线程相关函数 */
void switchThread(Thread *self, Thread *target);
Thread newUserThread(char *data);
int allocFd(Thread *thread);
void deallocFd(Thread *thread, int fd);

/* 线程池相关函数 */
ThreadPool newThreadPool(Scheduler scheduler);
void addToPool(ThreadPool *pool, Thread thread);
RunningThread acquireFromPool(ThreadPool *pool);
void retrieveToPool(ThreadPool *pool, RunningThread rt);
int tickPool(ThreadPool *pool);
void exitFromPool(ThreadPool *pool, int tid);

/* Processor 相关函数 */
void initCPU(Thread idle, ThreadPool pool);
void addToCPU(Thread thread);
void idleMain();
void tickCPU();
void exitFromCPU(usize code);
void runCPU();
void yieldCPU();
void wakeupCPU(int tid);
int executeCPU(Inode *inode, int hostTid);
int getCurrentTid();
Thread *getCurrentThread();

/* 调度器相关函数 */
void schedulerInit();
void schedulerPush(int tid);
int  schedulerPop();
int  schedulerTick();
void schedulerExit(int tid);

#endif