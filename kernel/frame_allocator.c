#include "types.h"
#include "defs.h"

typedef struct segmenttreeallocator
{
    uint8 a[0x10000];
    uint64 m, n, offset;
} Sta;

static Sta sta;

void
init_sta(uint64 l, uint64 r)
{
    sta.offset = l - 1;
    sta.n = r - l;
    sta.m = 1;
    while(sta.m < sta.n + 2) {
        sta.m = sta.m << 1;
    }
    int i;
    for(i = 1; i < sta.m<<1; i ++) {
        sta.a[i] = 1;
    }
    for(i = 1; i < sta.n; i ++) {
        sta.a[sta.m + i] = 0;
    }
    for(i = sta.m-1; i >= 1; i --) {
        sta.a[i] = sta.a[i << 1] & sta.a[(i << 1) | 1];
    }
}

uint64
alloc()
{
    if(sta.a[1] == 1) {
        panic("physical memory depleted!");
    }
    uint64 p = 1;
    while(p < sta.m) {
        if(sta.a[p << 1] == 0) {
            p = p << 1;
        } else {
            p = (p << 1) | 1;
        }
    }
    uint64 result = p + sta.offset - sta.m;
    sta.a[p] = 1;
    p >>= 1;
    while(p > 0) {
        sta.a[p] = sta.a[p << 1] & sta.a[(p << 1) | 1];
        p >>= 1;
    }
    return result;
}

void
dealloc(uint64 n)
{
    uint64 p = n + sta.m - sta.offset;
    if(sta.a[p] != 1) {
        printf("No need to dealloc!\n");
        return;
    }
    sta.a[p] = 0;
    p >>= 1;
    while(p > 0) {
        sta.a[p] = sta.a[p << 1] & sta.a[(p << 1) | 1];
        p >>= 1;
    }
}