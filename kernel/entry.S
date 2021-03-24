    .section .text.entry
    .globl _start
    # 仅仅是设置了 sp 就跳转到 main
_start:
    la sp, bootstacktop
    call main
    
    # 启动线程的内核栈 bootstack 放置在 bss 段的 stack 标记处
    .section .bss.stack
    .align 12
    .global bootstack
bootstack:
    # 以下 4096 × 16 字节的空间作为 OS 的启动栈
    .space 4096 * 16
    .global bootstacktop
bootstacktop: