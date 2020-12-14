#ifndef _MEMORY_H
#define _MEMORY_H

#include "types.h"

// 具体的页帧分配/回收算法
typedef struct
{
    usize (*alloc)(void);
    void (*dealloc)(usize index);
} Allocator;

// 页帧分配/回收管理
typedef struct
{
    // 可用空间的起始
    usize startPpn;
    Allocator allocator;
} FrameAllocator;

#endif