/*
a simple, iterative HTTP/1.0 web server that uses
the GET method to serve static and dynamic content.

CGI (Common Gateway Interface):
- a standard protocol that allows a web server to talk to external
  executable programs to generate dynamic web pages.
- the web server passes the client request to a separate program,
  waits for the program to run, then sends output back to client.
*/

#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum,
                 char *shortmsg, char *longmsg);

int main(int argc, char **argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE,
                    port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }
}

/* Handle 1 HTTP request/response transaction */
void doit(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    // Read request line and headers
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))
        return;
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET"))
    {
        clienterror(fd, method, "501", "Not implemented",
                    "Tiny does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    // Parse URI from GET request
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0)
    {
        clienterror(fd, filename, "404", "Not found",
                    "Tiny couldn't find this file");
        return;
    }

    if (is_static)
    { // Serve static content
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        { // must be regular file & user running the server can read
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't read the file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    else
    { // Serve dynamic content
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        { // must be regular file & user running the server can execute
            clienterror(fd, filename, "403", "Forbidden",
                        "Tiny couldn't run the CGI program");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

/* Read HTTP request headers */
void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    while (strcmp(buf, "\r\n")) // check termination
    {
        Rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return;
}

/* Parse URI into filename and CGI args;
return 0 if dynamic content, 1 if static */
int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin"))
    {                        // static content
        strcpy(cgiargs, ""); // clear CGI arguments

        // convert to local file path
        strcpy(filename, ".");
        strcat(filename, uri);

        if (uri[strlen(uri) - 1] == '/') // default (uri = "/")
            strcat(filename, "home.html");

        return 1;
    }
    else
    {                          // dynamic content
        ptr = index(uri, '?'); // locate argument separator
        if (ptr)
        {
            strcpy(cgiargs, ptr + 1); // extract arguments
            *ptr = '\0';              // cut off arguments part (modify 'uri')
        }
        else
            strcpy(cgiargs, ""); // no arguments

        // construct executable local path
        strcpy(filename, ".");
        strcat(filename, uri);

        return 0;
    }
}

/* Copy a file back to client */
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;
    char *srcp, filetype[256], buf[MAXBUF];

    // Send response headers to client
    get_filetype(filename, filetype);
    snprintf(buf, sizeof(buf),
             "HTTP/1.0 200 OK\r\n"
             "Server: Tiny Web Server\r\n"
             "Connection: close\r\n"
             "Content-length: %d\r\n"
             "Content-type: %s\r\n\r\n",
             filesize, filetype);
    Rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    // Send response body to client
    srcfd = Open(filename, O_RDONLY, 0);
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    Close(srcfd);
    Rio_writen(fd, srcp, filesize);
    Munmap(srcp, filesize);
}

/* Derive filetype from filename */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}

/* Run a CGI program on behalf of the client */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    // Return 1st part of HTTP response
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    Rio_writen(fd, buf, strlen(buf));

    if (Fork() == 0)
    { // child process
        setenv("QUERY_STRING", cgiargs, 1);
        Dup2(fd, STDOUT_FILENO);              // redirect stdout to client
        Execve(filename, emptylist, environ); // run CGI program
    }
    Wait(NULL); // parent waits for and reaps child
}

/* Return error message to the client */
void clienterror(
    int fd, char *cause, char *errnum,
    char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    // Build the HTTP response body
    snprintf(body, sizeof(body),
             "<html>"
             "<title>Tiny Error</title>"
             "<body bgcolor=\"ffffff\">"
             "<p>%s: %s</p>"
             "<p>%s: %s</p>"
             "<hr><em>The Tiny Web server</em>"
             "</body></html>",
             errnum, shortmsg, longmsg, cause);

    // Print the HTTP response
    snprintf(buf, sizeof(buf),
             "HTTP/1.0 %s %s\r\n"
             "Content-type: text/html\r\n"
             "Content-length: %d\r\n\r\n",
             errnum, shortmsg, (int)strlen(body));
    Rio_writen(fd, buf, strlen(buf));   // response headers
    Rio_writen(fd, body, strlen(body)); // response body
}