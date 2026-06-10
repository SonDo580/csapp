# Socket interface

## `socket`

```c
int socket(int domain, int type, int protocol);

// Example
int clientfd = Socket(AF_INET, SOCK_STREAM, 0);
```

- client and server use `socket` to create a socket descriptor
- best practice: use `getaddrinfo` to generate parameters

## `bind`

```c
int bind(int sockfd, SA *addr, socklen_t addrlen);
```

- server uses `bind` to ask the kernel to associate server's socket address with a socket descriptor.
- the process can read bytes that arrive on the connection whose endpoint is `addr` by reading from descriptor `sockfd`.
- writes to `sockfd` is transferred along connection whose endpoint is `addr`.
- best practice: use `getaddrinfo` to supply arguments `addr` and `addrlen`.

## `listen`

```c
int listen(int sockfd, int backlog);
```

- by default, kernel assumes that descriptor from `socket` function is an **active socket** that will be on the client end of a connection.
- server calls `listen` to tell the kernel that a descriptor will be used by a server rather than a client.
- `listen` converts `sockfd` from an active socket to a **listening socket** that can accept connection requests from clients.
- `backlog` is a hint about the number of outstanding connection requests that the kernel should queue up before starting to refuse requests.

## `accept`

```c
int accept(int listenfd, SA *addr, int *addrlen);
```

- for a server to wait for connection requests from clients.
- `accept` waits for connection request to arrive on the connection bound to `listenfd`, then fills in the client's socket address in `addr` and size of socket address in `addrlen`
- returns a **connected descriptor** that can be used to communicate with client via Unix I/O routines.

## `connect`

```c
int connect(int clientfd, SA *addr, socklen_t addrlen);
```

- for a client to establish a connection with a server at socket address `addr`.
- if successful, `clientfd` is now ready for reading and writing.
- resulting connection is characterized by socket pair: `(client_addr:ephemeral_port, addr.sin_addr:addr.sin_port)`.
- best practice: use `getaddrinfo` to supply arguments `addr` and `addrlen`.

## Connection vs Listening descriptors

- Listening descriptor:
  - endpoint for client connection requests.
  - created once and exists for lifetime of the server.
- Connection descriptor:
  - endpoint of the connection between client and server.
  - created each time the server accepts a connection requests from a client.
  - exists only as long as it takes to service client.
- Why the distinction:
  - allows for concurrent servers that can communicate over many client connections simultaneously _(for example, folk a child to handle each request)_.

# Socket helpers

## `open_clientfd`

- Establish a connection with a server

```c
int open_clientfd(char *hostname, char *port) {
    int clientfd;
    struct addrinfo hints, *listp, *p;

    // Get a list of potential server addresses
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; // open a connection
    hints.ai_flags = AI_NUMERICSERV; // using numeric port arg
    hints.ai_flags |= AI_ADDRCONFIG; // restricts DNS lookups to only match IP protocols configured on local system
    Getaddrinfo(hostname, port, &hints, &listp);

    // Walk the list for the one that we can connect
    for (p = listp; p; p = p->ai_next) {
        // Create a socket descriptor
        if ((clientfd = socket(p->ai_family, p->ai_socktype,
                               p->ai_protocol)) < 0)
            continue; // failed, try next addr

        // Connect to the server
        if (connect(clientfd, p->ai_addr, a->ai_addrlen) != -1)
            break; // success

        Close(clientfd); // connect failed, try next addr
    }

    // Clean up
    if (!p) // all connects failed
        return -1;
    else // the last connect succeeded
        return clientfd;
}
```

## `open_listenfd`

- Create a listening descriptor that can be used to accept connection requests from clients.

```c
int open_listenfd(char *port) {
    struct addrinfo hints, *listp, *p;
    int listenfd, optval=1;

    // Get a list of potential server addresses
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM; // accept connection
    hint.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // listen on all network interfaces
    hints.ai_flags |= AI_NUMERICSERV; // use port number
    Getaddrinfo(NULL, port, &hints, &listp);

    // Walk the list for one that we can bind to
    for (p = listp; p; p = p->ai_next) {
        // Create a socket descriptor
        if ((listenfd = socket(p->ai_family, p->ai_socktype,
                               p->ai_protocol)) < 0)
            continue; // failed, try next addr

        // Eliminate "address already in use" error from bind
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void*)&optval, sizeof(int));

        // Bind the descriptor to the address
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break; // success

        Close(listenfd); // bind failed, try next addr
    }

    // Cleanup
    Freeaddrinfo(listp);
    if (!p) // no addresses worked
        return -1;

    // Make it a listening socket ready to accept connection requests
    if (listen(listenfd, LISTENQ) < 0) {
        Close(listenfd);
        return -1;
    }

    return listen_fd;
}
```

# Testing server: `telnet`

```bash
# Create a connection running on <host> and listening on <port>
telnet <host> <port>
```

# Web server basics

- clients and servers communicate using the (HyperText Transfer) HTTP protocol.
  - client and server establish TCP connection.
  - client requests content.
  - server responds with requested content.
  - client and server close connection (eventually).

# Web content

- **content**: a sequence of bytes with an associated MIME (Multipurpose Internet Mail Extensions) type
- example **MIME types**: text/html, text/plain, image/gif, image/png, image/jpeg, ...
- content can be **static** (stored in files) or **dynamic** (produced on-the-fly)

# URL (Unique Resource Locator)

- example: "http://www.cmu.edu:80/index.html"
- client uses prefix "http://www.cmu.edu:80" to infer:
  - protocol (HTTP)
  - where the server is (www.cmu.edu)
  - what port it is listening on (80)
- server uses suffix "/index.html" to:
  - determine if request is for static or dynamic content.
  - find file on file system.

# HTTP requests

- a **request line**, followed by zero or more **request headers**
- request line: `<method> <uri> <verion>`
  - method: GET, POST, OPTIONS, HEAD, PUT, PATCH, DELETE, TRACE
  - uri: URL for proxies, URL suffix for servers
  - version: HTTP version (HTTP/1.0, HTTP/1.1)
- request header: `<header name>: <header_data>`
  - provide additional info to server

# HTTP responses

- a **response line** followed by zero or more **response headers**, possibly followed by **content**, with blank line (`\r\n`) separating headers from content.
- response line: `<version> <status code> <status msg>`
  - version: HTTP version of the response
  - status code, status msg: numeric status and corresponding text
- response header: `<header name>: <header data>`
  - provide additional info about response

# HTTP versions

- Major differences between HTTP/1.0 and HTTP/1.1:
  - HTTP/1.0 uses a new connection for each transaction.
  - HTTP/1.1 also supports persistent connections:
    - multiple transactions over the same connection
    - `Connection: Keep-Alive`
  - HTTP/1.1 requires `Host` header
    - makes it possible to host multiple websites at single Internet host.
  - HTTP/1.1 supports **chunked** encoding
    - `Transfer-Encoding: chunked`
  - HTTP/1.1 adds additional support for caching.

# Data transfer mechanisms
- **standard**:
  - specify total length with `Content-Length` header.
  - requires that program buffer the entire message.
- **chunked**:
  - break into blocks.
  - prefix each block with number of bytes (hex coded).

# Proxies
- an intermediary between a client and an **origin server**
  - to client, proxy acts like server
  - to server, proxy acts like client
- can perform useful functions as requests and responses pass by: caching, logging, filtering, ... 