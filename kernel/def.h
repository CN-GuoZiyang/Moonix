#ifndef _DEF_H
#define _DEF_H

#include "types.h"

/*  sbi.c    */
void consolePutchar(usize c);
usize consoleGetchar();
void shutdown() __attribute__((noreturn));
void setTimer(usize time);

/* printf.c */
void printf(char *, ...);
void panic(char*) __attribute__((noreturn));

/*  heap.c  */
void *malloc(uint32 size);
void free(void *ptr);

/*  memory.c    */
usize allocFrame();
void deallocFrame(usize ppn);

#endif