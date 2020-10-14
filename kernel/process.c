#include "types.h"
#include "defs.h"
#include "context.h"
#include "consts.h"
#include "riscv.h"
#include "process.h"
#include "scheduler.h"

extern void __trapret();

Processor CPU;

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
get_boot_thread()
{
    Context c = new_null_context();
    KernelStack k = new_kernel_stack();
    Thread t;
    t.context = c;
    t.kstack = k;
    return t;
}

ContextContent
new_kernel_context_content(uint64 entry, uint64 kstack_top, uint64 satp)
{
    ContextContent content;
    content.ra = (uint64)__trapret;
    content.satp = satp;
    TrapFrame tf;
    tf.x[2] = kstack_top;
    tf.sepc = entry;
    tf.sstatus = r_sstatus() | SSTATUS_SPP | SSTATUS_SPIE;
    tf.sstatus &= ~SSTATUS_SIE;
    content.tf = tf;
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
new_kernel_context(uint64 entry, uint64 kstack_top, uint64 satp)
{
    ContextContent cc = new_kernel_context_content(entry, kstack_top, satp);
    return push_content(cc, kstack_top);
}

void
append_initial_arguments(Thread *self, uint64 args[3])
{
    ContextContent *cc = (ContextContent *)(self->context.content_addr);
    cc->tf.x[10] = args[0];
    cc->tf.x[11] = args[1];
    cc->tf.x[12] = args[2];
}

Thread
new_kernel_thread(uint64 entry)
{
    KernelStack kstack = new_kernel_stack();
    Thread t;
    Context c = new_kernel_context(entry, top_kernel_stack(kstack), r_satp());
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

Tid
alloc_tid(ThreadPool *self)
{
    int i;
    for(i = 0; i < MAX_THREAD_NUM; i ++) {
        if(self->occupied[i] == 0) {
            self->occupied[i] = 1;
            return i;
        }
    }
    panic("Alloc tid failed!");
    return -1;
}

void
add_thread_to_thread_pool(ThreadPool *self, Thread thread)
{
    Tid tid = alloc_tid(self);
    ThreadInfo ti;
    ti.present = 1;
    ti.status = Ready;
    ti.thread = thread;
    self->threads[tid] = ti;
    scheduler_push(tid);
}

RunningThread
thread_pool_acquire(ThreadPool *self)
{
    Tid tid = scheduler_pop();
    RunningThread rt;
    rt.tid = tid;
    if(tid != -1) {
        ThreadInfo *ti = &self->threads[tid];
        ti->status = Running;
        ti->tid = tid;
        rt.thread = &ti->thread;
    }
    return rt;
}

void
thread_pool_retrieve(ThreadPool *self, Tid tid, Thread *thread)
{
    ThreadInfo *ti = &self->threads[tid];
    if(ti->present) {
        ti->thread = *thread;
        ti->status = Ready;
        scheduler_push(tid);
    }
}

int
thread_pool_tick()
{
    return scheduler_tick();
}

void
thread_pool_exit(ThreadPool *self, Tid tid, uint64 code)
{
    drop_kernel_stack(self->threads[tid].thread.kstack);
    ThreadInfo ti;
    ti.status = Ready;
    ti.present = 0;
    self->threads[tid] = ti;
    self->occupied[tid] = 0;
    scheduler_exit(tid);
    printf("Exit code %d\n", code);
}

void
init_processor(Processor *self, Thread idle, ThreadPool *pool)
{
    self->pool = *pool;
    self->idle = idle;
    self->hasCurrent = 0;
}

void
add_thread_to_processor(Processor *self, Thread thread)
{
    add_thread_to_thread_pool(&self->pool, thread);
}

void
idle_main(Processor *self)
{
    disable_and_store();
    while(1) {
        RunningThread thread = thread_pool_acquire(&self->pool);
        if(thread.tid != -1) {
            self->current = thread;
            self->hasCurrent = 1;
            printf("Process %d ready to run\n", thread.tid);
            switch_to_thread(&self->idle, self->current.thread);
            RunningThread rt = self->current;
            printf("\nThread %d ran just now\n", rt.tid);
            thread_pool_retrieve(&self->pool, rt.tid, rt.thread);
        } else {
            enable_and_wfi();
            disable_and_store();
        }
    }
}

void
processor_tick(Processor *self)
{
    if(self->hasCurrent) {
        if(thread_pool_tick()) {
            uint64 flags = disable_and_store();
            switch_to_thread(self->current.thread, &self->idle);
            restore_sstatus(flags);
        }
    }
}

void
processor_exit(Processor *self, uint64 code)
{
    Tid tid = self->current.tid;
    self->hasCurrent = 0;
    thread_pool_exit(&self->pool, tid, code);
    switch_to_thread(self->current.thread, &self->idle);
    while(1) {}
}

void
hello_thread(uint64 arg)
{
        printf("thread is %d\n", arg);
        int i;
        for(i = 0; i < 100; i ++) {
            printf("%d%d%d%d%d%d%d%d", arg, arg, arg, arg, arg, arg, arg, arg);
            int j;
            for(j = 0; j < 5000; j ++) {}
        }
        
    printf("\nend of thread %d\n", arg);
    processor_exit(&CPU, 0);
}

void
tick()
{
    processor_tick(&CPU);
}

void
processor_run(Processor *self)
{
    Thread boot = get_boot_thread();
    switch_to_thread(&boot, &self->idle);
}

void
init_process()
{
    init_scheduler(1);
    ThreadPool pool;
    Thread idle = new_kernel_thread((uint64)idle_main);
    append_initial_arguments(&idle, (uint64[]){(uint64)&CPU, 0, 0});
    init_processor(&CPU, idle, &pool);
    int i;
    for(i = 0; i < 5; i ++) {
        Thread t = new_kernel_thread((uint64)hello_thread);
        append_initial_arguments(&t, (uint64[]){i, 0, 0});
        add_thread_to_processor(&CPU, t);
    }
    processor_run(&CPU);
}