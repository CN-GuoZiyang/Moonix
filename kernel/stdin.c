#include "types.h"
#include "def.h"
#include "queue.h"
#include "condition.h"

// 标准输入缓冲区，buf 为输入字符缓冲，pushed 为条件变量（等待输入的线程）
struct
{
    Queue buf;
    Condvar pushed;
} STDIN;

void
pushChar(char ch)
{
    pushBack(&STDIN.buf, (usize)ch);
    notifyCondition(&STDIN.pushed);
}

char
popChar()
{
    while(1) {
        if(!isEmpty(&STDIN.buf)) {
            char ret = (char)popFront(&STDIN.buf);
            return ret;
        } else {
            waitCondition(&STDIN.pushed);
        }
    }
}