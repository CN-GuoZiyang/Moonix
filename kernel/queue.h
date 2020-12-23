#ifndef _QUEUE_H
#define _QUEUE_H

#include "types.h"

typedef struct n {
    usize item;
    struct n *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} Queue;

void pushBack(Queue *q, usize data);
usize popFront(Queue *q);
int isEmpty(Queue *q);

#endif