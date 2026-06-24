#include "csapp.h"

// 2 consecutive forks
void fork2()
{
    printf("L0\n");
    fork();
    printf("L1\n");
    fork();
    printf("Bye\n");
}
/* process graph:
                                    Bye
                            .------->x
                            |      printf
                  L1        |       Bye
          .------->x------->x------->x
          |      printf    fork    printf
          |                         Bye
          |                 .------->x
          |                 |      printf
L0        |       L1        |       Bye
 x------->x------->x------->x------->x
printf   fork    printf    fork    printf
*/

// nested forks in parent
void fork4()
{
    printf("L0\n");
    if (fork() != 0)
    {
        printf("L1\n");
        if (fork() != 0)
        {
            printf("L2\n");
        }
    }
    printf("Bye\n");
}
/* process graph:
                  Bye               Bye
          .------->x        .------->x
          |      printf     |      printf
L0        |       L1        |       L2       Bye
 x------->x------->x------->x------->x------->x
printf   fork    printf    fork    printf   printf
*/

// zombie
void fork7()
{
    if (fork() == 0)
    { // child
        printf("terminating child, PID = %d\n", getpid());
        exit(0);
    }
    else
    {
        printf("running parent, PID = %d\n", getpid());
        while (1)
            ; // infinite loop
    }
}

// non-terminating child
void fork8()
{
    if (fork() == 0)
    { // child
        printf("running child, PID = %d\n", getpid());
        while (1)
            ; // infinite loop
    }
    else
    {
        printf("terminating parent, PID = %d\n", getpid());
        exit(0);
    }
}

// wait (1 child)
void fork9()
{
    int child_status;
    if (fork() == 0)
    {
        printf("HC: hello from child\n");
        exit(0);
    }
    else
    {
        printf("HP: hello from parent\n");
        wait(&child_status);
        printf("CT: child has terminated\n");
    }
    printf("Bye");
}

// wait (multiple children)
#define N 5
void fork10()
{
    pid_t pid[N];
    int i, child_status;

    for (i = 0; i < N; i++)
        if ((pid[i] == fork()) == 0) // child
            exit(100 + i);

    // parent
    for (i = 0; i < N; i++)
    {
        pid_t wpid = wait(&child_status);
        if (WIFEXITED(child_status))
            printf("Child %d terminated with exit status %d\n",
                   wpid, WEXITSTATUS(child_status));
        else
            printf("Child %d terminated abnormally\n", wpid);
    }
}

// waitpid
void fork11()
{
    pid_t pid[N];
    int i, child_status;

    for (i = 0; i < N; i++)
        if ((pid[i] == fork()) == 0) // child
            exit(100 + i);

    // parent
    for (i = 0; i < N; i++)
    {
        pid_t wpid = wait(&child_status);
        if (WIFEXITED(child_status))
            printf("Child %d terminated with exit status %d\n",
                   wpid, WEXITSTATUS(child_status));
        else
            printf("Child %d terminated abnormally\n", wpid);
    }
}