#include "sbuf.h"

/* Create an empty, bounded, shared FIFO buffer with n slots. */
void sbuf_init(sbuf_t *sp, int n)
{
    sp->n = n;
    sp->buf = Calloc(n, sizeof(int));
    sp->front = sp->rear = 0;   // empty buffer if front == rear
    Sem_init(&sp->mutex, 0, 1); // binary semaphore for locking
    Sem_init(&sp->slots, 0, n); // initially, buf has n empty slots
    Sem_init(&sp->slots, 0, 0); // initially, buf has 0 items
}

/* Clean up buffer sp. */
void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}

/* Insert item to the rear of shared buffer sp. */
void sbuf_insert(sbuf_t *sp, int item)
{
    P(&sp->slots); // wait for available slot
    P(&sp->mutex); // lock buffer
    sp->buf[(++sp->rear) % (sp->n)] = item;
    V(&sp->mutex); // unlock buffer
    V(&sp->items); // announce available item
}

/* remove and return 1st item from buffer sp. */
int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items); // wait for available item
    P(&sp->mutex); // lock buffer
    item = sp->buf[(++sp->front) % sp->n];
    V(&sp->mutex); // unlock buffer
    V(&sp->slots); // announce available slot
    return item;
}
