#ifndef _ULIB_H
#define _ULIB_H

/*  io.c    */
void printf(char *, ...);
void panic(char*);
void putchar(int c);

/*  malloc.c    */
void *malloc(uint32 size);
void free(void *ptr);

#endif