#ifndef _MAPPING_H
#define _MAPPING_H

#include "types.h"
#include "consts.h"

typedef usize PageTableEntry;   /* 页表项长度 64 位 */

/*
 * 页表由页表项组成 
 * 一页页表中包含 4096 / 8 个页表项
 */
typedef struct
{
    PageTableEntry entries[PAGE_SIZE >> 3];
} PageTable;

/* 页表项的 8 个标志位 */
#define VALID       1 << 0
#define READABLE    1 << 1
#define WRITABLE    1 << 2
#define EXECUTABLE  1 << 3
#define USER        1 << 4
#define GLOBAL      1 << 5
#define ACCESSED    1 << 6
#define DIRTY       1 << 7

// 映射片段，描述映射到虚拟内存的一个段
typedef struct
{
    /* 映射虚拟地址范围 */
    usize startVaddr;
    usize endVaddr;
    /* 该段映射的权限 */
    usize flags;
} Segment;

/* 一个虚拟地址空间，可能映射了多个段 */
typedef struct
{
    usize rootPpn;      /* 根页表的物理页号 */
} Mapping;

usize accessVaViaPa(usize pa);

Mapping newKernelMapping();
void mapLinearSegment(Mapping self, Segment segment);
void mapFramedSegment(Mapping m, Segment segment);
void mapFramedAndCopy(Mapping m, Segment segment, char *data, usize length);

#endif