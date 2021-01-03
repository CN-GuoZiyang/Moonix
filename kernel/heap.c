/*
 *  kernel/heap.c
 *  
 *  (C) 2021  Ziyang Guo
 */

/*
 * heap.c 主要使用 Buddy System Allocation 算法实现堆的动态内存分配
 * 
 * 由于 Buddy System Allocation 算法的辅助结构也会占用较大的空间
 * 应当适量设置最小分配的内存块大小，以防止得不偿失
 */

#include "types.h"
#include "def.h"
#include "consts.h"

#define LEFT_LEAF(index) ((index) * 2 + 1)
#define RIGHT_LEAF(index) ((index) * 2 + 2)
#define PARENT(index) ( ((index) + 1) / 2 - 1)

#define IS_POWER_OF_2(x) (!((x)&((x)-1)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* 用于分配的堆空间，存放在 .bss 段，8 MBytes */
static uint8 HEAP[KERNEL_HEAP_SIZE];

void buddyInit(int size);
int buddyAlloc(int size);
void buddyFree(int offset);

void
initHeap()
{
    buddyInit(HEAP_BLOCK_NUM);
}

/*
 * 获得大于等于某个数的最小的 2 的幂级数
 * 算法来源于 Java HashMap 实现
 */
int
fixSize(int size)
{
    int n = size - 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

/* 在堆上分配内存 */
void *
kalloc(int size)
{
    if(size <= 0) return 0;
    int n = (size - 1) / MIN_BLOCK_SIZE + 1;
    int block = buddyAlloc(n);
    if(block == -1) panic("Malloc failed!\n");

    /* 清零被分配的内存空间 */
    int totalBytes = fixSize(n) * MIN_BLOCK_SIZE;
    uint8 *beginAddr = (uint8 *)((usize)HEAP + (usize)(block * MIN_BLOCK_SIZE));
    int i;
    for(i = 0; i < totalBytes; i ++) {
        beginAddr[i] = 0;
    }
    
    return (void *)beginAddr;
}

/* 回收被分配出去的内存 */
void
kfree(void *ptr)
{
    if((usize)ptr < (usize)HEAP) return;
    if((usize)ptr > (usize)HEAP + KERNEL_HEAP_SIZE - MIN_BLOCK_SIZE) return;
    /* 相对于堆空间起始地址的偏移 */
    usize offset = (usize)((usize)ptr - (usize)HEAP);
    buddyFree(offset / MIN_BLOCK_SIZE);
}

/* 
 * Buddy System Allocation 的具体实现
 * 使用一棵数组形式的完全二叉数来监控内存
 */

struct
{
    int size;                    /* 管理的总块数 */
    int longest[BUDDY_NODE_NUM]; /* 每个节点表示范围内最大连续空闲块个数 */
} buddyTree;

void
buddyInit(int size)
{
    buddyTree.size = size;
    int nodeSize = size << 1;
    int i;
    /* 初始化每个节点，此时每一块都是空闲的 */
    for(i = 0; i < (size << 1) - 1; i ++) {
        if(IS_POWER_OF_2(i+1)) {
            nodeSize /= 2;
        }
        buddyTree.longest[i] = nodeSize;
    }
}

/* 
 * 试图分配 size 块，返回初始块号
 * 该版本的分配过程是从上往下搜索，寻找大小最合适的节点
 */
int
buddyAlloc(int size)
{
    int index = 0;
    int nodeSize;
    int offset;

    /* 调整内存块数量到 2 的幂 */
    if(size <= 0) size = 1;
    else if(!IS_POWER_OF_2(size)) size = fixSize(size);

    /* 一共也没有那么多空闲块 */
    if(buddyTree.longest[0] < size) {
        return -1;
    }
    
    /* 从二叉树根开始，寻找大小最符合的节点 */
    for(nodeSize = buddyTree.size; nodeSize != size; nodeSize /= 2) {
        int left = buddyTree.longest[LEFT_LEAF(index)];
        int right = buddyTree.longest[RIGHT_LEAF(index)];
        /* 优先选择最小的且满足条件的分叉，小块优先，尽量保留大块 */
        if(left <= right) {
            if(left >= size) index = LEFT_LEAF(index);
            else index = RIGHT_LEAF(index);
        } else {
            if(right >= size) index = RIGHT_LEAF(index);
            else index = LEFT_LEAF(index);
        }
    }

    /* 
     * 将该节点标记为占用
     * 
     * 注意这里标记将该节点的下级节点，便于回收时确定内存块数量
     */
    buddyTree.longest[index] = 0;

    /* 获得这一段空闲块的第一块在堆上的偏移 */
    offset = (index + 1) * nodeSize - buddyTree.size;

    /* 向上调整父节点的值 */
    while(index) {
        index = PARENT(index);
        buddyTree.longest[index] = 
            MAX(buddyTree.longest[LEFT_LEAF(index)], buddyTree.longest[RIGHT_LEAF(index)]);
    }

    return offset;
}

/* 根据 offset 回收区间 */
void
buddyFree(int offset)
{
    int nodeSize, index = 0;
    
    nodeSize = 1;
    index = offset + buddyTree.size - 1;

    /* 
     * 向上回溯到之前分配块的节点位置
     * 由于分配时没有标记下级节点，这里只需要向上寻找到第一个被标记的节点就是当时分配的节点
     */
    for( ; buddyTree.longest[index]; index = PARENT(index)) {
        nodeSize *= 2;
        if(index == 0) {
            return;
        }
    }
    buddyTree.longest[index] = nodeSize;

    /* 继续向上回溯，合并连续的空闲区间 */
    while(index) {
        index = PARENT(index);
        nodeSize *= 2;

        int leftLongest, rightLongest;
        leftLongest = buddyTree.longest[LEFT_LEAF(index)];
        rightLongest = buddyTree.longest[RIGHT_LEAF(index)];

        if(leftLongest + rightLongest == nodeSize) {
            buddyTree.longest[index] = nodeSize;
        } else {
            buddyTree.longest[index] = MAX(leftLongest, rightLongest);
        }
    }
}