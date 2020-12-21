#include "types.h"
#include "ulib.h"
#include "syscall.h"

__attribute__((weak)) uint64
main()
{
    panic("No main linked!\n");
    return 1;
}

void
_start(uint8 _args, uint8 *_argv)
{
    extern void initHeap();
    initHeap();
    sys_exit(main());
}