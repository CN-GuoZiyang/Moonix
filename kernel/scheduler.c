#include "types.h"
#include "defs.h"
#include "context.h"
#include "consts.h"
#include "process.h"
#include "scheduler.h"

RRScheduler scheduler;

void
init_scheduler(uint64 max_time_slice)
{
    scheduler.max_time = max_time_slice;
    scheduler.current = 0;
    RRInfo rri;
    rri.valid = 0;
    rri.time = 0;
    rri.prev = 0;
    rri.next = 0;
    scheduler.threads[0] = rri;
}

void
scheduler_push(Tid tid)
{
    tid = tid + 1;
    if(scheduler.threads[tid].time == 0) {
        scheduler.threads[tid].time = scheduler.max_time;
    }
    uint64 prev = scheduler.threads[0].prev;
    scheduler.threads[tid].valid = 1;
    scheduler.threads[prev].next = tid;
    scheduler.threads[tid].prev = prev;
    scheduler.threads[0].prev = tid;
    scheduler.threads[tid].next = 0;
}

Tid
scheduler_pop()
{
    uint64 ret = scheduler.threads[0].next;
    if(ret != 0) {
        uint64 next = scheduler.threads[ret].next;
        uint64 prev = scheduler.threads[ret].prev;
        scheduler.threads[next].prev = prev;
        scheduler.threads[prev].next = next;
        scheduler.threads[ret].prev = 0;
        scheduler.threads[ret].next = 0;
        scheduler.threads[ret].valid = 0;
        scheduler.current = ret;
        return ret - 1;
    } else {
        return -1;
    }
}

int
scheduler_tick()
{
    uint64 tid = scheduler.current;
    if(tid != 0) {
        scheduler.threads[tid].time -= 1;
        if(scheduler.threads[tid].time == 0) {
            return 1;
        } else {
            return 0;
        }
    }
    return 1;
}

void
scheduler_exit(Tid tid)
{
    tid = tid + 1;
    if(scheduler.current == tid) {
        scheduler.current = 0;
    }
}