#include "csapp.h"
#include "sbuf.h"

#define N_THREADS 4
#define SBUF_SIZE 16

void echo_cnt(int connfd);
void *thread(void *vargp);

sbuf_t sbuf; // shared buffer of connected descriptors

int main(int argc, char **argv)
{
    int i, listenfd, connfd;
    socklen_t client_len;
    struct sockaddr_storage client_addr;
    pthread_t tid;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);

    sbuf_init(&sbuf, SBUF_SIZE);

    // create worker threads
    for (i = 0; i < N_THREADS; i++)
        Pthread_create(&tid, NULL, thread, NULL);

    while (1)
    {
        client_len = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&client_addr, &client_len);
        sbuf_insert(&sbuf, connfd); // insert connfd into buffer
    }
}

void *thread(void *vargp)
{
    // - cannot be reaped or killed by other threads
    // - resources are automatically reaped on termination
    Pthread_detach(pthread_self());

    while (1)
    {
        int connfd = sbuf_remove(&sbuf); // remove connfd from buffer
        echo_cnt(connfd);                // service client
        Close(connfd);
    }
}