/*
 *  mkfs/mksfs.c
 *  
 *  (C) 2021  Ziyang Guo
 */

/*
 * mksfs.c 用于将一个文件夹作为根目录打包成一个 SimpleFS 镜像文件
 * 
 * SimpleFS 镜像组成如下：
 * +-------+------+     +------+------+------+     +------+
 * | Super | Free | ... | Free | Root |Other | ... |Other |
 * | Block | Map  |     | Map  |Inode |Inode |     |Inode |
 * +-------+------+     +------+------+------+     +------+
 */

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "simplefs.h"

#define BLOCK_NUM       256         /* 总块数 256 块，大小为 1M */
#define FREEMAP_NUM     1           /* Freemap 块个数 */

char Image[BLOCK_SIZE * BLOCK_NUM]; /* 最终的镜像数据 */

char freemap[BLOCK_NUM];            /* 临时的 freemap，此时一个 char 代表一块，写入时一个 bit 一块 */
uint32 freenum = BLOCK_NUM;

char *rootdir = "rootfs";           /* 作为根目录的文件夹路径 */

void walk(char *dirName, Inode *nowInode, uint32 nowInodeNum);
uint64 getBlockAddr(int blockNum);
int getFreeBlock();
void copyInodeToBlock(int blockNum, Inode *in);

void
main()
{
    /* 
     * 最开始的几块分别是 超级块，freemap 块 和 root 文件夹所在的 inode
     * 都标记为占用
     */
    freemap[0] = 1;
    int i;
    for(i = 0; i < FREEMAP_NUM; i ++) freemap[1+i] = 1;
    freemap[FREEMAP_NUM + 1] = 1;
    freenum -= (FREEMAP_NUM + 2);
    
    /* 填充 superblock 信息 */
    SuperBlock spBlock;
    spBlock.magic = MAGIC_NUM;
    spBlock.blocks = BLOCK_NUM;
    spBlock.freemapBlocks = FREEMAP_NUM;
    char *info = "SimpleFS By Ziyang";
    for(i = 0; i < strlen(info); i ++) {
        spBlock.info[i] = info[i];
    }
    spBlock.info[i] = '\0';
    
    /* 设置根 inode */
    Inode rootInode;
    rootInode.size = 0;
    rootInode.type = TYPE_DIR;
    rootInode.filename[0] = '/'; rootInode.filename[1] = '\0';
    
    /* 递归遍历根文件夹，并设置和填充数据 */
    walk(rootdir, &rootInode, FREEMAP_NUM+1);

    spBlock.unusedBlocks = freenum;

    /* 将超级块写入 Image */
    char *ptr = (char *)getBlockAddr(0);
    *(SuperBlock *)ptr = spBlock;

    /* 将 freemap 写入 Image */
    ptr = (char *)getBlockAddr(1);
    for(i = 0; i < BLOCK_NUM/8; i ++) {
        char c = 0;
        int j;
        for(j = 0; j < 8; j ++) {
            if(freemap[i*8+j]) {
                c |= (1 << j);
            }
        }
        *ptr = c;
        ptr ++;
    }

    /* 将 rootInode 写入 Image */
    copyInodeToBlock(FREEMAP_NUM+1, &rootInode);

    /* 将 Image 写入磁盘 */
    FILE *img = fopen("fs.img", "w+b");
    fwrite(Image, sizeof(Image), 1, img);
    fflush(img); fclose(img);
}

/* 根据块号获取 Image 中的块的起始地址 */
uint64
getBlockAddr(int blockNum) {
    void *addr = (void *)Image;
    addr += (blockNum * BLOCK_SIZE);
    return (uint64)addr;
}

/*
 * 遍历目标文件夹，并填充 Inode
 * nowInode 为当前文件夹的 Inode
 * nowInodeNum 为其 Inode 号
 */
