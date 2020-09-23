#include "types.h"
#include "defs.h"
#include "consts.h"
#include "buddysystem.h"

static uint8 HEAP_SPACE[KERNEL_HEAP_SIZE];

// 最小分配大小为16字节
Buddy buddy_system;

void
init_buddy_system()
{
    int i;
    buddy_system.size = BUDDY_MANAGE_BLOCK;
    uint node_size = BUDDY_MANAGE_BLOCK << 1;
    for(i = 0; i < BUDDY_NODE_NUM; i ++) {
        if(((i+1) & i) == 0) {
            node_size /= 2;
        }
        buddy_system.longest[i] = node_size;
    }

    printf("node 0: %x\n", buddy_system.longest[0]);
    printf("node last: %x\n", buddy_system.longest[BUDDY_NODE_NUM-2]);
}

void
buddy_alloc(uint size)
{
    uint index = 0;
    uint node_size;
    uint offset = 0;
    
    if(size <= 0) {
        size = 1;
    }
    if(size & (size-1)) {

    }
    
}