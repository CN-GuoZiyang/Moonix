#ifndef _CONTEXT_H
#define _CONTEXT_H

typedef struct trapframe
{
    uint64 x[32];       // General register
    uint64 sstatus;     // Supervisor status register
    uint64 sepc;        // Supervisor exception program counter
    uint64 stval;       // Supervisor trap value
    uint64 scause;      // Scause register
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