#include "types.h"
#include "def.h"
#include "context.h"
#include "stdin.h"
#include "thread.h"
#include "fs.h"

const usize SYS_SHUTDOWN = 13;
const usize SYS_LSDIR    = 20;
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
sysExec(char *path)
{
    if(executeCPU(path, getCurrentTid())) {
        yieldCPU();
    }
    return 0;
}

usize
sysLsDir(char *path)
{
    Inode *node = lookup(0, path);
    if(node == 0) {
        printf("ls: No such file or directory\n");
        return 0;
    }
    ls(node);
    return 0;
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
        sysLsDir((char *)args[0]);
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
        sysExec((char *)args[0]);
        return 0;
    default:
        printf("Unknown syscall id %d\n", id);
        panic("");
        return -1;
    }
}