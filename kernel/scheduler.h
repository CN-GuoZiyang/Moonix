#ifndef _SCHEDULER_H
#define _SCHEDULER_H

typedef struct rrinfo
{
    int valid;
    uint64 time;
    uint64 prev;
    uint64 next;
} RRInfo;

typedef struct rrscheduler
{
    RRInfo threads[MAX_THREAD_NUM+1];
    uint64 max_time;
    uint64 current;
} RRScheduler;

void init_scheduler(uint64 max_time_slice);
void scheduler_push(Tid tid);
Tid scheduler_pop();
int scheduler_tick();
void scheduler_exit(Tid tid);

#endif