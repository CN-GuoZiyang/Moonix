#include "types.h"
#include "defs.h"
#include "consts.h"

extern void end();

void
init_memory()
{
    init_sta(((uint64)(end - KERNEL_BEGIN_VADDR + KERNEL_BEGIN_PADDR)>>12)+1, PHYSICAL_MEMORY_END>>12);
    printf("---- setup memory ----\n");
}