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

int
allocTid(ThreadPool *pool)
{
    int i;
    for(i = 0; i < MAX_THREAD; i ++) {
        // 分配一个未被使用的空间，返回 tid
        if(!pool->threads[i].occupied) {
            return i;
        }
    }
    panic("Alloc tid failed!\n");
    return -1;
}

void
addToPool(ThreadPool *pool, Thread thread)
{
    int tid = allocTid(pool);
    pool->threads[tid].status = Ready;
    pool->threads[tid].occupied = 1;
    pool->threads[tid].thread = thread;
    pool->scheduler.push(tid);
}

// 从线程池中获取一个可以运行的线程
// 如果没有则返回的 RunningThread 的 tid 为 -1
RunningThread
acquireFromPool(ThreadPool *pool)
{
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

// 线程从 Running 进入 Ready
// 修改线程池对应信息并保存上下文
void
retrieveToPool(ThreadPool *pool, RunningThread rt)
{
    int tid = rt.tid;
    if(!pool->threads[tid].occupied) {
        // 表明刚刚这个线程退出了，回收栈空间
        free((void *)pool->threads[tid].thread.kstack);
        return;
    }
    ThreadInfo *ti = &pool->threads[tid];
    ti->thread = rt.thread;
    if(ti->status == Running) {
        ti->status = Ready;
        pool->scheduler.push(tid);
    }
}

// 查看当前线程是否需要切换
// 返回 bool
int
tickPool(ThreadPool *pool)
{
    return pool->scheduler.tick();
}

// 线程退出，释放占据的线程池位置，并通知调度器
void
exitFromPool(ThreadPool *pool, int tid)
{
    pool->threads[tid].occupied = 0;
    pool->scheduler.exit(tid);
}