#include "csapp.h"

#define MAXARGS 128

void eval(char *cmdline);

int main()
{
    char cmdline[MAXLINE]; // command line

    while (1)
    {
        // read
        printf("> ");
        Fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        // evaluate
        eval(cmdline);
    }
}

/* parse the command line and build argv array. */
int parseline(char *buf, char **argv)
{
    char *delim; // points to 1st space delimiter
    int argc;    // number of args
    int bg;      // is background job?

    buf[strlen(buf) - 1] = ' '; // replace trailing '\n' with space
    while ((*buf != '\0') && (*buf == ' '))
        buf++; // ignore leading spaces

    // build argv
    argc = 0;
    while ((delim = strchr(buf, ' ')) != NULL)
    {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while ((*buf != '\0') && (*buf == ' '))
            buf++; // ignore spaces
    }
    argv[argc] = NULL;

    if (argc == 0)
        return 1; // ignore blank line

    // should the job run in the background
    if (bg = (*argv[argc - 1] == '&'))
        argv[--argc] = NULL;

    return bg;
}

/* if 1st arg is builtin command, run it and return 1;
   otherwise return 0. */
int builtin_command(char **argv)
{
    if (!strcmp(argv[0], "quit")) // quit command
        exit(0);
    if (!strcmp(argv[0], "&")) // ignore
        return 1;
    return 0; // not a builtin command
}

/* evaluate a command line. */
void eval(char *cmdline)
{
    char *argv[MAXARGS]; // argument list to execve()
    char buf[MAXLINE];   // holds modified command line
    int bg;              // should the job run in the background
    pid_t pid;           // process ID

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);

    if (argv[0] == NULL)
        return; // ignore empty lines

    if (builtin_command(argv))
        return;

    if ((pid = Fork()) == 0)
    { // child runs user job
        if (execve(argv[0], argv, environ) < 0)
        {
            perror(argv[0]);
            exit(0);
        }
    }
    else
    { // parent waits for foreground job to terminate
        if (!bg)
        {
            int status;
            if (waitpid(pid, &status, 0) < 0)
                unix_error("waitfg: waitpid error");
        }
        else
            printf("%d %s", pid, cmdline);
    }
}