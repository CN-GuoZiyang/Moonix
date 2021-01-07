#ifndef _FILE_H
#define _FILE_H

#include "types.h"
#include "fs.h"

#define FD_NONE     0
#define FD_INODE    1

typedef struct
{
    uint8 fdType;
    Inode *inode;
    usize offset;
} File;

#endif