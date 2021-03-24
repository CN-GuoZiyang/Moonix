#include "types.h"
#include "sbi.h"
#include "context.h"

// 线程上下文保存的地址
usize contextAddr[2];
// 线程栈空间
char threadStack[2][0x80000];

// 当前正在运行的线程编号，用来控制切换
int currentThread = 0;

__attribute__((naked, noinline)) void
switchContext(usize *from, usize *to)
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
    if(currentThread) {
        currentThread = 0;
        switchContext(&contextAddr[1], &contextAddr[0]);
    } else {
        currentThread = 1;
        switchContext(&contextAddr[0], &contextAddr[1]);
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

    InterruptContext ic[2];
    ThreadContext tc[2];
    int i = 0;
    for(i = 0; i < 2; i ++) {
        ic[i].x[2] = (usize)&threadStack[i] + 0x80000;
        ic[i].x[10] = (usize)(i?'B':'A');
        ic[i].x[11] = (usize)(i?'B':'A');
        ic[i].sepc = (usize)threadFunc;
        ic[i].sstatus = sstatus;
        tc[i].ra = (usize) __restore;
        tc[i].ic = ic[i];
        ThreadContext *ca = (ThreadContext *)((usize)&threadStack[i] + 0x80000 - sizeof(ThreadContext));
        *ca = tc[i];
        contextAddr[i] = (usize)ca;
    }

    // 声明成局部变量，后续不需要再切换回启动线程
    usize bootAddr;
    switchContext(&bootAddr, &contextAddr[0]);
}