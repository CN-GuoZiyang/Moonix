#include "types.h"
#include "defs.h"

void
main()
{
    init_interrupt();
    init_memory();
    init_timer();
    init_process();
}