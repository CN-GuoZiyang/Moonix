/*
 *  kernel/condition.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "condition.h"
#include "queue.h"
#include "thread.h"

/*
 * 将当前线程加入到等待队列中
 * 并主动让出 CPU 使用权
 */
void
waitCondition(Condvar *self)
{
    pushBack(&self->waitQueue, getCurrentTid());
    yieldCPU();
}

/*
 * 从等待队列中获取队首线程
 * 并将其唤醒，加入 CPU 调度
 */
void
notifyCondition(Condvar *self)
{
    if(!isEmpty(&self->waitQueue)) {
        int tid = (int)popFront(&self->waitQueue);
        wakeupCPU(tid);
    }
}