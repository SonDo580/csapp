#include "csapp.h"

#define N 5

// sending signals with kill() function
void fork12()
{
    pid_t pid[N];
    int i;
    int child_status;

    for (i = 0; i < N; i++)
        if ((pid[i] = fork()) == 0) // child
            while (1)               // infinite loop
                ;

    for (i = 0; i < N; i++)
    {
        printf("killing process %d\n", pid[i]);
        kill(pid[i], SIGINT);
    }

    for (i = 0; i < N; i++)
    {
        pid_t wpid = wait(&child_status);
        if (WIFEXITED(child_status))
            printf("child %d terminated with exit status %d\n",
                   wpid, WEXITSTATUS(child_status));
        else
            printf("child %d terminated abnormally\n", wpid);
    }
}

int ccount = 0;

/* signal handler that reaps 1 terminated child */
void child_handler(int sig)
{
    int old_errno = errno;

    pid_t pid;
    if ((pid = wait(NULL)) < 0)
        Sio_error("wait error");

    ccount--;
    Sio_puts("Handler reaped child ");
    Sio_putl((long)pid);
    Sio_puts(" \n");
    sleep(1);

    errno = old_errno; // restore errno
}

// incorrect signal handling:
// - pending signals are not queued
//   -> some child terminations are not handled.
//   -> parent spins forever (ccount doesn't reduce to 0).
void fork14()
{
    pid_t pid[N];
    int i;
    ccount = N;
    Signal(SIGCHLD, child_handler);

    for (i = 0; i < N; i++)
    {
        if ((pid[i] = Fork()) == 0)
        {
            Sleep(1);
            exit(0); // child exits
        }
    }

    while (ccount > 0)
        ; // parent spins
}

/* signal handler that reaps all terminated children */
void child_handler2(int sig)
{
    int old_errno = errno;

    pid_t pid;
    while ((pid = wait(NULL)) > 0)
    {
        ccount--;
        Sio_puts("Handler reaped child ");
        Sio_putl((long)pid);
        Sio_puts(" \n");
    }

    if (errno != ECHILD)
        Sio_error("wait error");

    errno = old_errno; // restore errno
}

// correct signal handling:
// - use a handler that reaps multiple children.
void fork15()
{
    pid_t pid[N];
    int i;
    ccount = N;
    Signal(SIGCHLD, child_handler2);

    for (i = 0; i < N; i++)
    {
        if ((pid[i] = Fork()) == 0)
        {
            Sleep(1);
            exit(0); // child exits
        }
    }

    while (ccount > 0)
        ; // parent spins
}

int main(int argc, char *argv[])
{
    int option = 0;
    if (argc > 1)
        option = atoi(argv[1]);

    switch (option)
    {
    case 12:
        fork12();
        break;
    case 14:
        fork14();
        break;
    case 15:
        fork15();
        break;
    default:
        printf("unknown option %d\n", option);
        break;
    }

    return 0;
}