/*
 *  user/entry.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "ulib.h"
#include "syscall.h"

/*
 * 弱链接 main 函数
 * 当没有 main 函数被链接时会链接此函数
 */
__attribute__((weak)) uint64
main()
{
    panic("No main linked!\n");
    return 1;
}

/*
 * 用户程序入口点
 * 初始化堆并调用 main
 */
void
_start(uint8 _args, uint8 *_argv)
{
    extern void initHeap();
    initHeap();
    sys_exit(main());
}