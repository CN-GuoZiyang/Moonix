#include "types.h"
#include "def.h"
#include "sbi.h"

void
console_putchar(usize c)
{
    SBI_ECALL(SBI_CONSOLE_PUTCHAR, c, 0, 0);
}

usize
console_getchar()
{
    return SBI_ECALL(SBI_CONSOLE_GETCHAR, 0, 0, 0);
}

void
shutdown()
{
    SBI_ECALL(SBI_SHUTDOWN, 0, 0, 0);
    while(1) {}
}