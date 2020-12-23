#include "types.h"
#include "ulib.h"

uint64
main()
{
    int t1 = 0, t2 = 1, nextTerm = 0, n = 50;
 
    // 显示前两项
    printf("fibonacci: %d, %d", t1, t2);
 
    nextTerm = t1 + t2;
 
    while(nextTerm <= n)
    {
        printf(", %d",nextTerm);
        t1 = t2;
        t2 = nextTerm;
        nextTerm = t1 + t2;
    }
    printf("\n");
    
    return 0;
}