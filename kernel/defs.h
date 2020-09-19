#ifndef _DEFS_H
#define _DEFS_H
/*  printf.c    */
void putchar(int c);
void printf(char *fmt, ...);
void panic(char*) __attribute__((noreturn));

/*  interrupt.c */
void init_interrupt();

/*  timer.c */
void set_next_clock();
void init_timer();

/*  frame_allocator.c   */
void init_sta(uint64 l, uint64 r);
uint64 alloc();
void dealloc(uint64 n);

/*  memory.c    */
void init_memory();
#endif