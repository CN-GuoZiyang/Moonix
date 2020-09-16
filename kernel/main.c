#include "defs.h"

extern void _start();
extern void bootstacktop();

void
main()
{
    printf("_start vaddr = %p\n", _start);
    printf("bootstacktop vaddr = %p\n", bootstacktop);
    panic("Nothing to do!\n");
}