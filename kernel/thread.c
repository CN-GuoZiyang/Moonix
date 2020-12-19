#include "types.h"
#include "def.h"
#include "consts.h"
#include "thread.h"
#include "riscv.h"

usize
newKernelStack()
{
    // 将内核线程的线程栈分配在堆空间
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
    // 将线程上下文压入栈底
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
    // 设置返回后的特权级为 S-Mode
    ic.sstatus |= SSTATUS_SPP;
    // 异步中断使能
    ic.sstatus |= SSTATUS_SPIE;
    ic.sstatus &= ~SSTATUS_SIE;
    ThreadContext tc;
    extern void __restore(); tc.ra = (usize)__restore;
    tc.satp = satp;
    tc.ic = ic;
    return pushContextToStack(tc, kernelStackTop);
}

void
appendArguments(Thread thread, usize args[8])
{
    ThreadContext *ptr = (ThreadContext *)thread.contextAddr;
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
helloThread(usize arg)
{
    printf("Begin of thread %d\n", arg);
    int i;
    for(i = 0; i < 800; i ++) {
        printf("%d", arg);
    }
    printf("\nEnd of thread %d\n", arg);
    exitFromCPU(0);
    while(1) {}
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
    Scheduler s = {
        schedulerInit,
        schedulerPush,
        schedulerPop,
        schedulerTick,
        schedulerExit
    };
    s.init();
    ThreadPool pool = newThreadPool(s);
    Thread idle = newKernelThread((usize)idleMain);
    initCPU(idle, pool);
    usize i;
    for(i = 0; i < 5; i ++) {
        Thread t = newKernelThread((usize)helloThread);
        usize args[8];
        args[0] = i;
        appendArguments(t, args);
        addToCPU(t);
    }
    printf("***** init thread *****\n");
}