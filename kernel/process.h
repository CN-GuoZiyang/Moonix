#ifndef _PROCESS_H
#define _PROCESS_H

typedef uint64 KernelStack;

typedef struct thread
{
    Context context;
    KernelStack kstack;
} Thread;


#endif