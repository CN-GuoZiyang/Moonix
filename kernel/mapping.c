#include "types.h"
#include "def.h"
#include "consts.h"
#include "mapping.h"

// 根据虚拟页号得到三级页号
usize
*getVpnLevels(usize vpn)
{
    static usize res[3];
    res[0] = (vpn >> 18) & 0x1ff;
    res[1] = (vpn >> 9) & 0x1ff;
    res[2] = vpn & 0x1ff;
    return res;
}

// 创建一个有根页表的映射
Mapping
newMapping()
{
    usize rootPaddr = allocFrame();
    Mapping m = {rootPaddr >> 12};
    return m;
}

// 根据给定的虚拟页号寻找三级页表项
// 如果找不到对应页表项就会创建页表
PageTableEntry
*findEntry(Mapping self, usize vpn)
{
    PageTable *rootTable = (PageTable *)accessVaViaPa(self.rootPpn << 12);
    usize *levels = getVpnLevels(vpn);
    PageTableEntry *entry = &(rootTable->entries[levels[0]]);
    int i;
    for(i = 1; i <= 2; i ++) {
        if(*entry == 0) {
            // 页表不存在，创建新页表
            usize newPpn = allocFrame() >> 12;
            *entry = (newPpn << 10) | VALID;
        }
        usize nextPageAddr = (*entry & PDE_MASK) << 2;
        entry = &(((PageTable *)accessVaViaPa(nextPageAddr))->entries[levels[i]]);
    }
    return entry;
}

// 线性映射一个段，填充页表
void
mapLinarSegment(Mapping self, Segment segment)
{
    usize startVpn = segment.startVaddr >> 12;
    usize endVpn = segment.endVaddr >> 12;
    if((segment.endVaddr & 0x0111L) == 0) {
        endVpn --;
    }
    usize vpn;
    for(vpn = startVpn; vpn <= endVpn; vpn ++) {
        PageTableEntry *entry = findEntry(self, vpn);
        if(*entry != 0) {
            panic("Virtual address already mapped!\n");
        }
        *entry = ((vpn - KERNEL_PAGE_OFFSET) << 10) | segment.flags | VALID;
    }
}

// 激活页表
void
activateMapping(Mapping self)
{
    usize satp = self.rootPpn | (8L << 60);
    asm volatile("csrw satp, %0" : : "r" (satp));
    asm volatile("sfence.vma":::);
}

// 映射内核，并返回页表（不激活）
Mapping
newKernelMapping()
{
    Mapping m = newMapping();
    
    // .text 段，r-x
    Segment text = {
        (usize)text_start,
        (usize)rodata_start,
        1L | READABLE | EXECUTABLE
    };
    mapLinarSegment(m, text);

    // .rodata 段，r--
    Segment rodata = {
        (usize)rodata_start,
        (usize)data_start,
        1L | READABLE
    };
    mapLinarSegment(m, rodata);

    // .data 段，rw-
    Segment data = {
        (usize)data_start,
        (usize)bss_start,
        1L | READABLE | WRITABLE
    };
    mapLinarSegment(m, data);

    // .bss 段，rw-
    Segment bss = {
        (usize)bss_start,
        (usize)kernel_end,
        1L | READABLE | WRITABLE
    };
    mapLinarSegment(m, bss);

    // 剩余空间，rw-
    Segment other = {
        (usize)kernel_end,
        (usize)(MEMORY_END_PADDR + KERNEL_MAP_OFFSET),
        1L | READABLE | WRITABLE
    };
    mapLinarSegment(m, other);

    return m;
}

// 映射内核
void
mapKernel()
{
    Mapping m = newKernelMapping();
    activateMapping(m);
    printf("***** Remap Kernel *****\n");
}

// 获得线性映射后的虚拟地址
usize
accessVaViaPa(usize pa)
{
    return pa + KERNEL_MAP_OFFSET;
}