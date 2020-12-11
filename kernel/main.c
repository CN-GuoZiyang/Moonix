#include "types.h"
#include "def.h"

asm(".include \"kernel/entry.asm\"");

void
main()
{
    initInterrupt();
    asm volatile("ebreak":::);
    printf("Return here!\n");
}