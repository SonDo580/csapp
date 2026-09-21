#include "csapp.h"

void sigint_handler(int sig)
{
    printf("So you think you can stop the bomb with Ctrl-C, do you?\n");
    sleep(2);
    printf("Well...");
    fflush(stdout);
    sleep(1);
    printf("OK. :-)\n");
    exit(0);
}

int main()
{
    // install SIGINT handler
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        unix_error("signal error");

    // wait for receipt of a signal
    pause();

    return 0;
}