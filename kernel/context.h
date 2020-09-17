#ifndef _CONTEXT_H
#define _CONTEXT_H

typedef struct trapframe
{
    uint64 x[32];
    uint64 sstatus, sepc, stval, scause;
} TrapFrame;



#endif