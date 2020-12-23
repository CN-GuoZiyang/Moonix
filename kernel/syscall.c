#include "types.h"
#include "def.h"
#include "context.h"
#include "stdin.h"

const usize SYS_READ     = 63;
const usize SYS_WRITE    = 64;
const usize SYS_EXIT     = 93;

usize
sysRead(usize fd, uint8 *base, usize len) {
    *base = (uint8)popChar();
    return 1;
}

usize
syscall(usize id, usize args[3], InterruptContext *context)
{
    switch (id)
    {
    case SYS_READ:
        return sysRead(args[0], (uint8 *)args[1], args[2]);
    case SYS_WRITE:
        consolePutchar(args[0]);
        return 0;
    case SYS_EXIT:
        exitFromCPU(args[0]);
        return 0;
    default:
        printf("Unknown syscall id %d\n", id);
        panic("");
        return -1;
    }
}