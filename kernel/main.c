#include "types.h"
#include "defs.h"

void
main()
{
    init_memory();
    init_interrupt();
    init_timer();
    asm volatile("ebreak");
    printf("return from trap!\n");
}