#include "types.h"
#include "ulib.h"

uint64
main()
{
    int i;
    char *c = malloc(8);
    for(i = 0; i < 8; i ++) {
        c[i] = i;
    }
    for(i = 0; i < 10; i ++) {
        printf("Hello world from user mode program!\n");
    }
    return 0;
}