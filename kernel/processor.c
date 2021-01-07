/*
 *  kernel/processor.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "thread.h"
#include "riscv.h"
#include "condition.h"
#include "fs.h"

/* 全局唯一的 Processor 实例 */
static Processor CPU;

void
initCPU(Thread idle, ThreadPool pool)
{
    CPU.idle = idle;
    CPU.pool = pool;
    CPU.occupied = 0;
}

/* 让线程参与 CPU 调度 */
void
addToCPU(Thread thread)
{
    addToPool(&CPU.pool, thread);
}

/* 
 * 调度线程的运行逻辑
 * 在线程用完时间片或线程结束后，会返回调度线程
 * 用于按照一定规则选择下一个要运行的线程
 */
void
idleMain()
{
    /* 
     * 进入 idle 时禁用异步中断
     * 防止调度过程本身被时钟中断打断
     */
    disable_and_store();
    while(1) {
        /* 从线程池获取一个可运行的线程 */
        RunningThread rt = acquireFromPool(&CPU.pool);
        if(rt.tid != -1) {
            /* 有线程可以运行就切换到该线程 */
            CPU.current = rt;
            CPU.occupied = 1;
            switchThread(&CPU.idle, &CPU.current.thread);

            /*
             * 线程用尽时间片或运行结束
             * 切换回 idle 线程处，修改线程状态，进行下一次调度
             */
            CPU.occupied = 0;
            retrieveToPool(&CPU.pool, CPU.current);
        } else {
            /* 
             * 当前无可运行线程
             * 开启异步中断响应并处理
             */
            enable_and_wfi();
            disable_and_store();
        }
    }
}

/* 时钟中断发生时，CPU 检查正在运行程序的时间片 */
void
tickCPU()
{
    if(CPU.occupied) {
        /* 当前有正在运行线程（不是 idle） */
        if(tickPool(&CPU.pool)) {
            /* 
             * 当前线程运行时间耗尽，切换回 idle
             * 进入 idle 线程前需要关闭异步中断 
             */
            usize flags = disable_and_store();
            switchThread(&CPU.current.thread, &CPU.idle);

            /* 某个时刻再切回此线程时从这里开始 */
            restore_sstatus(flags);
        }
    }
}

/* 由当前线程执行，退出线程并切换到 idle 线程 */
void
exitFromCPU(usize code)
{
    disable_and_store();
    int tid = CPU.current.tid;
    exitFromPool(&CPU.pool, tid);
    
    /* 
     * 检查是否有线程在等待当前线程退出
     * 如果有就唤醒，让其参与调度
     */
    if(CPU.current.thread.wait != -1) {
        wakeupCPU(CPU.current.thread.wait);
    }

    switchThread(&CPU.current.thread, &CPU.idle);
}

void
runCPU()
{   
    /*
     * 在启动线程的最后调用
     * 从启动线程切换进 idle，boot 线程信息丢失，不会再回来
     */
    Thread boot;
    boot.contextAddr = 0;
    boot.kstack = 0;
    boot.wait = -1;
    switchThread(&boot, &CPU.idle);
}

/* 当前线程主动放弃 CPU，并进入休眠 */
void
yieldCPU()
{
    if(CPU.occupied) {
        /* 修改当前线程状态并切换到 idle 线程 */
        usize flags = disable_and_store();
        int tid = CPU.current.tid;
        ThreadInfo *ti = &CPU.pool.threads[tid];
        ti->status = Sleeping;
        switchThread(&CPU.current.thread, &CPU.idle);

        /* 从休眠中被唤醒时从该处开始执行 */
        restore_sstatus(flags);
    }
}

/* 
 * 将某个线程唤醒
 * 并使其参与调度
 */
void
wakeupCPU(int tid)
{
    ThreadInfo *ti = &CPU.pool.threads[tid];
    ti->status = Ready;
    schedulerPush(tid);
}

/*
 * 执行一个用户进程
 * path 为可执行文件在文件系统的路径
 * hostTid 为需要暂停的线程的 tid
 */
int
executeCPU(Inode *inode, int hostTid)
{
    if(inode->type == TYPE_DIR) {
        printf("%s: is a directory!\n", inode->filename);
        return 0;
    }
    /* 暂时将 ELF 文件读入 buf 数组中 */
    char *buf = kalloc(inode->size);
    readall(inode, buf);
    Thread t = newUserThread(buf);
    t.wait = hostTid;
    kfree(buf);
    addToCPU(t);
    return 1;
}

int
getCurrentTid()
{
    return CPU.current.tid;
}

Thread
*getCurrentThread()
{
    return &CPU.current.thread;
}