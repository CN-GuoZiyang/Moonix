#ifndef _CONSTS_H
#define _CONSTS_H

/* 链接脚本中定义的部分符号，主要用于内核重映射 */
extern void kernel_start();                     /* 内核所在内存区域起始的虚拟地址 */
extern void text_start();                       /* 代码段起始虚拟地址 */
extern void rodata_start();                     /* 只读数据段起始虚拟地址 */
extern void data_start();                       /* 数据段起始虚拟地址 */
extern void bss_start();                        /* .bss 段起始虚拟地址 */
extern void bss_end();                          /* .bbs 段结束虚拟地址 */
extern void kernel_end();                       /* 内核所在内存空间结束的虚拟地址 */

/* 动态内存中定义堆的相关常量 */
#define KERNEL_HEAP_SIZE    0x800000            /* 堆空间大小 */
#define MIN_BLOCK_SIZE      0x40                /* 最小分配的内存块大小 */
#define HEAP_BLOCK_NUM      0x20000             /* 管理的总块数 */
#define BUDDY_NODE_NUM      0x3ffff             /* 二叉树节点个数 */

#define PAGE_SIZE           4096                /* 页/帧大小 */
#define MEMORY_START_PADDR  0x80000000          /* 可以访问的内存区域起始地址 */
#define MEMORY_END_PADDR    0x88000000          /* 可以访问的内存区域结束地址 */
#define KERNEL_BEGIN_PADDR  0x80200000          /* 内核起始的物理地址 */
#define KERNEL_BEGIN_VADDR  0xffffffff80200000  /* 内核起始的虚拟地址 */

#define KERNEL_MAP_OFFSET   0xffffffff00000000  /* 内核地址线性映射偏移 */
#define KERNEL_PAGE_OFFSET  0xffffffff00000     /* 内核页面线性映射偏移 */
#define PDE_MASK            0x003ffffffffffC00  /* 该掩码用于从页表项中获取物理页号 */

#define KERNEL_STACK_SIZE   0x80000             /* 内核栈大小 */
#define USER_STACK_SIZE     0x80000             /* 用户栈大小 */
#define USER_STACK_OFFSET   0xffffffff00000000  /* 用户栈起始虚拟地址 */

#define MAX_THREAD          0x40                /* 线程池最大线程数 */

#endif