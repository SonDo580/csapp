#include "csapp.h"

#define N_ITERS 10000

int cnt;
sem_t mutex[2];

void *count(void *vargp);

int main()
{
    pthread_t tid[2];

    cnt = 0;
    Sem_init(&mutex[0], 0, 1); // mutex[0] = 1
    Sem_init(&mutex[1], 0, 1); // mutex[1] = 1

    Pthread_create(&tid[0], NULL, count, (void *)(intptr_t)0);
    Pthread_create(&tid[1], NULL, count, (void *)(intptr_t)1);

    Pthread_join(tid[0], NULL);
    Pthread_join(tid[1], NULL);

    printf("cnt=%d\n", cnt);
    exit(0);
}

/* thread routine */
void *count(void *vargp)
{
    int i;
    int id = (int)(intptr_t)vargp;

    for (i = 0; i < N_ITERS; i++)
    {
        P(&mutex[id]);
        P(&mutex[1 - id]);
        cnt++;
        V(&mutex[id]);
        V(&mutex[1 - id]);
    }

    return NULL;
}
