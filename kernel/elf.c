#include "types.h"
#include "def.h"
#include "mapping.h"
#include "elf.h"
#include "consts.h"

// 将 ELF 标志转换为页表项属性
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

// 新建用户进程页映射
// data 为用户进程 elf 起始字节指针
Mapping
newUserMapping(char *elf)
{
    Mapping m = newKernelMapping();
    ElfHeader *eHeader = (ElfHeader *)elf;
    // 校验 ELF 头
    if(eHeader->magic != ELF_MAGIC) {
        printf("Unknown file type: %p\n", eHeader->magic);
        panic("Unknown file type!");
    }
    ProgHeader *pHeader = (ProgHeader *)((usize)elf + eHeader->phoff);
    usize offset;
    int i;
    // 遍历所有的程序段
    for(i = 0, offset = (usize)pHeader; i < eHeader->phnum; i ++, offset += sizeof(ProgHeader)) {
        pHeader = (ProgHeader *)offset;
        if(pHeader->type != ELF_PROG_LOAD) {
            continue;
        }
        usize flags = convertElfFlags(pHeader->flags);
        usize vhStart = pHeader->vaddr, vhEnd = vhStart + pHeader->memsz;
        Segment segment = {vhStart, vhEnd, flags};
        char *source = (char *)((usize)elf + pHeader->off);
        mapFramedAndCopy(m, segment, source, pHeader->filesz);
    }
    return m;
}