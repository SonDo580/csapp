/* demonstrate a race condition */

#include "csapp.h"

#define N 4

void *thread(void *vargp);

int main()
{
    pthread_t tid[N];
    int i; // N threads share i

    // create threads
    for (i = 0; i < N; i++)
        Pthread_create(&tid[i], NULL, thread, &i);

    // wait for threads to complete
    for (i = 0; i < N; i++)
        Pthread_join(tid[i], NULL);

    exit(0);
}

/* thread routine */
void *thread(void *vargp)
{
    // race between update of i in main thread
    // and deref of vargp in peer thread
    int my_id = *((int *)vargp);

    printf("Hello from thread %d\n", my_id);
    return NULL;
}
