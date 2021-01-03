/*
 *  kernel/mapping.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "consts.h"
#include "mapping.h"

/* 根据虚拟页号得到其对应页表项在三级页表中的位置 */
void
getVpnLevels(usize vpn, usize *levels)
{
    levels[0] = (vpn >> 18) & 0x1ff;
    levels[1] = (vpn >> 9) & 0x1ff;
    levels[2] = vpn & 0x1ff;
}

/* 
 * 创建一个有根页表的映射
 * 只分配了三级页表的空间
 */
Mapping
newMapping()
{
    usize rootPaddr = allocFrame();
    Mapping m = {rootPaddr >> 12};
    return m;
}

/* 
 * 根据给定的虚拟页号寻找三级页表项
 * 如果某一级页表项为空，会创建下一级页表并填充
 */
PageTableEntry
*findEntry(Mapping self, usize vpn)
{
    PageTable *rootTable = (PageTable *)accessVaViaPa(self.rootPpn << 12);
    usize levels[3]; getVpnLevels(vpn, levels);
    PageTableEntry *entry = &(rootTable->entries[levels[0]]);
    int i;
    for(i = 1; i <= 2; i ++) {
        /* 页表不存在，创建新页表 */
        if(*entry == 0) {
            usize newPpn = allocFrame() >> 12;
            *entry = (newPpn << 10) | VALID;
        }
        usize nextPageAddr = (*entry & PDE_MASK) << 2;
        entry = &(((PageTable *)accessVaViaPa(nextPageAddr))->entries[levels[i]]);
    }
    return entry;
}

/*
 * 线性映射一个段
 * 段中的每一个虚拟地址都会按照固定偏移量线性映射到一个物理地址
 */
void
mapLinearSegment(Mapping self, Segment segment)
{
    usize startVpn = segment.startVaddr / PAGE_SIZE;
    usize endVpn = (segment.endVaddr - 1) / PAGE_SIZE + 1;
    usize vpn;
    for(vpn = startVpn; vpn < endVpn; vpn ++) {
        PageTableEntry *entry = findEntry(self, vpn);
        if(*entry != 0) {
            panic("Virtual address already mapped!\n");
        }
        *entry = ((vpn - KERNEL_PAGE_OFFSET) << 10) | segment.flags | VALID;
    }
}

/* 
 * 映射一个未被分配物理内存的段
 * 在映射时会实时分配物理内存并填充页表项
 */
void
mapFramedSegment(Mapping m, Segment segment)
{
    usize startVpn = segment.startVaddr / PAGE_SIZE;
    usize endVpn = (segment.endVaddr - 1) / PAGE_SIZE + 1;
    usize vpn;
    for(vpn = startVpn; vpn < endVpn; vpn ++) {
        PageTableEntry *entry = findEntry(m, vpn);
        if(*entry != 0) {
            panic("Virtual address already mapped!\n");
        }
        *entry = (allocFrame() >> 2) | segment.flags | VALID;
    }
}

/* 
 * 映射一个未被分配物理内存的段
 * 在映射时会实时分配物理内存并填充页表项
 * 并将数据复制到新分配的内存区域
 */
void
mapFramedAndCopy(Mapping m, Segment segment, char *data, usize length)
{
    usize s = (usize)data, l = length;
    usize startVpn = segment.startVaddr / PAGE_SIZE;
    usize endVpn = (segment.endVaddr - 1) / PAGE_SIZE + 1;
    usize vpn;
    for(vpn = startVpn; vpn < endVpn; vpn ++) {
        PageTableEntry *entry = findEntry(m, vpn);
        if(*entry != 0) {
            panic("Virtual address already mapped!\n");
        }
        usize pAddr = allocFrame();
        *entry = (pAddr >> 2) | segment.flags | VALID;
        /* 
         * 复制数据到目标位置
         * 访问目标位置还得通过虚拟地址访问
         */
        char *dst = (char *)accessVaViaPa(pAddr);
        if(l >= PAGE_SIZE) {
            char *src = (char *)s;
            int i;
            for(i = 0; i < PAGE_SIZE; i ++) {
                dst[i] = src[i];
            }
        } else {
            char *src = (char *)s;
            int i;
            for(i = 0; i < l; i ++) {
                dst[i] = src[i];
            }
            for(i = l; i < PAGE_SIZE; i ++) {
                dst[i] = 0;
            }
        }
        s += PAGE_SIZE;
        if(l >= PAGE_SIZE) l -= PAGE_SIZE;
        else l = 0;
    }
}

