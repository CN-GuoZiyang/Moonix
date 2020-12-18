#include "types.h"
#include "def.h"
#include "consts.h"
#include "thread.h"
#include "riscv.h"

usize
newKernelStack()
{
    usize bottom = (usize)malloc(KERNEL_STACK_SIZE);
    return bottom;
}

__attribute__((naked, noinline)) void
switchContext(usize *self, usize *target)
{
    asm volatile(".include \"kernel/switch.asm\"");
}

void
switchThread(Thread *self, Thread *target)
{
    switchContext(&self->contextAddr, &target->contextAddr);
}

usize
pushContextToStack(ThreadContext self, usize stackTop)
{
    ThreadContext *ptr = (ThreadContext *)(stackTop - sizeof(ThreadContext));
    *ptr = self;
    return (usize)ptr;
}

usize
newKernelThreadContext(usize entry, usize kernelStackTop, usize satp)
{
    InterruptContext ic;
    ic.x[2] = kernelStackTop;
    ic.sepc = entry;
    ic.sstatus = r_sstatus();
    ic.sstatus |= SSTATUS_SPP;
    ic.sstatus |= SSTATUS_SPIE;
    ic.sstatus &= ~SSTATUS_SIE;
    ThreadContext tc;
    extern void __restore(); tc.ra = (usize)__restore;
    tc.satp = satp;
    tc.ic = ic;
    return pushContextToStack(tc, kernelStackTop);
}

void
appendArguments(Thread *thread, usize args[8])
{
    ThreadContext *ptr = (ThreadContext *)thread->contextAddr;
    ptr->ic.x[10] = args[0];
    ptr->ic.x[11] = args[1];
    ptr->ic.x[12] = args[2];
    ptr->ic.x[13] = args[3];
    ptr->ic.x[14] = args[4];
    ptr->ic.x[15] = args[5];
    ptr->ic.x[16] = args[6];
    ptr->ic.x[17] = args[7];
}

Thread
newKernelThread(usize entry)
{
    usize stackBottom = newKernelStack();
    usize contextAddr = newKernelThreadContext(
        entry,
        stackBottom + KERNEL_STACK_SIZE,
        r_satp()
    );
    Thread t = {
        contextAddr, stackBottom
    };
    return t;
}

void
tempThreadFunc(Thread *from, Thread *current, usize c)
{
    printf("The char passed by is ");
    consolePutchar(c);
    consolePutchar('\n');
    printf("Hello world from tempThread!\n");
    switchThread(current, from);
}

Thread
newBootThread()
{
    Thread t = {
        0L, 0L
    };
    return t;
}

void
initThread()
{
    Thread bootThread = newBootThread();
    Thread tempThread = newKernelThread((usize)tempThreadFunc);
    usize args[8];
    args[0] = (usize)&bootThread;
    args[1] = (usize)&tempThread;
    args[2] = (long)'M';
    appendArguments(&tempThread, args);
    switchThread(&bootThread, &tempThread);
    printf("I'm back from tempThread!\n");
}