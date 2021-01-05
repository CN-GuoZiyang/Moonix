/*
 *  user/hello.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "ulib.h"

uint64
main()
{
    int i;
    for(i = 0; i < 10; i ++) {
        printf("Hello world from user mode program 1!\n");
    }
    return 0;
}