#include "types.h"
#include "def.h"

asm(".include \"kernel/entry.asm\"");

void
main()
{
    printf("Welcome to Moonix!\n");
    extern void initInterrupt();    initInterrupt();
    extern void initMemory();       initMemory();
    extern void mapKernel();        mapKernel();
    extern void initThread();       initThread();
    while(1) {}
}