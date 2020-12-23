#include "types.h"
#include "def.h"
#include "condition.h"
#include "queue.h"
#include "thread.h"

// 将当前线程加入到等待队列中
void
waitCondition(Condvar *self)
{
    pushBack(&self->waitQueue, getCurrentTid());
    yieldCPU();
}

// 从等待队列中唤醒一个线程
void
notifyCondition(Condvar *self)
{
    if(!isEmpty(&self->waitQueue)) {
        int tid = (int)popFront(&self->waitQueue);
        wakeupCPU(tid);
    }
}