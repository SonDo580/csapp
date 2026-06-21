/* Readers-Writers solution with weak reader priority:
- no reader should be kept waiting unless a writer
  has already been granted permission to use the object.
*/

#include "csapp.h"

int readcnt;    // initially = 0
sem_t mutex, w; // both initially = 1
// - mutex: protect readcnt.
// - w: lock for critical sections (shared resources accessed by both readers and writers).

void reader(void)
{
    while (1)
    {
        P(&mutex);
        readcnt++;
        if (readcnt == 1) // 1st in
            P(&w);
        V(&mutex);

        // Critical section
        // Reading happens

        P(&mutex);
        readcnt--;
        if (readcnt == 0) // last out
            V(&w);
        V(&mutex);
    }
}

void writer(void)
{
    while (1)
    {
        P(&w);

        // Critical section
        // Writing happens

        V(&w);
    }
}