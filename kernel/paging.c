#include "types.h"
#include "defs.h"
#include "riscv.h"
#include "consts.h"

typedef uint64 MemoryAttr;
typedef void *InactivePageTable;

// text段开始和结束
extern void stext();
extern void etext();

MemoryAttr
new_memory_attr()
{
    return 1L;
}

void
set_attr_readonly(MemoryAttr *attr)
{
    *attr |= (1<<1);
}

void
set_attr_execute(MemoryAttr *attr)
{
    *attr |= (1<<3);
}

void
set_attr_WR(MemoryAttr *attr)
{
    *attr |= (1<<1);
    *attr |= (1<<2);
}

// Physical address of page table
InactivePageTable pageTable;

uint64
get_vpn1(uint64 addr)
{
    return (addr>>30)&0x1ff;
}

uint64
get_vpn2(uint64 addr)
{
    return (addr>>21)&0x1ff;
}

uint64
get_vpn3(uint64 addr)
{
    return (addr>>12)&0x1ff;
}

uint64
pgtable_vaddr(InactivePageTable pg_table)
{
    return (uint64)pg_table + PHYSICAL_MEMORY_OFFSET;
}

void
set_pg_table(InactivePageTable table, uint64 start, uint64 end, MemoryAttr attr)
{
    // 4k对齐
    uint64 vaddr = start & (~0xfffL);
    // 三级页表空间
    MemoryAttr *pg_table1 = (MemoryAttr *)pgtable_vaddr(table);
    while(vaddr < end) {
        uint64 vpn1 = get_vpn1(vaddr);
        uint64 pde1 = pg_table1[vpn1];
        // 本条目还没有初始化
        if(pde1 == 0) {
            uint64 pde1_ppn = alloc_frame();
            pg_table1[vpn1] = (pde1_ppn << 10) | 0x1;
        }
        // 二级页表空间
        uint64 pg_table2_paddr = (pg_table1[vpn1] & PDE_ADDRESS_MASK) << 2;
        MemoryAttr *pg_table2 = (MemoryAttr *)(pg_table2_paddr + PHYSICAL_MEMORY_OFFSET);
        uint64 vpn2 = get_vpn2(vaddr);
        uint64 pde2 = pg_table2[vpn2];
        if(pde2 == 0) {
            uint64 pde2_ppn = alloc_frame();
            pg_table2[vpn2] = (pde2_ppn << 10) | 0x1;
        }
        // 一级页表空间
        uint64 pg_table3_paddr = (pg_table2[vpn2] & PDE_ADDRESS_MASK) << 2;
        MemoryAttr *pg_table3 = (MemoryAttr *)(pg_table3_paddr + PHYSICAL_MEMORY_OFFSET);
        // 填充页表项
        pg_table3[get_vpn3(vaddr)] = ((vaddr - PHYSICAL_MEMORY_OFFSET) >> 12) << 10 | attr;
        vaddr += (1 << 12);
    }
}

void
activate_page_table(InactivePageTable pt)
{
    w_satp(((uint64)pt >> 12) | (8L << 60));
    flush_tlb();
}

void
remap_kernel()
{
    pageTable = (InactivePageTable)(alloc_frame() << 12);

    MemoryAttr ma = new_memory_attr();
    set_attr_readonly(&ma); set_attr_execute(&ma);
    set_pg_table(pageTable, (uint64)stext, (uint64)etext, ma);

    ma = new_memory_attr();
    set_attr_WR(&ma);
    set_pg_table(pageTable, (uint64)etext, PHYSICAL_MEMORY_END+PHYSICAL_MEMORY_OFFSET, ma);

    activate_page_table(pageTable);
}