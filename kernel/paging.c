#include "types.h"
#include "defs.h"
#include "riscv.h"
#include "consts.h"

typedef uint64 MemoryAttr;
typedef void *PageTablePaddr;

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
pgtable_vaddr(PageTablePaddr pg_table)
{
    return (uint64)pg_table + PHYSICAL_MEMORY_OFFSET;
}

static PageTablePaddr ptp;

void
set_pg_table(PageTablePaddr table, uint64 start, uint64 end, MemoryAttr attr)
{
    uint64 svaddr = start & (~0xfffL);
    MemoryAttr *pg_table1 = (MemoryAttr *)pgtable_vaddr(table);
    while(svaddr < end) {
        uint64 vpn1 = get_vpn1(svaddr);
        uint64 pde1 = pg_table1[vpn1];
        if(pde1 == 0) {
            uint64 pde1_ppn = alloc();
            pg_table1[vpn1] = (pde1_ppn << 10) | 0x1;
        }
        uint64 pg_table2_paddr = (pg_table1[vpn1] & PDE_ADDRESS_MASK) << 2;
        MemoryAttr *pg_table2 = (MemoryAttr *)(pg_table2_paddr + PHYSICAL_MEMORY_OFFSET);
        uint64 vpn2 = get_vpn2(svaddr);
        uint64 pde2 = pg_table2[vpn2];
        if(pde2 == 0) {
            uint64 pde2_ppn = alloc();
            pg_table2[vpn2] = (pde2_ppn << 10) | 0x1;
        }
        uint64 pg_table3_paddr = (pg_table2[vpn2] & PDE_ADDRESS_MASK) << 2;
        MemoryAttr *pg_table3 = (MemoryAttr *)(pg_table3_paddr + PHYSICAL_MEMORY_OFFSET);
        uint64 vpn3 = get_vpn3(svaddr);
        pg_table3[vpn3] = (((svaddr - PHYSICAL_MEMORY_OFFSET) >> 12) << 10) | attr;
        svaddr += (1 << 12);
    }
}

void
activate_page_table(PageTablePaddr pt)
{
    w_satp(((uint64)pt >> 12) | (8L << 60));
    flush_tlb();
}

void
remap_kernel()
{
    ptp = (PageTablePaddr)(alloc() << 12);

    MemoryAttr ma = new_memory_attr();
    set_attr_readonly(&ma); set_attr_execute(&ma);
    set_pg_table(ptp, (uint64)stext, (uint64)etext, ma);
    printf("map from %p to %p\n", (uint64)stext, (uint64)etext);

    ma = new_memory_attr();
    set_attr_WR(&ma);
    set_pg_table(ptp, (uint64)etext, PHYSICAL_MEMORY_END + PHYSICAL_MEMORY_OFFSET, ma);
    printf("map from %p to %p\n", (uint64)etext, PHYSICAL_MEMORY_END + PHYSICAL_MEMORY_OFFSET);

    activate_page_table(ptp);
}