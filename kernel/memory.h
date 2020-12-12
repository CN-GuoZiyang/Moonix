#ifndef _MEMORY_H
#define _MEMORY_H

#include "types.h"

extern void kernel_end();

// 页/帧大小
#define PAGE_SIZE 4096

// 可以访问的内存区域起始地址
#define PHYSICAL_MEMORY_START 0x80000000

// 可以访问的内存区域结束地址
#define PHYSICAL_MEMORY_END 0x88000000

// 内核起始的物理地址
#define KERNEL_BEGIN_PADDR 0x80200000

// 内核起始的虚拟地址
#define KERNEL_BEGIN_VADDR 0x80200000

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