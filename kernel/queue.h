#ifndef _QUEUE_H
#define _QUEUE_H

#include "types.h"

/* 队列中的元素 */
typedef struct n {
    usize item;
    struct n *next;
} Node;

/* 一个队列，两个指针指向首尾元素 */
typedef struct {
    Node *head;
    Node *tail;
} Queue;

void pushBack(Queue *q, usize data);
usize popFront(Queue *q);
int isEmpty(Queue *q);

#endif