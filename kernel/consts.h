#ifndef _CONSTS_H
#define _CONSTS_H

// 链接脚本的相关符号
extern void kernel_start();
extern void text_start();
extern void rodata_start();
extern void data_start();
extern void bss_start();
extern void kernel_end();

// 动态内存分配相关常量
#define KERNEL_HEAP_SIZE    0x800000        // 堆空间大小
#define MIN_BLOCK_SIZE      0x40            // 最小分配的内存块大小
#define HEAP_BLOCK_NUM      0x20000         // 管理的总块数
#define BUDDY_NODE_NUM      0x3ffff         // 二叉树节点个数

// 页/帧大小
#define PAGE_SIZE           4096
// 可以访问的内存区域起始地址
#define MEMORY_START_PADDR  0x80000000
// 可以访问的内存区域结束地址
#define MEMORY_END_PADDR    0x88000000
// 内核起始的物理地址
#define KERNEL_BEGIN_PADDR  0x80200000
// 内核起始的虚拟地址
#define KERNEL_BEGIN_VADDR  0xffffffff80200000

// 内核地址线性映射偏移
#define KERNEL_MAP_OFFSET   0xffffffff00000000
// 内核页面线性映射偏移
#define KERNEL_PAGE_OFFSET  0xffffffff00000
// 从 PageTableEntry 中获取物理页号
#define PDE_MASK            0x003ffffffffffC00

#endif