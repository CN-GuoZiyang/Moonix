#include "types.h"
#include "def.h"
#include "memory.h"

// 全局页帧分配器
FrameAllocator frameAllocator;

// 分配算法的三个关键函数
Allocator newAllocator(usize startPpn, usize endPpn);
usize alloc();
void dealloc(usize ppn);

void
initFrameAllocator(usize startPpn, usize endPpn)
{
    frameAllocator.startPpn = startPpn;
    frameAllocator.allocator = newAllocator(startPpn, endPpn);
}

// 返回物理页的起始地址
usize
allocFrame()
{
    return alloc() << 12;
}

// 参数为物理页的起始地址
void
deallocFrame(usize startAddr)
{
    dealloc(startAddr >> 12);
}

// 初始化页分配和动态内存分配
void
initMemory()
{
    initFrameAllocator(
        (((usize)(kernel_end) - KERNEL_BEGIN_VADDR + KERNEL_BEGIN_PADDR) >> 12) + 1,
        PHYSICAL_MEMORY_END >> 12
    );
    extern void initHeap(); initHeap();
    printf("***** Init Memory *****\n");
}


/*    以下为分配算法的具体实现    */

// 最大可用的内存长度，从 0x80000000 ~ 0x88000000
#define MAX_PHYSICAL_PAGES 0x8000

struct
{
    uint8 a[MAX_PHYSICAL_PAGES << 1];
    usize m;
    usize n;
    usize offset;
} sta;

Allocator
newAllocator(usize startPpn, usize endPpn)
{
    sta.offset = startPpn - 1;
    sta.n = endPpn - startPpn;
    sta.m = 1;
    while(sta.m < sta.n + 2) {
        sta.m <<= 1;
    }
    usize i = 1;
    for(i = 1; i < (sta.m << 1); i ++) {
        sta.a[i] = 1;
    }
    for(i = 1; i < sta.n; i ++) {
        sta.a[sta.m + i] = 0;
    }
    for(i = sta.m - 1; i >= 1; i --) {
        sta.a[i] = sta.a[i << 1] & sta.a[(i << 1) | 1];
    }
    Allocator ac = {alloc, dealloc};
    return ac;
}

// 分配一个物理页，返回物理页号
usize
alloc()
{
    if(sta.a[1] == 1) {
        panic("Physical memory depleted!\n");
    }
    usize p = 1;
    while(p < sta.m) {
        if(sta.a[p << 1] == 0) {
            p = p << 1;
        } else {
            p = (p << 1) | 1;
        }
    }
    usize result = p + sta.offset - sta.m;
    sta.a[p] = 1;
    p >>= 1;
    while(p >> 0) {
        sta.a[p] = sta.a[p << 1] & sta.a[(p << 1) | 1];
        p >>= 1;
    }
    return result;
}

// 回收物理页，参数是物理页号
void
dealloc(usize ppn)
{
    usize p = ppn + sta.m - sta.offset;
    if(sta.a[p] != 1) {
        printf("The page is free, no need to dealloc!\n");
        return;
    }
    sta.a[p] = 0;
    p >>= 1;
    while(p > 0) {
        sta.a[p] = sta.a[p << 1] & sta.a[(p << 1) | 1];
        p >>= 1;
    }
}