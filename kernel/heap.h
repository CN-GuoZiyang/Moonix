#ifndef _HEAP_H
#define _HEAP_H

// 动态内存分配相关常量
#define KERNEL_HEAP_SIZE    0x800000        // 堆空间大小
#define MIN_BLOCK_SIZE      0x40            // 最小分配的内存块大小
#define HEAP_BLOCK_NUM      0x20000         // 管理的总块数
#define BUDDY_NODE_NUM      0x3ffff         // 二叉树节点个数

#endif