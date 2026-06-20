#include "csapp.h"

void *thread(void *vargp);

// global shared variable
volatile long cnt = 0; // read from and write to memory, don't store in registers
sem_t mutex;           // semaphore that protects 'cnt'

int main(int argc, char **argv)
{
    long niters;
    pthread_t tid1, tid2;

    if (argc != 2)
    {
        printf("usage: %s <niters>\n", argv[0]);
        exit(0);
    }
    niters = atoi(argv[1]);

    // Initialize the semaphore
    Sem_init(&mutex, 0, 1); // mutex = 1

    // Create threads and wait for them to finish
    Pthread_create(&tid1, NULL, thread, &niters);
    Pthread_create(&tid2, NULL, thread, &niters);
    Pthread_join(tid1, NULL);
    Pthread_join(tid2, NULL);

    // Check result
    if (cnt != 2 * niters)
        printf("BOOM! cnt=%ld\n", cnt);
    else
        printf("OK cnt=%ld\n", cnt);
    exit(0);
}

/* thread routine */
void *thread(void *vargp)
{
    long i, niters = *((long *)vargp);

    for (i = 0; i < niters; i++)
    {
        P(&mutex); // lock
        cnt++;
        V(&mutex); // release
    }

    return NULL;
}

/* Note: much slower than badcnt, but correct. */