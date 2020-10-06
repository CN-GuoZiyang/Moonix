#ifndef _CONTEXT_H
#define _CONTEXT_H

typedef struct trapframe
{
    uint64 x[32];
    uint64 sstatus, sepc, stval, scause;
} TrapFrame;

typedef struct context
{
    uint64 content_addr;
} Context;

typedef struct contextcontent
{
    uint64 ra;
    uint64 satp;
    uint64 s[12];
} ContextContent;

#endif