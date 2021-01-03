/*
 *  kernel/stdin.c
 *  
 *  (C) 2021  Ziyang Guo
 */

#include "types.h"
#include "def.h"
#include "queue.h"
#include "condition.h"

/* 
 * 全局唯一标准输入缓冲区
 * buf 为输入字符缓冲
 * pushed 为条件变量（等待输入的线程）
 */
struct
{
    Queue buf;
    Condvar pushed;
} STDIN;

/*
 * 将一个字符放入标准输入缓冲区
 * 并唤醒一个等待字符的线程
 */
void
pushChar(char ch)
{
    pushBack(&STDIN.buf, (usize)ch);
    notifyCondition(&STDIN.pushed);
}

/*
 * 线程请求从 stdin 中获取一个输入的字符
 * 如果当前缓冲区为空，线程会进入等待队列，并挂起自己
 * 在之后的某个时刻被唤醒时，缓冲区必然有字符，就可以顺利返回
 */
char
popChar()
{
    /* 实际上第二次循环就能顺利返回 */
    while(1) {
        if(!isEmpty(&STDIN.buf)) {
            char ret = (char)popFront(&STDIN.buf);
            return ret;
        } else {
            waitCondition(&STDIN.pushed);
        }
    }
}