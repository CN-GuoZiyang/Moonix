#include "types.h"
#include "def.h"

asm(".include \"kernel/entry.asm\"");
asm(".include \"kernel/linkFS.asm\"");

void
main()
{
    printf("Welcome to Moonix!\n");
    extern void initInterrupt();    initInterrupt();
    extern void initMemory();       initMemory();
    extern void mapKernel();        mapKernel();
    extern void initFs();           initFs();
    extern void initThread();       initThread();
    extern void initTimer();        initTimer();
    extern void runCPU();           runCPU();
    // 不可能回到此处，因为 boot 线程的信息已经丢失
    while(1) {}
}