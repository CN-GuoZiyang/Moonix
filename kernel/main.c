#include "types.h"
#include "def.h"

asm(".include \"kernel/entry.asm\"");

void
main()
{
    initInterrupt();
    extern void initMemory(); initMemory();
    extern void mapKernel(); mapKernel();
    printf("Safe and sound!\n");
    while(1) {}
}