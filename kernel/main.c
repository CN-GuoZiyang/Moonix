#include "types.h"
#include "defs.h"

void frame_alloc_test();

void
main()
{
    init_interrupt();
    init_memory();
    frame_alloc_test();
    init_timer();
    asm volatile("ebreak");
    printf("return from trap!\n");
}

void
frame_alloc_test()
{
    printf("alloc 0x%x\n", alloc());
    uint64 f = alloc();
    printf("alloc 0x%x\n", f);
    printf("alloc 0x%x\n", alloc());
    printf("dealloc 0x%x\n", f);
    dealloc(f);
    printf("alloc 0x%x\n", alloc());
    printf("alloc 0x%x\n", alloc());
}