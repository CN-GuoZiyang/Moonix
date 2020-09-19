#include "defs.h"

extern void _start();
extern void bootstacktop();

void
main()
{
    init_interrupt();
    init_timer();
    asm volatile("ebreak");
    printf("return from trap!\n");
}