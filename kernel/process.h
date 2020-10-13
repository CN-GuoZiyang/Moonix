#ifndef _PROCESS_H
#define _PROCESS_H

typedef uint64 KernelStack;

typedef struct thread
{
    Context context;
    KernelStack kstack;
} Thread;

typedef uint64 Tid;
typedef uint64 ExitCode;

typedef enum status {
    Ready,
    Running,
    Sleeping,
    Exited
} Status;

typedef struct threadinfo
{
    Status status;
    Tid tid;
    int present;
    Thread *thread;
} ThreadInfo;

typedef struct threadpool
{
    ThreadInfo threads[MAX_THREAD_NUM];
    int occupied[MAX_THREAD_NUM];
} ThreadPool;

typedef struct runningthread
{
    Tid tid;
    Thread *thread;
} RunningThread;

typedef struct processor
{
    ThreadPool pool;
    Thread idle;
    RunningThread current;
    int hasCurrent;
} Processor;

#endif