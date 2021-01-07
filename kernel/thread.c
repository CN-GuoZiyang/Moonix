/*
 *  kernel/thread.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "consts.h"
#include "thread.h"
#include "riscv.h"
#include "elf.h"
#include "mapping.h"
#include "fs.h"

usize
newKernelStack()
{
    /* 将内核线程的线程栈分配在内核堆中 */
    usize bottom = (usize)kalloc(KERNEL_STACK_SIZE);
    return bottom;
}

/*
 * 该函数用于切换上下文，保存当前函数的上下文，并恢复目标函数的上下文
 * 函数返回时即返回到了新线程的运行位置
 * naked 防止 gcc 自动插入开场白和结束语，这部分需要自行保存恢复
 * noinline 防止 gcc 将函数内联，上下文切换借助了函数调用返回，不应内联
 */
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
    /*
     * 将线程上下文压入栈顶
     * 并返回新的栈顶地址
     */
    ThreadContext *ptr = (ThreadContext *)(stackTop - sizeof(ThreadContext));
    *ptr = self;
    return (usize)ptr;
}

usize
newKernelThreadContext(usize entry, usize kernelStackTop, usize satp)
{
    /*
     * 创建新的内核线程
     * 借助中断恢复机制进行线程的初始化工作
     * 从中断恢复结束时即跳转到 sepc，就是线程的入口点
     */
    InterruptContext ic;
    ic.x[2] = kernelStackTop;
    ic.sepc = entry;
    ic.sstatus = r_sstatus();
    /* 内核线程，返回后特权级为 S-Mode */
    ic.sstatus |= SSTATUS_SPP;
    /* 开启新线程异步中断使能 */
    ic.sstatus |= SSTATUS_SPIE;
    ic.sstatus &= ~SSTATUS_SIE;
    ThreadContext tc;
    extern void __restore(); tc.ra = (usize)__restore;
    tc.satp = satp;
    tc.ic = ic;
    return pushContextToStack(tc, kernelStackTop);
}

usize
newUserThreadContext(usize entry, usize ustackTop, usize kstackTop, usize satp)
{
    InterruptContext ic;
    /* 新线程使用的栈为用户栈 */
    ic.x[2] = ustackTop;
    ic.sepc = entry;
    ic.sstatus = r_sstatus();
    /* 用户线程，返回后的特权级为 U-Mode */
    ic.sstatus &= ~SSTATUS_SPP;
    ic.sstatus |= SSTATUS_SPIE;
    ic.sstatus &= ~SSTATUS_SIE;
    ThreadContext tc;
    extern void __restore(); tc.ra = (usize)__restore;
    tc.satp = satp;
    tc.ic = ic;
    return pushContextToStack(tc, kstackTop);
}

/*
 * 借助函数调用机制向新线程传递参数
 * 最多可传递 8 个参数
 */
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
    Process p;
    p.satp = r_satp();
    int i;
    for(i = 0; i < 3; i ++) p.fdOccupied[i] = 1;
    usize contextAddr = newKernelThreadContext(
        entry,
        stackBottom + KERNEL_STACK_SIZE,
        p.satp
    );
    Thread t = {
        contextAddr, stackBottom, p, -1
    };
    return t;
}

Thread
newUserThread(char *data)
{
    /* 解析 ELF 文件，完成内核和可执行程序各个段的映射 */
    Mapping m = newUserMapping(data);
    usize ustackBottom = USER_STACK_OFFSET, ustackTop = USER_STACK_OFFSET + USER_STACK_SIZE;
    /* 映射用户栈 */
    Segment s = {ustackBottom, ustackTop, 1L | USER | READABLE | WRITABLE};
    mapFramedSegment(m, s);

    usize kstack = newKernelStack();
    usize entryAddr = ((ElfHeader *)data)->entry;
    Process p;
    p.satp = m.rootPpn | (8L << 60);
    int i;
    for(i = 0; i < 3; i ++) p.fdOccupied[i] = 1;
    usize context = newUserThreadContext(
        entryAddr,
        ustackTop,
        kstack + KERNEL_STACK_SIZE,
        p.satp
    );
    Thread t = {context, kstack, p, -1};
    return t;
}

Thread
newBootThread()
{
    /*
     * 该函数调用时，应当处于启动线程内
     * 只需要创建一个空结构，在进行切换时即可自动填充
     */
    Thread t;
    t.contextAddr = 0L;
    t.kstack = 0L;
    t.wait = -1;
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

    /* 启动终端 */
    Inode *shInode = lookup(0, "/bin/sh");
    char *buf = kalloc(shInode->size);
    readall(shInode, buf);
    Thread t = newUserThread(buf);
    kfree(buf);

    addToCPU(t);
    printf("***** init thread *****\n");
}

int
allocFd(Thread *thread)
{
    int i = 0;
    for(i = 0; i < 16; i ++) {
        if(!thread->process.fdOccupied[i]) {
            return i;
        }
    }
    return -1;
}

void
deallocFd(Thread *thread, int fd)
{
    thread->process.fdOccupied[fd] = 0;
}