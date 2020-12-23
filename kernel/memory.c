#include "types.h"
#include "def.h"
#include "memory.h"
#include "consts.h"
#include "riscv.h"

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
    usize start = alloc() << 12;
    int i;
    char *vStart = (char *)(start + KERNEL_MAP_OFFSET);
    for(i = 0; i < PAGE_SIZE; i ++) {
        vStart[i] = 0;
    }
    return (usize)start;
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
    // 打开 sstatus 的 SUM 位，允许内核访问用户内存
    w_sstatus(r_sstatus() | SSTATUS_SUM);
    initFrameAllocator(
        (((usize)(kernel_end) - KERNEL_MAP_OFFSET) >> 12) + 1,
        MEMORY_END_PADDR >> 12
    );
    extern void initHeap(); initHeap();
    extern void mapKernel(); mapKernel();
    printf("***** Init Memory *****\n");
}


/*    以下为分配算法的具体实现    */

// 最大可用的内存长度，从 0x80000000 ~ 0x88000000
#define MAX_PHYSICAL_PAGES 0x8000

struct
{
    // 线段树的节点，每个都表示该范围内是否有空闲页
    uint8 node[MAX_PHYSICAL_PAGES << 1];
    // 第一个单块节点的下标
    usize firstSingle;
    // 分配区间长度
    usize length;
    // 分配的起始 ppn
    usize startPpn;
} sta;

Allocator
newAllocator(usize startPpn, usize endPpn)
{
    sta.startPpn = startPpn - 1;
    sta.length = endPpn - startPpn;
    sta.firstSingle = 1;
    while(sta.firstSingle < sta.length + 2) {
        sta.firstSingle <<= 1;
    }
    usize i = 1;
    for(i = 1; i < (sta.firstSingle << 1); i ++) {
        sta.node[i] = 1;
    }
    for(i = 1; i < sta.length; i ++) {
        sta.node[sta.firstSingle + i] = 0;
    }
    for(i = sta.firstSingle - 1; i >= 1; i --) {
        sta.node[i] = sta.node[i << 1] & sta.node[(i << 1) | 1];
    }
    Allocator ac = {alloc, dealloc};
    return ac;
}

// 分配一个物理页，返回物理页号
usize
alloc()
{
    if(sta.node[1] == 1) {
        panic("Physical memory depleted!\n");
    }
    usize p = 1;
    while(p < sta.firstSingle) {
        if(sta.node[p << 1] == 0) {
            p = p << 1;
        } else {
            p = (p << 1) | 1;
        }
    }
    usize result = p - sta.firstSingle + sta.startPpn;
    sta.node[p] = 1;
    p >>= 1;
    while(p >> 0) {
        sta.node[p] = sta.node[p << 1] & sta.node[(p << 1) | 1];
        p >>= 1;
    }
    return result;
}

// 回收物理页，参数是物理页号
void
dealloc(usize ppn)
{
    usize p = ppn - sta.startPpn + sta.firstSingle;
    if(sta.node[p] != 1) {
        printf("The page is free, no need to dealloc!\n");
        return;
    }
    sta.node[p] = 0;
    p >>= 1;
    while(p > 0) {
        sta.node[p] = sta.node[p << 1] & sta.node[(p << 1) | 1];
        p >>= 1;
    }
}