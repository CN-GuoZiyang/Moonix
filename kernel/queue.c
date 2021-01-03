/*
 *  kernel/queue.c
 *  
 *  (C) 2021  Ziyang Guo
 */

/*
 * queue.c 实现了一个队列的相关操作，队列中的元素是 usize
 * 队列中的元素以链表的形式组织，并用两个指针分别指向首和尾
 */

#include "types.h"
#include "def.h"
#include "queue.h"

void
pushBack(Queue *q, usize data)
{
    Node *n = kalloc(sizeof(Node));
    n->item = data;
    if(q->head == q->tail && q->head == 0) {
        q->head = n;
        q->tail = n;
    } else {
        q->tail->next = n;
        q->tail = n;
    }
}

usize
popFront(Queue *q)
{
    /* 
     * popFront 假设队列中存在元素
     * 在使用这个函数之前应当用 isEmpty 判断队列是否为空
     */
    Node *n = q->head;
    usize ret = n->item;
    if(q->head == q->tail) {
        q->head = 0;
        q->tail = 0;
    } else {
        q->head = q->head->next;
    }
    kfree(n);
    return ret;
}

int
isEmpty(Queue *q)
{
    if(q->head == q->tail && q->head == 0) {
        return 1;
    } else {
        return 0;
    }
}