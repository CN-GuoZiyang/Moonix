/*
 *  kernel/rrscheduler.c
 *  
 *  (C) 2021  Ziyang Guo
 */

/*
 * rrscheduler.c 实现了 Round-robin 算法
 * 该实现规定了最大线程数量，以链表的形式将各个线程的信息连接起来
 * tid 号线程的信息会被存放在数组的 tid + 1 处
 */

#include "types.h"
#include "def.h"
#include "thread.h"

typedef struct
{
    int valid;
    usize time;
    int prev;
    int next;
} RRInfo;

struct
{
    RRInfo threads[MAX_THREAD + 1];
    usize maxTime;
    int current;
} rrScheduler;

void
schedulerInit()
{
    rrScheduler.maxTime = 1;
    rrScheduler.current = 0;
    /* 第 0 个位置为 Dummy head，用于快速找到链表头和尾 */
    RRInfo ri = {0, 0L, 0, 0};
    rrScheduler.threads[0] = ri;
}

void
schedulerPush(int tid)
{
    tid += 1;
    if(tid + 1 > MAX_THREAD + 1) {
        panic("Cannot push to scheduler!\n");
    }
    if(rrScheduler.threads[tid].time == 0) {
        rrScheduler.threads[tid].time = rrScheduler.maxTime;
    }
    int prev = rrScheduler.threads[0].prev;
    rrScheduler.threads[tid].valid = 1;
    rrScheduler.threads[prev].next = tid;
    rrScheduler.threads[tid].prev = prev;
    rrScheduler.threads[0].prev = tid;
    rrScheduler.threads[tid].next = 0;
}

int
schedulerPop()
{
    int ret = rrScheduler.threads[0].next;
    if(ret != 0) {
        int next = rrScheduler.threads[ret].next;
        int prev = rrScheduler.threads[ret].prev;
        rrScheduler.threads[next].prev = prev;
        rrScheduler.threads[prev].next = next;
        rrScheduler.threads[ret].prev = 0;
        rrScheduler.threads[ret].next = 0;
        rrScheduler.threads[ret].valid = 0;
        rrScheduler.current = ret;
    }
    return ret-1;
}

int
schedulerTick()
{
    int tid = rrScheduler.current;
    if(tid != 0) {
        rrScheduler.threads[tid].time -= 1;
        if(rrScheduler.threads[tid].time == 0) {
            return 1;
        } else {
            return 0;
        }
    }
    return 1;
}

void
schedulerExit(int tid)
{
    tid += 1;
    if(rrScheduler.current == tid) {
        rrScheduler.current = 0;
    }
}