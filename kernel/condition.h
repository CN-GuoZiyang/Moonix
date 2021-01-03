#ifndef _CONDITION_H
#define _CONDITION_H

#include "types.h"
#include "queue.h"

/* 
 * 条件变量
 * 内部为等待该条件满足的等待线程队列
 */
typedef struct {
    Queue waitQueue;
} Condvar;

void waitCondition(Condvar *self);
void notifyCondition(Condvar *self);

#endif