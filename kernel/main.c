/*
 *  kernel/main.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"

asm(".include \"kernel/entry.asm\"");
asm(".include \"kernel/linkFS.asm\"");

/*
 * main 函数在启动线程中被调用
 * 主要进行各个模块的初始化
 * 在最后 runCPU 时会启动终端用户线程
 * 在切换时由于使用局部变量保存的线程信息
 * 会丢失启动线程的信息，无法再被切换回来
 */
void
main()
{
    printf("Initializing Moonix...\n");
    extern void initMemory();       initMemory();
    extern void initInterrupt();    initInterrupt();
    extern void initFs();           initFs();
    extern void initThread();       initThread();
    extern void initTimer();        initTimer();
    extern void runCPU();           runCPU();
    /* 不可能回到此处，因为启动线程的信息已经丢失 */
    while(1) {}
}