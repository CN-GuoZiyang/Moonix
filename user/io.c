#include <stdarg.h>
#include "types.h"
#include "ulib.h"
#include "syscall.h"

static char digits[] = "0123456789abcdef";

uint8 getc()
{
    uint8 c;
    sys_read(0, &c, 1);
    return c;
}

void putchar(int c)
{
    sys_write(c);
}

static void
printint(int xx, int base, int sign)
{
    char buf[16];
    int i;
    uint x;

    if (sign && (sign = xx < 0))
        x = -xx;
    else
        x = xx;

    i = 0;
    do
    {
        buf[i++] = digits[x % base];
    } while ((x /= base) != 0);

    if (sign)
        buf[i++] = '-';

    while (--i >= 0)
        putchar(buf[i]);
}

static void
printptr(uint64 x)
{
    int i;
    putchar('0');
    putchar('x');
    for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
        putchar(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

void printf(char *fmt, ...)
{
    va_list ap;
    int i, c;
    char *s;

    if (fmt == 0)
        panic("null fmt");

    va_start(ap, fmt);
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++)
    {
        if (c != '%')
        {
            putchar(c);
            continue;
        }
        c = fmt[++i] & 0xff;
        if (c == 0)
            break;
        switch (c)
        {
        case 'd':
            printint(va_arg(ap, int), 10, 1);
            break;
        case 'x':
            printint(va_arg(ap, int), 16, 1);
            break;
        case 'p':
            printptr(va_arg(ap, uint64));
            break;
        case 's':
            if ((s = va_arg(ap, char *)) == 0)
                s = "(null)";
            for (; *s; s++)
                putchar(*s);
            break;
        case '%':
            putchar('%');
            break;
        default:
            putchar('%');
            putchar(c);
            break;
        }
    }
}

void panic(char *s)
{
    printf("panic: ");
    printf(s);
    printf("\n");
    sys_exit(1);
}