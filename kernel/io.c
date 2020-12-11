#include "types.h"
#include "def.h"
#include "sbi.h"

void
console_putchar(usize c)
{
    SBI_ECALL_1(SBI_CONSOLE_PUTCHAR, c);
}

usize
console_getchar()
{
    return SBI_ECALL_0(SBI_CONSOLE_GETCHAR);
}

void
shutdown()
{
    SBI_ECALL_0(SBI_SHUTDOWN);
    while(1) {}
}