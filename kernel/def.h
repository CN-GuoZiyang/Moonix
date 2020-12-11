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

/*  interrupt.c */
void initInterrupt();

#endif