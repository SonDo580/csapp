#include "csapp.h"

/* safe sigint handler */
void sigint_handler(int sig)
{
    Sio_puts("So you think you can stop the bomb with ctrl-c, do you?\n");
    sleep(2);
    Sio_puts("Well...");
    sleep(1);
    Sio_puts("OK. :-)\n");
    _exit(0);
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