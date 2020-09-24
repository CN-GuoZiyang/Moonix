#include "types.h"
#include "defs.h"
#include "consts.h"
#include "buddysystem.h"

#define LEFT_LEAF(index) ((index) * 2 + 1)
#define RIGHT_LEAF(index) ((index) * 2 + 2)
#define PARENT(index) ( ((index) + 1) / 2 - 1)

#define IS_POWER_OF_2(x) (!((x)&((x)-1)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define NULL 0

static uint8 HEAP_SPACE[KERNEL_HEAP_SIZE];

// 最小分配大小为16字节
static Buddy buddy_system;

void
init_buddy_system()
{
    int i;
    buddy_system.size = BUDDY_MANAGE_BLOCK;
    uint node_size = BUDDY_MANAGE_BLOCK << 1;
    for(i = 0; i < BUDDY_NODE_NUM; i ++) {
        if(IS_POWER_OF_2(i+1)) {
            node_size /= 2;
        }
        buddy_system.longest[i] = node_size;
    }
}

uint
fix_size(uint size)
{
    uint n = size - 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n+1;
}

uint
buddy_alloc(uint size)
{
    uint index = 0;
    uint node_size;
    uint offset = 0;
    
    if(size <= 0) {
        size = 1;
    } else if(!IS_POWER_OF_2(size)) {
        size = fix_size(size);
    }
    if(buddy_system.longest[index] < size) {
        return -1;
    }
    for(node_size = buddy_system.size; node_size != size; node_size /= 2) {
        if(buddy_system.longest[LEFT_LEAF(index)] >= size)
            index = LEFT_LEAF(index);
        else
            index = RIGHT_LEAF(index);
    }
    printf("index = %d\n", index);
    buddy_system.longest[index] = 0;
    offset = (index + 1) * node_size - buddy_system.size;

    printf("alloc block = %d\n", offset);

    while(index) {
        index = PARENT(index);
        buddy_system.longest[index] =  MAX(buddy_system.longest[LEFT_LEAF(index)], buddy_system.longest[RIGHT_LEAF(index)]);
    }

    return offset;
}

void
buddy_free(uint offset)
{
    uint node_size, index = 0;
    uint left_longest, right_longest;

    if(offset < 0 || offset >= buddy_system.size) {
        return;
    }

    node_size = 1;
    index = offset + buddy_system.size - 1;

    for(; buddy_system.longest[index]; index = PARENT(index)) {
        node_size *= 2;
        if (index == 0) return;
    }

    buddy_system.longest[index] = node_size;

    while (index) {
        index = PARENT(index);
        node_size *= 2;
        left_longest = buddy_system.longest[LEFT_LEAF(index)];
        right_longest = buddy_system.longest[RIGHT_LEAF(index)];

        if(left_longest + right_longest == node_size)
            buddy_system.longest[index] = node_size;
        else
            buddy_system.longest[index] = MAX(left_longest, right_longest);
    }
}

void*
malloc(uint size)
{
    if(size <= 0) return NULL;
    uint n;
    if((size & (MIN_NODE_SIZE-1)) != 0)
        n = size / MIN_NODE_SIZE + 1;
    else
        n = size / MIN_NODE_SIZE;
    uint block = buddy_alloc(n);
    // clear the memory
    n = fix_size(n);
    uint64 beginaddr = (uint64)HEAP_SPACE + (uint64)(block << 4);
    uint64 endaddr = (uint64)HEAP_SPACE + (uint64)((block+n) << 4);
    uint64 i;
    for(i = beginaddr; i < endaddr; i ++) {
        *((uint *)i) = 0;
    }
    return HEAP_SPACE + (uint64)(block << 4);
}

void
free(void* ptr)
{
    if((uint64)ptr < (uint64)HEAP_SPACE) return;
    if((uint64)ptr >= (uint64)HEAP_SPACE + KERNEL_HEAP_SIZE - MIN_NODE_SIZE) return;
    uint offset = (uint64)ptr - (uint64)HEAP_SPACE;
    if(!IS_POWER_OF_2(offset)) return;
    buddy_free(offset >> 4);
}