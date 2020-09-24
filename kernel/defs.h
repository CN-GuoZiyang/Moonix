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

/*  memory.c   */
uint64 alloc();
void dealloc(uint64 n);
void init_memory();

/*  heap.c  */
void* malloc(uint size);
void free(void* ptr);

/*  heap.c  */
void init_buddy_system();

/*  paging.c    */
void remap_kernel();
#endif