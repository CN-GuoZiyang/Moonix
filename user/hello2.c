#include "types.h"
#include "ulib.h"

char *str = "Hello world!\n";

uint64
main()
{
    int i;
    for(i = 0; i < 10; i ++) {
        printf(str);
    }
    return 0;
}