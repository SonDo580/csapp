/*
Process-based concurrent echo server
*/

#include "echo.h"

void sigchld_handler(int sig)
{
    // -1 -> wait for any child process
    // WNOHANG -> immediately return 0 if no zombie children to reap
    while (waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    Signal(SIGCHLD, sigchld_handler);
    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        if (Fork() == 0) // child
        {
            Close(listenfd); // child closes its listening socket
            echo(connfd);    // child services client
            Close(connfd);   // child closes connection with client
            exit(0);         // child exits
        }
        Close(connfd); // parent closes connection socket
    }
}