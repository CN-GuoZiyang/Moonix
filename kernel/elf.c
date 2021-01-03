/*
 *  kernel/elf.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "mapping.h"
#include "elf.h"
#include "consts.h"

/* 
 * 将 ELF 标志转换为页表项属性
 * 目前只转换可读可写可执行三种属性，并且会直接设置 User 位以保证 U-Mode 访问
 */
usize
convertElfFlags(uint32 flags)
{
    usize ma = 1L;
    ma |= USER;
    if(flags & ELF_PROG_FLAG_EXEC) {
        ma |= EXECUTABLE;
    }
    if(flags & ELF_PROG_FLAG_WRITE) {
        ma |= WRITABLE;
    }
    if(flags & ELF_PROG_FLAG_READ) {
        ma |= READABLE;
    }
    return ma;
}

/*
 * 新建用户进程页映射
 * 参数 elf 为用户进程 elf 文件的起始字节指针
 */
Mapping
newUserMapping(char *elf)
{
    /* 直接获取一个已经映射完内核的地址空间 */
    Mapping m = newKernelMapping();
    ElfHeader *eHeader = (ElfHeader *)elf;
    /* 校验 ELF 头 */
    if(eHeader->magic != ELF_MAGIC) {
        printf("Unknown file type: %p\n", eHeader->magic);
        panic("Unknown file type!");
    }
    ProgHeader *pHeader = (ProgHeader *)((usize)elf + eHeader->phoff);
    usize offset;
    int i;
    /* 遍历所有的程序段，并映射类型为 LOAD 的段 */
    for(i = 0, offset = (usize)pHeader; i < eHeader->phnum; i ++, offset += sizeof(ProgHeader)) {
        pHeader = (ProgHeader *)offset;
        if(pHeader->type != ELF_PROG_LOAD) {
            continue;
        }
        usize flags = convertElfFlags(pHeader->flags);
        usize vhStart = pHeader->vaddr, vhEnd = vhStart + pHeader->memsz;
        Segment segment = {vhStart, vhEnd, flags};
        char *source = (char *)((usize)elf + pHeader->off);
        /* 
         * 映射的同时需要拷贝数据到实际的内存区域
         * 拷贝数据的大小是 filesz 而不是 memsz
         * 可能由于 bss 段的存在导致 filesz 小于 memsz
         * 多出的部分留空即可
         */
        mapFramedAndCopy(m, segment, source, pHeader->filesz);
    }
    return m;
}