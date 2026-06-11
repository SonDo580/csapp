/*
a minimal CGI program that adds 2 numbers together
*/

#include "csapp.h"

int main(void)
{
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1 = 0, n2 = 0;

    // Extract the 2 arguments
    if ((buf = getenv("QUERY_STRING")) != NULL)
    {
        p = strchr(buf, '&');
        *p = '\0';
        strcpy(arg1, buf);
        strcpy(arg2, p + 1);
        n1 = atoi(arg1);
        n2 = atoi(arg2);
    }

    // Make the response body
    snprintf(content, sizeof(content),
             "Welcome to add.com: THE Internet addition portal.\r\n"
             "The answer is: %d + %d = %d\r\n"
             "Thanks for visiting!\r\n",
             n1, n2, n1 + n2);

    // Generate the HTTP response
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}