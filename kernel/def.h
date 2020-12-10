#ifndef _DEF_H
#define _DEF_H

/*  io.c    */
void console_putchar(usize c);
usize console_getchar();
void shutdown() __attribute__((noreturn));

/* printf.c */
void printf(char *, ...);
void panic(char*) __attribute__((noreturn));

#endif