/*
 * 将页表地址写入 satp 中
 * 设置 satp 为 SV39，并刷新 TLB
 */
void
activateMapping(Mapping self)
{
    usize satp = self.rootPpn | (8L << 60);
    asm volatile("csrw satp, %0" : : "r" (satp));
    asm volatile("sfence.vma":::);
}

/* 
 * 创建一个映射了内核的虚拟地址空间
 * 在该地址空间中，内核的各个段按照固定的偏移被映射到虚拟地址空间的高地址空间处
 * 不激活
 */
Mapping
newKernelMapping()
{
    Mapping m = newMapping();
    
    /* .text 段，r-x */
    Segment text = {
        (usize)text_start,
        (usize)rodata_start,
        1L | READABLE | EXECUTABLE
    };
    mapLinearSegment(m, text);

    /* .rodata 段，r-- */
    Segment rodata = {
        (usize)rodata_start,
        (usize)data_start,
        1L | READABLE
    };
    mapLinearSegment(m, rodata);

    /* .data 段，rw- */
    Segment data = {
        (usize)data_start,
        (usize)bss_start,
        1L | READABLE | WRITABLE
    };
    mapLinearSegment(m, data);

    /* .bss 段，rw- */
    Segment bss = {
        (usize)bss_start,
        (usize)kernel_end,
        1L | READABLE | WRITABLE
    };
    mapLinearSegment(m, bss);

    /* 剩余空间，rw- */
    Segment other = {
        (usize)kernel_end,
        (usize)(MEMORY_END_PADDR + KERNEL_MAP_OFFSET),
        1L | READABLE | WRITABLE
    };
    mapLinearSegment(m, other);

    return m;
}

/* 
 * 由于启用外部中断和键盘中断需要修改一部分内存
 * 将这部分内存也线性映射到虚拟地址空间
 */
void
mapExtInterruptArea(Mapping m)
{
    Segment s1 = {
        (usize)0x0C000000 + KERNEL_MAP_OFFSET,
        (usize)0x0C001000 + KERNEL_MAP_OFFSET,
        1L | READABLE | WRITABLE
    };
    mapLinearSegment(m, s1);

    Segment s2 = {
        (usize)0x0C002000 + KERNEL_MAP_OFFSET,
        (usize)0x0C003000 + KERNEL_MAP_OFFSET,
        1L | READABLE | WRITABLE
    };
    mapLinearSegment(m, s2);

    Segment s3 = {
        (usize)0x0C201000 + KERNEL_MAP_OFFSET,
        (usize)0x0C202000 + KERNEL_MAP_OFFSET,
        1L | READABLE | WRITABLE
    };
    mapLinearSegment(m, s3);

    Segment s4 = {
        (usize)0x10000000 + KERNEL_MAP_OFFSET,
        (usize)0x10001000 + KERNEL_MAP_OFFSET,
        1L | READABLE | WRITABLE
    };
    mapLinearSegment(m, s4);
}

/* 重映射内核 */
void
mapKernel()
{
    Mapping m = newKernelMapping();
    mapExtInterruptArea(m);
    activateMapping(m);
}

/* 获得线性映射后的虚拟地址 */
usize
accessVaViaPa(usize pa)
{
    return pa + KERNEL_MAP_OFFSET;
}