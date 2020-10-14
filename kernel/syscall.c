#include "types.h"
#include "context.h"
#include "defs.h"

const uint64 SYS_WRITE = 64;
const uint64 SYS_EXIT = 93;

void
sys_exit(uint64 code)
{
    exit(code);
}

uint64
syscall(uint64 id, uint64 args[3], TrapFrame *tf)
{
    switch (id)
    {
    case SYS_WRITE:
        printf("%c", (char)args[0]);
        return 0;
    case SYS_EXIT:
        sys_exit(args[0]);
        return 0;
    default:
        panic("Unknown syscall id %d\n", id);
        return -1;
    }
}