    .section .text.entry
    .globl _start
    # 仅仅是设置了 sp 就跳转到 main
_start:
    la sp, bootstacktop
    call main

    .section .bss.stack
    .align 12
    # 以下 4096 × 16 字节的空间作为 OS 的启动栈
    .global bootstack
bootstack:
    .space 4096 * 16
    .global bootstacktop
bootstacktop: