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