#include "types.h"
#include "def.h"

asm(".include \"kernel/entry.asm\"");

void
testHeap()
{
    char *c1 = malloc(64);
    printf("c1: %p\n", c1);
    char *c2 = malloc(63);
    printf("c2: %p\n", c2);
    c1[0] = 'c';
    free(c1);
    c1 = malloc(65);
    printf("c1: %p\n", c1);
    char *c3 = malloc(62);
    printf("c3: %p\n", c3);
}

void
main()
{
    initInterrupt();
    asm volatile("ebreak":::);
    printf("Return here!\n");
    extern void initHeap(); initHeap();
    testHeap();
}