#ifndef _THREAD_H
#define _THREAD_H

#include "context.h"

typedef struct {
    // 线程上下文存储的地址
    usize contextAddr;
    // 线程栈底地址
    usize kstack;
} Thread;

#endif