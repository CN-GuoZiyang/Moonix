#include "types.h"
#include "def.h"
#include "thread.h"
#include "riscv.h"
#include "condition.h"
#include "fs.h"

// 全局唯一的 Processor 实例
static Processor CPU;

void
initCPU(Thread idle, ThreadPool pool)
{
    CPU.idle = idle;
    CPU.pool = pool;
    CPU.occupied = 0;
}

void
addToCPU(Thread thread)
{
    addToPool(&CPU.pool, thread);
}

void
idleMain()
{
    // 进入 idle 时禁用异步中断
    disable_and_store();
    while(1) {
        RunningThread rt = acquireFromPool(&CPU.pool);
        if(rt.tid != -1) {
            // 有线程可以运行
            CPU.current = rt;
            CPU.occupied = 1;
            switchThread(&CPU.idle, &CPU.current.thread);

            // 切换回 idle 线程处
            CPU.occupied = 0;
            retrieveToPool(&CPU.pool, CPU.current);
        } else {
            // 无可运行线程，短暂开启异步中断并处理
            enable_and_wfi();
            disable_and_store();
        }
    }
}

void
tickCPU()
{
    if(CPU.occupied) {
        // 当前有正在运行线程（不是 idle）
        if(tickPool(&CPU.pool)) {
            // 当前线程运行时间耗尽，切换到 idle
            usize flags = disable_and_store();
            switchThread(&CPU.current.thread, &CPU.idle);

            // 某个时刻再切回此线程时从这里开始
            restore_sstatus(flags);
        }
    }
}

// 由当前线程执行，退出线程并切换到 idle
void
exitFromCPU(usize code)
{
    disable_and_store();
    int tid = CPU.current.tid;
    exitFromPool(&CPU.pool, tid);
    
    if(CPU.current.thread.wait != -1) {
        wakeupCPU(CPU.current.thread.wait);
    }

    switchThread(&CPU.current.thread, &CPU.idle);
}

void
runCPU()
{   
    // 从启动线程切换进 idle，boot 线程信息丢失，不会再回来
    Thread boot = {0L, 0L};
    switchThread(&boot, &CPU.idle);
}

// 当前线程主动放弃 CPU，并进入休眠状态
void
yieldCPU()
{
    if(CPU.occupied) {
        usize flags = disable_and_store();
        int tid = CPU.current.tid;
        ThreadInfo *ti = &CPU.pool.threads[tid];
        ti->status = Sleeping;
        switchThread(&CPU.current.thread, &CPU.idle);

        restore_sstatus(flags);
    }
}

// 将某个线程唤醒
void
wakeupCPU(int tid)
{
    ThreadInfo *ti = &CPU.pool.threads[tid];
    ti->status = Ready;
    schedulerPush(tid);
}

// 执行一个用户进程
// hostTid 为需要暂停的线程的 tid
int
executeCPU(char *path, int hostTid)
{
    Inode *res = lookup(0, path);
    if(res == 0) {
        printf("Command not found!\n");
        return 0;
    }
    char *buf = kalloc(res->size);
    readall(res, buf);
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