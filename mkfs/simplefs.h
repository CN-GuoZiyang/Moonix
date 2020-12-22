#ifndef _SIMPLEFS_H
#define _SIMPLEFS_H

#include "types.h"

#define BLOCK_SIZE  4096
#define MAGIC_NUM   0x4D534653U

typedef struct {
    uint32 magic;               // 魔数
    uint32 blocks;              // 总磁盘块数
    uint32 unused_blocks;       // 未使用的磁盘块数
    uint32 freemap_blocks;      // freemap 块数
    uint8 info[32];             // 其他信息
} SuperBlock;

#define TYPE_FILE   0
#define TYPE_DIR    1

typedef struct
{
    uint32 size;                // 文件大小，type 为文件夹时该字段为0
    uint32 type;                // 文件类型
    uint8 filename[32];         // 文件名称
    uint32 blocks;              // 占据磁盘块个数
    uint32 direct[12];          // 直接磁盘块
    uint32 indirect;            // 间接磁盘块
} Inode;

#endif