# Client-server model

- 1 server process and 1 or more client processes.
- server manages resources.
- server provides service by manipulating resources for clients.
- server is activated by requests from clients.

# Computer network

- a group of interconnected devices.
- by geographical proximity:
  - SAN (system area network)
  - LAN (local area network)
  - WAN (wide area network)
  - Global IP Internet

# Internet protocol

- **protocol**: a set of rules that governs how hosts and routers should cooperate when they transfer data from network to network.
- provides a **naming scheme**: defines a uniform format for **host addresses**.
  - each host (and router) is assigned at least one of these internet addresses that uniquely identify it.
- provides a **delivery mechanism**: defines a standard transfer unit (**packet**)
  - packet consists of **header** and **payload**
  - header: contains packet size, source and destination address,...
  - payload: contains data sent from source host

# Global IP Internet

- Based on the TCP/IP protocol family:
  - IP (Internet Protocol): provides **basic naming scheme** and **unreliable delivery capability** of packets (datagrams) from **host to host**.
  - UDP (Unreliable Datagram Protocol): uses IP to provide **unreliable** datagram delivery from **process to process**.
  - TCP (Transmission Control Protocol): uses IP to provide **reliable** byte streams from **process to process** over **connections**.
- Accessed via **sockets interface**.

# A programmer's view of the Internet

- Hosts are mapped to a set of 32-bit **IP addresses** (IPv4).
- The set of IP addresses are mapped to a set of Internet **domain names**.
- A process on 1 Internet host can communicate with a process on another Internet host over a **connection**.

# IPv4 vs IPv6

- the original Internet Protocol with its 32-bit addresses is known as IPv4.
- IETF later introduced IPv6 with 128-bit addresses.

# IP addresses

- stored in an **IP address struct**, in **network byte order** (big-endian).

```c
/* Internet address structure */
struct in_addr {
    uint32_t s_addr; // network byte order (big-endian)
}
```

- dotted decimal notation:
  - example: 0x8002C2F2 = 128.2.194.242
  - use `getaddrinfo` and `getnameinfo` functions to convert between IP addresses and dotted decimal format.

# Internet domain names

- the Internet maintains a mapping between IP addresses and domain names in a worldwide distributed system called **domain name system (DNS)**.
- `nslookup`: explore properties of DNS mapping.
- each host has a locally defined domain name **localhost** which always maps to the **loopback address** 127.0.0.1.
- `hostname`: determine real domain name of local host.
- mapping can be:
  - one-to-one.
  - multiple domain names to the same IP addresses.
  - multiple domain names to multiple IP addresses.
  - some valid domain names don't map to any IP address.
- DNS record types:
  - A (for IPv4) and AAAA (for IPv6): map domain name to IP address.
  - MX (mail exchange): point to email server.
  - CNAME (canonical name): map domain name to another domain name (alias).
  - TXT

# Internet connections

- clients and servers communicate by sending stream of bytes over **connections**. Each connection is:
  - point-to-point: connects a pair of processes.
  - full-duplex: data can flow in both direction at the same time.
  - reliable: stream of bytes sent by source is eventually received by destination in the same order it was sent.
- a **socket** is an endpoint of the connection
  - socket address is an `IPaddress:port` pair
- a **port** is a 16-bit integer that identifies a process:
  - **ephemeral port**: assigned automatically by client kernel when client makes a connection request
  - **well-known port**: associated with some **service** provided by a server
- popular services have permanently assigned well-known ports and corresponding **well-known service names**:
  - echo server: 7/echo
  - ssh server: 22/ssh
  - email server: 25/smtp
  - web server: 80/http, 443/https
- mappings between well-known ports and service names is contained in the file `/etc/services` on Linux machine.

# Anatomy of a connection

- a connection is uniquely identified by the socket addresses of its endpoints (**socket pair**): `(client_addr:client_port, server_addr:server_port)`
- use server port to identify service.

# Socket interface

- set of system-level functions used in conjunction with Unix I/O to build network application.

## Socket

- What is a socket:
  - To the kernel, a socket is an endpoint for communication.
  - To an application, a socket is a **file descriptor** that lets the application read/write from/to the network.
- Clients and servers communicate with each other by reading from and writing to socket descriptors.

## Socket address structure

- **Generic socket address**:
  - for address arguments to `connect`, `bind`, `accept`
  - necessary only because C did not have generic (void \*) pointers when the socket interface was designed.

```c
struct sockaddr {
  uint16_t sa_family; // protocol family
  char sa_data[14]; // address data
}

// alias
typedef struct sockaddr SA;
```

- **Internet-specific socket address**:
  - must cast `struct sockaddr_in *` to `struct sockaddr *` for functions that take socket address arguments.

```c
struct sockaddr_in { /* for IPv4 */
  uint16_t sin_family; // protocol family, always AF_INET
  uint16_t sin_port; // port number in network byte order
  struct in_addr sin_addr; // IP address in network byte order
  unsigned char sin_zero[8]; // pad to sizeof(struct sockaddr)
}
```

## Socket interface

- start client (open `client_fd`): `getaddrinfo` -> `socket`
- start server (open `listen_fd`): `getaddrinfo` -> `socket` -> `bind` -> `listen`
- connection request: client `connect`, server `accept`
- client-server session: read and write to socket file descriptors
- client disconnect:
  - client `close`
  - server reads EOF and `close` client `connection_fd`
  - server awaits connection from next client with `accept`

## Host and service conversion: `getaddrinfo`

- convert string representations of host names, host addresses, ports, service names to socket address structure.
- **reentrant** _(can be safely used by threaded-programs)_ and **protocol-independent** _(works with both IPv4 and IPv6)_.

```c
int getaddrinfo(
  const char *host, /* Hostname or address */
  const char *service, /* Port or service name */
  const struct addrinfo *hints, /* Input parameters */
  struct addrinfo **result /* Output linked list */
);

void freeaddrinfo(struct addrinfo *result); /* Free linked list */

const char *gai_strerror(int errcode); /* Return error msg */
```

- given `host` and `service`, returns `result` that points to a linked-list of `addrinfo` structs, each of which points to a socket address struct, which contains arguments for the socket interface functions.
- helper:
  - `freeaddrinfo`: frees the `result` linked list.
  - `gai_strerror`: converts error code to error message.

- `addrinfo` struct:

```c
struct addrinfo {
  int ai_flags; /* Hints argument flags */
  int ai_family; /* First arg to socket function */
  int ai_socktype; /* Second arg to socket function */
  int ai_protocol; /* Third arg to socket function */
  char *ai_canonname; /* Canonical host name */
  size_t ai_addrlen; /* Size of ai_addr struct */
  struct sockaddr *ai_addr; /* Ptr to socket address structure, can be passed directly to `connect` and `bind` */
  struct addrinfo *ai_next; /* Ptr to next item in linked list */
};
```

- `result` linked-list returned by `getaddrinfo`:
  - clients: walk this list, trying each socket address in return, until the calls to `socket` and `connect` succeed.
  - servers: walk this list until calls to `socket` and `bind` succeed.

## Host and service conversion: `getnameinfo`

- inverse of `getaddrinfo`: convert a socket address to the corresponding host and service.
- reentrant and protocol-independent.

```c
int getnameinfo(
  const SA *sa, socklen_t salen, /* In: socket addr */
  char *host, size_t hostlen, /* Out: host */
  char *serv, size_t servlen, /* Out: service */
  int flags /* optional flags */
);
```
