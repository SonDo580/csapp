#ifndef __SBUF_H__
#define __SBUF_H__

#include "csapp.h"

typedef struct
{
    int *buf;    // buffer array
    int n;       // max number of slots
    int front;   // buf[(front + 1) % n] is 1st item
    int rear;    // buf[rear % n] is last item
    sem_t mutex; // protect access to buf
    sem_t slots; // count available slots
    sem_t items; // count available items
} sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);

#endif
