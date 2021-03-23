#include "types.h"
#include "sbi.h"
#include "context.h"

// 线程上下文保存的地址
usize contextAddrA, contextAddrB;
// 线程栈空间
char stackA[0x80000], stackB[0x80000];

// 当前是否是线程A在运行，用来控制切换
int inThreadA = 1;

__attribute__((naked, noinline)) void
switchContext(usize *from, usize *target)
{
    asm volatile(".include \"kernel/switch.asm\"");
}

void
threadFunc(usize c1, usize c2)
{
    while(1) {
        // 玄学，写成一个就只能输出一次
        // 可能是虚拟机的问题
        consolePutchar(c1);
        consolePutchar(c2);
    }
}

void
switchToAnother()
{
    if(inThreadA) {
        inThreadA = 0;
        switchContext(&contextAddrA, &contextAddrB);
    } else {
        inThreadA = 1;
        switchContext(&contextAddrB, &contextAddrA);
    }
}

extern void __restore();

// 构造线程结构，并借助中断恢复的函数进行线程初始化
void
initThread()
{
    uint64 sstatus;
    asm volatile("csrr %0, sstatus":"=r"(sstatus));
    // sret 后特权级为 S-Mode
    sstatus |= (1L << 8);
    // 内核线程使能异步中断
    sstatus |= (1L << 5);

    InterruptContext icA;
    icA.x[2] = (usize)stackA + 0x80000;
    icA.x[10] = (usize)'A';
    icA.x[11] = (usize)'A';
    icA.sepc = (usize)threadFunc;
    icA.sstatus = sstatus;
    ThreadContext tcA;
    tcA.ra = (usize) __restore;
    tcA.ic = icA;
    ThreadContext *caA = (ThreadContext *)((usize)stackA + 0x80000 - sizeof(ThreadContext));
    *caA = tcA;
    contextAddrA = (usize)caA;

    InterruptContext icB;
    icB.x[2] = (usize)stackB + 0x80000;
    icB.x[10] = (usize)'B';
    icB.x[11] = (usize)'B';
    icB.sepc = (usize)threadFunc;
    icB.sstatus = sstatus;
    ThreadContext tcB;
    tcB.ra = (usize) __restore;
    tcB.ic = icB;
    ThreadContext *caB = (ThreadContext *)((usize)stackB + 0x80000 - sizeof(ThreadContext));
    *caB = tcB;
    contextAddrB = (usize)caB;

    // 声明成局部变量，后续不需要再切换回启动线程
    usize bootAddr;
    switchContext(&bootAddr, &contextAddrA);
}