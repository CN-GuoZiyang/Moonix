/*
 *  kernel/fs.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "fs.h"

Inode *ROOT_INODE;      /* 根目录的 Inode */
char *FREEMAP;          /* 指向空闲 map 的指针 */

/* 
 * 目前文件系统被装载到 .data 段中
 * _fs_img_start 为文件系统部分内存的起始符号
 */
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
    ROOT_INODE = (Inode *)getBlockAddr(spBlock->freemapBlocks + 1);
}

/*
 * 在当前 Inode 所代表的文件夹中查找目标路径的 Inode
 * 当 filename 以 / 开头时直接从根目录开始查找
 */
Inode *
lookup(Inode *node, char *filename)
{
    if(filename[0] == '/') {
        node = ROOT_INODE;
        filename ++;
    }
    if(node == 0) node = ROOT_INODE;
    if(*filename == '\0') return node;
    if(node->type != TYPE_DIR) return 0;
    /*
     * filename 可能包含多层目录
     * 从 filename 中分割出来当前文件夹下需要寻找的文件名
     */
    char cTarget[strlen(filename) + 1];
    int i = 0;
    while (*filename != '/' && *filename != '\0') {
        cTarget[i] = *filename;
        filename ++;
        i ++;
    }
    cTarget[i] = '\0';
    
    if(*filename == '/') filename ++;
    int blockNum = node->blocks;
    /* 在当前文件夹下查找，寻找到 Inode 后递归搜索下一级 */
    if(blockNum <= 12) {
        for(i = 0; i < blockNum; i ++) {
            InodeItem candidateItem = node->direct[i];
            if(!strcmp((char *)candidateItem.filename, cTarget)) {
                Inode *candidate = (Inode *)getBlockAddr(node->direct[i].block);
                return lookup(candidate, filename);
            }
        }
        return 0;
    } else {
        for(i = 0; i < 12; i ++) {
            InodeItem candidateItem = node->direct[i];
            if(!strcmp((char *)candidateItem.filename, cTarget)) {
                Inode *candidate = (Inode *)getBlockAddr(node->direct[i].block);
                return lookup(candidate, filename);
            }
        }
        InodeItem *indirect = (InodeItem *)getBlockAddr(node->indirect);
        for(i = 12; i < blockNum; i ++) {
            InodeItem candidateItem = indirect[i];
            if(!strcmp((char *)candidateItem.filename, cTarget)) {
                Inode *candidate = (Inode *)getBlockAddr(node->direct[i].block);
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

/* 读取一个表示文件的 Inode 的所有字节到 buf 中 */
void
readall(Inode *node, char *buf) {
    if(node->type != TYPE_FILE) {
        panic("Cannot read a directory!\n");
    }
    int l = node->size, b = node->blocks;
    if(b <= 12) {
       int i;
       for(i = 0; i < b; i ++) {
           char *src = (char *)getBlockAddr(node->direct[i].block);
           int copySize = l >= 4096 ? 4096 : l;
           copyByteToBuf(src, buf, copySize);
           buf += copySize;
           l -= copySize;
       }
    } else {
        int i;
        for(i = 0; i < 12; i ++) {
            char *src = (char *)getBlockAddr(node->direct[i].block);
            int copySize = l >= 4096 ? 4096 : l;
            copyByteToBuf(src, buf, copySize);
            buf += copySize;
            l -= copySize;
        }
        InodeItem *indirect = (InodeItem *)getBlockAddr(node->indirect);
        for(i = 0; i < b-12; i ++) {
            char *src = (char *)getBlockAddr(indirect[i].block);
            int copySize = l >= 4096 ? 4096 : l;
            copyByteToBuf(src, buf, copySize);
            buf += copySize;
            l -= copySize;
        }
    }
}

/* 输出某个文件夹下的所有文件名 */
void
ls(Inode *node)
{
    if(node->type == TYPE_FILE) {
        printf("%s: is not a directory\n", node->filename);
        return;
    }
    if(node->blocks <= 12) {
        int i;
        for(i = 0; i < node->blocks; i ++) {
            printf("%s ", node->direct[i].filename);
        }
    } else {
        int i;
        for(i = 0; i < 12; i ++) {
            printf("%s ", node->direct[i].filename);
        }
        InodeItem *indirect = (InodeItem *)getBlockAddr(node->indirect);
        for(i = 0; i < node->blocks-12; i ++) {
            printf("%s ", indirect[i].filename);
        }
    }
    printf("\n");
}

char
*getInodePath(Inode *inode, char buf[256])
{
    char *ptr = buf;
    ptr += 256;
    *ptr = 0;
    ptr --;
    if(!strcmp((char *)inode->filename, "/")) {
        *ptr = '/';
        return ptr;
    }
    while(strcmp((char *)inode->filename, "/")) {
        int len = strlen((char *)inode->filename);
        int i;
        for(i = len-1; i >= 0; i --) {
            *ptr = inode->filename[i];
            ptr --;
        }
        *ptr = '/';
        ptr --;
        inode = lookup(inode, "..");
    }
    return ++ptr;
}