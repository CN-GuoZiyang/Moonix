#include "defs.h"

extern void _start();
extern void bootstacktop();

void
main()
{
    init_interrupt();
    asm volatile("ebreak");
    printf("return from trap!\n");
}