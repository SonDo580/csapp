/*
Concurrent echo server based on I/O multiplexing (event-based)
*/

#include "csapp.h"

/* Represent a pool of connected descriptor */
typedef struct
{
    int maxfd;                   // largest descriptor in read_set
    fd_set read_set;             // (bit mask) set of all active descriptors
    fd_set ready_set;            // (bit mask) subset of descriptors ready for reading
    int nready;                  // number of ready descriptors from 'select'
    int maxi;                    // high water index into client array
    int clientfd[FD_SETSIZE];    // set of active client connected descriptors
    rio_t clientrio[FD_SETSIZE]; // set of active read buffers for clients
} pool;

int byte_cnt = 0; // count total bytes received by server

void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients(pool *p);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    static pool pool;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);

    while (1)
    {
        // Wait for listening/connected descriptor(s) to become ready
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);

        // If listening descriptor is ready, add new client to the pool
        if (FD_ISSET(listenfd, &pool.ready_set))
        {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            add_client(connfd, &pool);
        }

        // Handle each ready connected descriptor
        check_clients(&pool);
    }
}

/* initialize the pool of active clients */
void init_pool(int listenfd, pool *p)
{
    // Initially, there're no connected descriptors
    int i;
    p->maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        p->clientfd[i] = -1;

    // Initially, listenfd is the only member of select's read set
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

/* add a new client connection to the pool */
void add_client(int connfd, pool *p)
{
    int i;
    p->nready--;

    for (i = 0; i < FD_SETSIZE; i++)
        if (p->clientfd[i] < 0) // found an available slot
        {
            // add connected descriptor to the pool
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->clientrio[i], connfd);

            // add the descriptor to descriptor set
            FD_SET(connfd, &p->read_set);

            // update max descriptor and pool high water mark
            if (connfd > p->maxfd)
                p->maxfd = connfd;
            if (i > p->maxi)
                p->maxi = i;

            break;
        }

    if (i == FD_SETSIZE) // couldn't find an empty slot
        app_error("add_client error: too many clients");
}

/* service ready client connections */
void check_clients(pool *p)
{
    int i, connfd, n;
    char buf[MAXLINE];
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
    {
        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        // If a descriptor is ready, echo a text line from it
        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set)))
        {
            p->nready--;
            if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
            {
                byte_cnt += n;
                printf("Server received %d (%d total) bytes on fd %d\n",
                       n, byte_cnt, connfd);
                Rio_writen(connfd, buf, n);
            }

            else
            { // EOF detected -> remove descriptor from pool
                Close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}