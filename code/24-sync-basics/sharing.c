#include "csapp.h"

#define N 2

void *thread(void *vargp);

char **ptr; // global var: 1 instance [data]

int main()
{
    int i; // local var: 1 instance i.m [main thread's stack]
    pthread_t tid;

    char *msgs[N] = {
        "Hello from foo",
        "Hello from bar",
    }; // local var: 1 instance msgs.m [main thread's stack]

    ptr = msgs;
    for (i = 0; i < N; i++)
        Pthread_create(&tid, NULL, thread, (void *)(intptr_t)i);

    // terminate main thread but keep process alive
    // until last worker thread finishes
    Pthread_exit(NULL);
}

void *thread(void *vargp)
{
    // local var: 2 instances:
    // - my_id.p0 [peer thread 0's stack]
    // - my_id.p1 [peer thread 1's stack]
    int my_id = (int)(intptr_t)vargp;

    static int cnt = 0; // local static var: 1 instance [data]

    // peer threads reference main thread's stack
    // through global variable 'ptr'
    printf("[%d]: %s (cnt=%d)\n", my_id, ptr[my_id], ++cnt);

    return NULL;
}

/* Which variables are shared?

variable    referenced by   referenced by   referenced by
instance    main thread?    peer thread 0?  peer thread 1?
----------------------------------------------------------
ptr         yes             yes             yes
cnt         no              yes             yes
i.m         yes             no              no
msgs.m      yes             yes             yes
my_id.p0    no              yes             no
my_id.p1    no              no              yes

=> . ptr, cnt, msgs are shared.
   . i and my_id are not shared.
*/