#include "types.h"
#include "defs.h"
#include "context.h"
#include "consts.h"
#include "riscv.h"
#include "process.h"

KernelStack
new_kernel_stack()
{
    return (KernelStack)malloc(KERNEL_STACK_SIZE);
}

uint64
top_kernel_stack(KernelStack self)
{
    return self + KERNEL_STACK_SIZE;
}

void
drop_kernel_stack(KernelStack self)
{
    free((void*)self);
}

Context
new_null_context()
{
    Context c;
    c.content_addr = 0;
    return c;
}

Thread
new_idle_thread()
{
    Context c = new_null_context();
    KernelStack k = new_kernel_stack();
    Thread t;
    t.context = c;
    t.kstack = k;
    return t;
}

ContextContent
new_kernel_context_content(uint64 entry, uint64 arg, uint64 kstack_top, uint64 satp)
{
    ContextContent content;
    content.ra = entry;
    content.satp = satp;
    content.s[0] = arg;
    content.s[1] = (r_sstatus() | SSTATUS_SPP);
    return content;
}

Context
push_content(ContextContent content, uint64 stack_top)
{
    uint64 ptr = stack_top - sizeof(ContextContent);
    *((ContextContent *)ptr) = content;
    Context c;
    c.content_addr = ptr;
    return c;
}

Context
new_kernel_context(uint64 entry, uint64 arg, uint64 kstack_top, uint64 satp)
{
    ContextContent cc = new_kernel_context_content(entry, arg, kstack_top, satp);
    return push_content(cc, kstack_top);
}

Thread
new_kernel_thread(uint64 entry, uint64 arg)
{
    KernelStack kstack = new_kernel_stack();
    Thread t;
    Context c = new_kernel_context(entry, arg, top_kernel_stack(kstack), r_satp());
    t.context = c;
    t.kstack = kstack;
    return t;
}

void switch_context(Context *self, Context *target) __attribute__((naked, noinline));

void
switch_to_thread(Thread *self, Thread *target)
{
    switch_context(&self->context, &target->context);
}

void
switch_context(Context *self, Context *target)
{
    asm(".include \"kernel/switch.asm\"");
}

void
hello_thread(uint64 arg)
{
    printf("hello thread\n");
    printf("arg is %d\n", arg);
    while(1) {}
}

void
init_process()
{
    Thread loop_thread = new_idle_thread();
    Thread new_thread = new_kernel_thread((uint64)hello_thread, 666);
    switch_to_thread(&loop_thread, &new_thread);
}