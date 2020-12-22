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
void *kalloc(uint32 size);
void kfree(void *ptr);

/*  memory.c    */
usize allocFrame();
void deallocFrame(usize ppn);

/*  processor.c    */
void exitFromCPU(usize code);

/*  string.c    */
int strlen(char *str);
int strcmp(char *str1, char *str2);

#endif