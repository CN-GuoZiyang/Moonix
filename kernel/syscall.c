#include "types.h"
#include "def.h"
#include "context.h"

const usize SYS_WRITE = 64;
const usize SYS_EXIT = 93;

usize
syscall(usize id, usize args[3], InterruptContext *context)
{
    switch (id)
    {
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