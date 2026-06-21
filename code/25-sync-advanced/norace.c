/* fix the race in race.c */

#include "csapp.h"

#define N 4

void *thread(void *vargp);

int main()
{
    pthread_t tid[N];
    int i, *ptr;

    // create threads
    for (i = 0; i < N; i++)
    {
        ptr = Malloc(sizeof(int)); // allocate memory, not shared
        *ptr = i;
        Pthread_create(&tid[i], NULL, thread, ptr);
    }

    // wait for threads to complete
    for (i = 0; i < N; i++)
        Pthread_join(tid[i], NULL);

    exit(0);
}

/* thread routine */
void *thread(void *vargp)
{
    int my_id = *((int *)vargp);
    Free(vargp); // free allocated memory

    printf("Hello from thread %d\n", my_id);
    return NULL;
}
