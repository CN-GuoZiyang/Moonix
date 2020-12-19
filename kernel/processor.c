#include "types.h"
#include "def.h"
#include "thread.h"
#include "riscv.h"

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
            printf("\n>>>> will switch_to thread %d in idle_main!\n", CPU.current.tid);
            switchThread(&CPU.idle, &CPU.current.thread);

            // 切换回 idle 线程处
            printf("<<<< switch_back to idle in idle_main!\n");
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
    printf("Thread %d exited, exit code = %d\n", tid, code);
    switchThread(&CPU.current.thread, &CPU.idle);
}

void
runCPU()
{   
    // 从启动线程切换进 idle，boot 线程信息丢失，不会再回来
    Thread boot = {0L, 0L};
    switchThread(&boot, &CPU.idle);
}