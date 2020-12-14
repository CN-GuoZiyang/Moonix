#include "types.h"
#include "def.h"

asm(".include \"kernel/entry.asm\"");

void
testAlloc()
{
    printf("alloc %p\n", allocFrame());
    usize f = allocFrame();
    printf("alloc %p\n", f);
    printf("alloc %p\n", allocFrame());
    printf("dealloc %p\n", f);
    deallocFrame(f);
    printf("alloc %p\n", allocFrame());
    printf("alloc %p\n", allocFrame());
}

void
main()
{
    initInterrupt();
    extern void initMemory(); initMemory();
    extern void mapKernel(); mapKernel();
    printf("Safe and sound!\n");
    while(1) {}
}