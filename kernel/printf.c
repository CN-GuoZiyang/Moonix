#include <stdarg.h>
#include "types.h"
#include "def.h"

static char digits[] = "0123456789abcdef";

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
        console_putchar(buf[i]);
}

static void
printptr(usize x)
{
    int i;
    console_putchar('0');
    console_putchar('x');
    for (i = 0; i < (sizeof(usize) * 2); i++, x <<= 4)
        console_putchar(digits[x >> (sizeof(usize) * 8 - 4)]);
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
            console_putchar(c);
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
            printptr(va_arg(ap, usize));
            break;
        case 's':
            if ((s = va_arg(ap, char *)) == 0)
                s = "(null)";
            for (; *s; s++)
                console_putchar(*s);
            break;
        case '%':
            console_putchar('%');
            break;
        default:
            console_putchar('%');
            console_putchar(c);
            break;
        }
    }
}

void panic(char *s)
{
    printf("panic: ");
    printf(s);
    printf("\n");
    shutdown();
}