/*
 *  kernel/threadpool.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "thread.h"

ThreadPool
newThreadPool(Scheduler scheduler)
{
    ThreadPool pool;
    pool.scheduler = scheduler;
    return pool;
}

/* 在线程池中分配一个未被使用的 tid */
int
allocTid(ThreadPool *pool)
{
    int i;
    for(i = 0; i < MAX_THREAD; i ++) {
        if(!pool->threads[i].occupied) {
            return i;
        }
    }
    panic("Alloc tid failed!\n");
    return -1;
}

/* 将一个线程加入线程池，并参与调度 */
void
addToPool(ThreadPool *pool, Thread thread)
{
    int tid = allocTid(pool);
    pool->threads[tid].status = Ready;
    pool->threads[tid].occupied = 1;
    pool->threads[tid].thread = thread;
    pool->scheduler.push(tid);
}

/*
 * 从线程池中获取一个可以运行的线程
 * 如果没有线程可运行则返回的 RunningThread 的 tid 为 -1
 */
RunningThread
acquireFromPool(ThreadPool *pool)
{
    /*
     * 此处从 scheduler 中 pop 出一个可运行线程的 pid
     * 如果不再主动加入 scheduler，该线程本次运行后就不会再参与调度
     */
    int tid = pool->scheduler.pop();
    RunningThread rt;
    rt.tid = tid;
    if(tid != -1) {
        ThreadInfo *ti = &pool->threads[tid];
        ti->status = Running;
        ti->tid = tid;
        rt.thread = ti->thread;
    }
    return rt;
}
 
void
retrieveToPool(ThreadPool *pool, RunningThread rt)
{
    int tid = rt.tid;
    if(!pool->threads[tid].occupied) {
        /* 表明刚刚这个线程退出了，回收栈空间 */
        kfree((void *)pool->threads[tid].thread.kstack);
        return;
    }
    ThreadInfo *ti = &pool->threads[tid];
    ti->thread = rt.thread;
    /*
     * 线程状态为 Running 表示上一个线程是因为时间片用尽而被打断，需要继续参与调度
     * 否则状态为 Sleeping，线程主动等待条件满足，无需参与调度
     */
    if(ti->status == Running) {
        ti->status = Ready;
        pool->scheduler.push(tid);
    }
}

/* 查看当前线程是否需要切换 */
int
tickPool(ThreadPool *pool)
{
    return pool->scheduler.tick();
}

/* 线程退出，释放占据的线程池位置，并通知调度器 */
void
exitFromPool(ThreadPool *pool, int tid)
{
    pool->threads[tid].occupied = 0;
    pool->scheduler.exit(tid);
}