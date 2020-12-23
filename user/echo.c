#include "types.h"
#include "ulib.h"

#define LF 0x0au
#define CR 0x0du
#define BS 0x08u
#define DL 0x7fu

uint64
main()
{
    printf("Welcome to echo!\n");
    int lineCount = 0;
    while(1) {
        uint8 c = getc();
        switch(c) {
            case LF: case CR: 
                lineCount = 0;
                putchar(LF);
                putchar(CR);
                break;
            case DL:
                if(lineCount > 0) {
                    putchar(BS);
                    putchar(' ');
                    putchar(BS);
                    lineCount -= 1;
                }
                break;
            default:
                lineCount += 1;
                putchar(c);
                break;
        }
    }
}