#ifndef _SIMPLEFS_H
#define _SIMPLEFS_H

#include "types.h"

#define BLOCK_SIZE  4096
#define MAGIC_NUM   0x4D534653U /* 'M' 'S' 'F' 'S' */

/* 超级块结构 */
typedef struct {
    uint32 magic;               /* 魔数 */
    uint32 blocks;              /* 文件系统的总磁盘块数 */
    uint32 unusedBlocks;        /* 空闲的磁盘块数 */
    uint32 freemapBlocks;       /* freemap 块数 */
    uint8 info[32];             /* 其他附加信息 */
} SuperBlock;

#define TYPE_FILE   0           /* 文件类型：普通文件 */
#define TYPE_DIR    1           /* 文件类型：文件夹 */

/* Inode 块结构 */
typedef struct
{
    uint32 size;                /* 文件大小，type 为 TYPE_DIR 时该字段为 0 */
    uint32 type;                /* 文件类型 */
    uint8 filename[32];         /* 文件名称 */
    uint32 blocks;              /* 占据磁盘块个数 */
    uint32 direct[12];          /* 直接磁盘块 */
    uint32 indirect;            /* 间接磁盘块 */
} Inode;

#endif