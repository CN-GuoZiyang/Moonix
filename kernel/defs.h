#ifndef _DEF_H
#define _DEF_H

/* printf.c */
void printf(char *, ...);
void panic(char*) __attribute__((noreturn));

/* interrupt.c */
void init_interrupt();

/* timer.c */
void init_timer();
void clock_set_next_event();

/* frame_allocator.c */
void init_memory();
uint64 alloc_frame();
void dealloc_frame(uint64 n);

/* paging.c */
void remap_kernel();

/*  heap.c  */
void* malloc(uint size);
void free(void* ptr);
void init_buddy_system();

/*  process.c   */
void init_process();
void tick();
void run();
void exit(uint64 code);

/*  syscall.c   */
uint64 syscall(uint64 id, uint64 args[3], TrapFrame *tf);

#endif