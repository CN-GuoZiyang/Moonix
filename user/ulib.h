#ifndef _ULIB_H
#define _ULIB_H

/*  io.c    */
uint8 getc();
void printf(char *, ...);
void panic(char*);
void putchar(int c);

/*  malloc.c    */
void *malloc(uint32 size);
void free(void *ptr);

/*  string.c    */
int strcmp(char *str1, char *str2);

#endif