#include "types.h"
#include "sbi.h"

void
main()
{
    extern void initInterrupt();    initInterrupt();
    extern void setNextTimer();     setNextTimer();
    extern void initThread();       initThread();
}