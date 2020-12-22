#include "types.h"
#include "def.h"
#include "fs.h"

Inode *ROOT_INODE;
char *FREEMAP;

extern void _fs_img_start();

usize
getBlockAddr(int blockNum) {
    void *addr = (void *)_fs_img_start;
    addr += (blockNum * BLOCK_SIZE);
    return (usize)addr;
}

void
initFs()
{
    FREEMAP = (char *)getBlockAddr(1);
    SuperBlock* spBlock = (SuperBlock *)_fs_img_start;
    ROOT_INODE = (Inode *)getBlockAddr(spBlock->freemap_blocks + 1);
}

Inode *
lookup(Inode *node, char *filename)
{
    if(filename[0] == '/') {
        node = ROOT_INODE;
        filename ++;
    }
    if(*filename == '\0') return node;
    if(node->type != TYPE_DIR) return 0;
    char cTarget[strlen(filename) + 1];
    int i = 0;
    while (*filename != '/' && *filename != '\0') {
        cTarget[i] = *filename;
        filename ++;
        i ++;
    }
    cTarget[i] = '\0';
    if(*filename == '/') filename ++;
    if(!strcmp(".", cTarget)) {
        return lookup(node, filename);
    }
    if(!strcmp("..", cTarget)) {
        Inode *upLevel = (Inode *)getBlockAddr(node->direct[1]);
        return lookup(upLevel, filename);
    }
    int blockNum = node->blocks;
    if(blockNum <= 12) {
        for(i = 2; i < blockNum; i ++) {
            Inode *candidate = (Inode *)getBlockAddr(node->direct[i]);
            if(!strcmp((char *)candidate->filename, cTarget)) {
                return lookup(candidate, filename);
            }
        }
        return 0;
    } else {
        for(i = 2; i < 12; i ++) {
            Inode *candidate = (Inode *)getBlockAddr(node->direct[i]);
            if(!strcmp((char *)candidate->filename, cTarget)) {
                return lookup(candidate, filename);
            }
        }
        uint32 *indirect = (uint32 *)getBlockAddr(node->indirect);
        for(i = 12; i < blockNum; i ++) {
            Inode *candidate = (Inode *)getBlockAddr(indirect[i-12]);
            if(!strcmp((char *)candidate->filename, cTarget)) {
                return lookup(candidate, filename);
            }
        }
        return 0;
    }
}

void
copyByteToBuf(char *src, char *dst, int length)
{
    int i;
    for(i = 0; i < length; i ++) {
        dst[i] = src[i];
    }
}

void
readall(Inode *node, char *buf) {
    if(node->type != TYPE_FILE) {
        panic("Cannot read a directory!\n");
    }
    int l = node->size, b = node->blocks;
    if(b <= 12) {
       int i;
       for(i = 0; i < b; i ++) {
           char *src = (char *)getBlockAddr(node->direct[i]);
           int copySize = l >= 4096 ? 4096 : l;
           copyByteToBuf(src, buf, copySize);
           buf += copySize;
           l -= copySize;
       }
    } else {
        int i;
        for(i = 0; i < 12; i ++) {
            char *src = (char *)getBlockAddr(node->direct[i]);
            int copySize = l >= 4096 ? 4096 : l;
            copyByteToBuf(src, buf, copySize);
            buf += copySize;
            l -= copySize;
        }
        uint32 *indirect = (uint32 *)getBlockAddr(node->indirect);
        for(i = 0; i < b-12; i ++) {
            char *src = (char *)getBlockAddr(indirect[i]);
            int copySize = l >= 4096 ? 4096 : l;
            copyByteToBuf(src, buf, copySize);
            buf += copySize;
            l -= copySize;
        }
    }
}