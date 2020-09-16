#ifndef _DEFS_H
#define _DEFS_H
/*  printf.c    */
void putchar(int c);
void printf(char *fmt, ...);
void panic(char*) __attribute__((noreturn));

#endif