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

// 线程的入口函数
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
    // 根据当前线程，切换到另一个线程
    currentThread = !currentThread;
    switchContext(&contextAddr[!currentThread], &contextAddr[currentThread]);
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
        usize stackTop = (usize)threadStack[i] + 0x80000;
        // x2 即为 sp 寄存器，指向新线程的栈顶
        ic[i].x[2] = stackTop;
        // x10 - x17 用于参数传递
        ic[i].x[10] = (usize)(i?'B':'A');
        ic[i].x[11] = (usize)(i?'B':'A');
        // 由于借助中断返回机制初始化线程
        // 初始化结束后会执行 sret 跳转到 sepc 寄存器中存储的地址
        // 所以将线程逻辑的入口点存入 sepc
        ic[i].sepc = (usize)threadFunc;
        ic[i].sstatus = sstatus;
        // 线程的实际入口点是中断返回时的 __restore 函数
        // 需要借助该函数进行初始化寄存器
        tc[i].ra = (usize) __restore;
        tc[i].ic = ic[i];
        ThreadContext *ca = (ThreadContext *)(stackTop - sizeof(ThreadContext));
        *ca = tc[i];
        contextAddr[i] = (usize)ca;
    }

    // 声明成局部变量，后续不需要再切换回启动线程
    usize bootAddr;
    switchContext(&bootAddr, &contextAddr[0]);
}