void
walk(char *dirName, Inode *nowInode, uint32 nowInodeNum)
{
    /* 打开当前文件夹 */
    DIR *dp = opendir(dirName);
    struct dirent *dirp;

    /* 文件夹下第一个文件为其自己 */
    nowInode->direct[0] = nowInodeNum;
    if(!strcmp(dirName, rootdir)) {
        /* 若在根目录，则无上一级，上一级文件夹也为其自己 */
        nowInode->direct[1] = nowInodeNum;
    }
    /* 下一个文件的序号 */
    int emptyIndex = 2;

    /* 遍历当前文件夹下所有文件 */
    while((dirp = readdir(dp))) {
        /* 跳过 . 和 .. */
        if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, "..")) {
            continue;
        }
        int blockNum;
        if(dirp->d_type == DT_DIR) {
            /* 文件夹处理，递归遍历 */
            Inode dinode;
            dinode.size = 0;
            dinode.type = TYPE_DIR;
            int i;
            for(i = 0; i < strlen(dirp->d_name); i ++) {
                dinode.filename[i] = dirp->d_name[i];
            }
            dinode.filename[i] = '\0';
            blockNum = getFreeBlock();
            /* 文件夹的前两个文件分别为 . 和 .. */
            dinode.direct[0] = blockNum;
            dinode.direct[1] = nowInodeNum;
            char *tmp = (char *)malloc(strlen(dirName) + strlen(dirp->d_name) + 1);
            sprintf(tmp, "%s/%s", dirName, dirp->d_name);
            walk(tmp, &dinode, blockNum);

            copyInodeToBlock(blockNum, &dinode);
        } else if(dirp->d_type == DT_REG) {
            /* 普通文件处理 */
            Inode finode;
            finode.type = TYPE_FILE;
            int i;
            for(i = 0; i < strlen(dirp->d_name); i ++) {
                finode.filename[i] = dirp->d_name[i];
            }
            finode.filename[i] = '\0';
            char *tmp = (char *)malloc(strlen(dirName) + strlen(dirp->d_name) + 1);
            sprintf(tmp, "%s/%s", dirName, dirp->d_name);
            /* 获取文件信息 */
            struct stat buf;
            stat(tmp, &buf);
            finode.size = buf.st_size;
            finode.blocks = (finode.size - 1) / BLOCK_SIZE + 1;
            
            blockNum = getFreeBlock();

            /* 将文件数据复制到对应的块 */
            uint32 l = finode.size;     /* 剩余未拷贝的大小 */
            int blockIndex = 0;
            FILE *fp = fopen(tmp, "rb");
            while(l) {
                int ffb = getFreeBlock();
                char *buffer = (char *)getBlockAddr(ffb);
                size_t size;
                if(l > BLOCK_SIZE) size = BLOCK_SIZE;
                else size = l;
                fread(buffer, size, 1, fp);
                l -= size;
                if(blockIndex < 12) {
                    finode.direct[blockIndex] = ffb;
                } else {
                    if(finode.indirect == 0) {
                        finode.indirect = getFreeBlock();
                    }
                    uint32 *inaddr = (uint32 *)getBlockAddr(finode.indirect);
                    inaddr[blockIndex - 12] = ffb;
                }
                blockIndex ++;
            }
            fclose(fp);
            copyInodeToBlock(blockNum, &finode);
        } else {
            continue;
        }

        if(emptyIndex < 12) {
            nowInode->direct[emptyIndex] = blockNum;
        } else {
            if(nowInode->indirect == 0) {
                nowInode->indirect = getFreeBlock();
            }
            uint32 *inaddr = (uint32 *)getBlockAddr(nowInode->indirect);
            inaddr[emptyIndex - 12] = blockNum;
        }
        emptyIndex ++;
    }
    closedir(dp);
    nowInode->blocks = emptyIndex;
}

int getFreeBlock() {
    int i;
    for(i = 0; i < BLOCK_NUM; i ++) {
        if(!freemap[i]) {
            freemap[i] = 1;
            freenum --;
            return i;
        }
    }
    printf("get free block failed!\n");
    exit(1);
}

void copyInodeToBlock(int blockNum, Inode *in) {
    Inode *dst = (Inode *)getBlockAddr(blockNum);
    *dst = *in;
}