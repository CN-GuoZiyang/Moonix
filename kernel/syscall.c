/*
 *  kernel/syscall.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "context.h"
#include "stdin.h"
#include "thread.h"
#include "fs.h"

const usize SYS_SHUTDOWN = 13;
const usize SYS_LSDIR    = 20;
const usize SYS_CDDIR    = 21;
const usize SYS_PWD      = 22;
const usize SYS_OPEN     = 56;
const usize SYS_CLOSE    = 57;
const usize SYS_READ     = 63;
const usize SYS_WRITE    = 64;
const usize SYS_EXIT     = 93;
const usize SYS_EXEC     = 221;

usize
sysRead(usize fd, uint8 *base, usize len) {
    *base = (uint8)popChar();
    return 1;
}

usize
sysExec(char *path, int fd)
{
    Inode *current = getCurrentThread()->process.oFile[fd].inode;
    Inode *inode = lookup(current, path);
    if(inode == 0) {
        printf("Command not found!\n");
        return 0;
    }
    if(executeCPU(inode, getCurrentTid())) {
        yieldCPU();
    }
    return 0;
}

usize
sysLsDir(char *path, int fd)
{
    Inode *current = getCurrentThread()->process.oFile[fd].inode;
    Inode *inode;
    if(*path == 0) {
        inode = current;
    } else {
        inode = lookup(current, path);
    }
    if(inode == 0) {
        printf("ls: No such file or directory\n");
        return 0;
    }
    ls(inode);
    return 0;
}

usize
sysCdDir(char *path, int fd)
{
    Inode *current = getCurrentThread()->process.oFile[fd].inode;
    Inode *inode = lookup(current, path);
    if(inode == 0) {
        printf("cd: No such file or directory\n");
        return 0;
    }
    if(inode->type != TYPE_DIR) {
        printf("%s: is not a directory!\n", inode->filename);
        return 0;
    }
    getCurrentThread()->process.oFile[fd].inode = inode;
    return 0;
}

int
sysOpen(char *path)
{
    Thread *thread = getCurrentThread();
    int fd = allocFd(thread);
    if(fd == -1) {
        panic("Max file open!\n");
    }
    thread->process.fdOccupied[fd] = 1;
    File file;
    file.fdType = FD_INODE;
    file.offset = 0;
    file.inode = lookup(0, path);
    thread->process.oFile[fd] = file;
    return fd;
}

void
sysPwd(int fd)
{
    Inode *current = getCurrentThread()->process.oFile[fd].inode;
    char buf[256];
    char *path = getInodePath(current, buf);
    printf("%s\n", path);
}

void
sysClose(int fd)
{
    Thread *thread = getCurrentThread();
    deallocFd(thread, fd);
}

usize
syscall(usize id, usize args[3], InterruptContext *context)
{
    switch (id)
    {
    case SYS_SHUTDOWN:
        shutdown();
        return 0;
    case SYS_LSDIR:
        sysLsDir((char *)args[0], args[1]);
        return 0;
    case SYS_CDDIR:
        sysCdDir((char *)args[0], args[1]);
        return 0;
    case SYS_OPEN:
        return sysOpen((char *)args[0]);
    case SYS_PWD:
        sysPwd(args[0]);
        return 0;
    case SYS_CLOSE:
        sysClose(args[0]);
        return 0;
    case SYS_READ:
        return sysRead(args[0], (uint8 *)args[1], args[2]);
    case SYS_WRITE:
        consolePutchar(args[0]);
        return 0;
    case SYS_EXIT:
        exitFromCPU(args[0]);
        return 0;
    case SYS_EXEC:
        sysExec((char *)args[0], args[1]);
        return 0;
    default:
        printf("Unknown syscall id %d\n", id);
        panic("");
        return -1;
    }
}