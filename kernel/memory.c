#include "types.h"
#include "defs.h"
#include "riscv.h"
#include "consts.h"

struct segmenttreeallocator
{
    uint8 a[0x10000]; // 0和1表示是否被占用
    uint64 m;
    uint64 n;   // 区间大小
    uint64 offset;  // 起始ppn
} sta;

extern void end();

/* use ppn to init segment tree */
void
init_memory_allocator(uint64 l, uint64 r)
{
    uint64 i = 0L;
    sta.offset = l - 1;
    sta.n = r - l;
    sta.m = 1;
    while(sta.m < sta.n + 2) {
        sta.m = sta.m << 1;
    }
    for(i = 1; i < (sta.m<<1); i ++) {
        sta.a[i] = 1;
    }
    for(i = 1; i < sta.n; i ++) {
        sta.a[sta.m+i] = 0;
    }
    for(i = sta.m-1; i >= 1; i --) {
        sta.a[i] = sta.a[i<<1] & sta.a[(i<<1)|1];
    }
}

void
init_memory()
{
    printf("---- setup memory ----\n");
    init_memory_allocator(
            ((uint64)(end - KERNEL_BEGIN_VADDR + KERNEL_BEGIN_PADDR)>>12)+1,
            PHYSICAL_MEMORY_END>>12
    );
    printf("---- remap kernel ----\n");
    remap_kernel();
    init_buddy_system();
}

uint64
alloc_frame()
{
    if(sta.a[1]==1) {
        panic("Physical memory depleted!\n");
    }
    int p = 1;
    while(p < sta.m) {
        if(sta.a[p<<1]==0) {
            p = p << 1;
        } else {
            p = (p << 1) | 1;
        }
    }
    uint64 result = p + sta.offset - sta.m;
    sta.a[p] = 1;
    p = p >> 1;
    while(p > 0) {
        sta.a[p] = sta.a[p << 1] & sta.a[(p << 1) | 1];
        p = p >> 1;
    }
    return result;
}

void
dealloc_frame(uint64 n)
{
    int p = n + sta.m - sta.offset;
    if(sta.a[p] != 1) {
        printf("The page is free, no need to dealloc!\n");
        return;
    }
    sta.a[p] = 0;
    p = p >> 1;
    while(p > 0) {
        sta.a[p] = sta.a[p << 1] & sta.a[(p << 1)|1];
        p = p >> 1;
    }
}