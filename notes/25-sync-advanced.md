# Using semaphore to coordinate access to shared resources

## Basic idea

- use counting semaphores to keep track of resource state and to notify other threads.
- use mutex to protect access to resource.

## 2 class examples

- producer-consumer problem.
- readers-writers problem.

# Producer-Consumer problem

[producer thread] -> [shared buffer] -> [consumer thread]

## Common synchronization pattern

- producer waits for empty **slot**, inserts item into buffer, and notifies consumer.
- consumer waits for **item**, removes it from buffer, and notifies producer.

## Examples

- **Multimedia processing**:
  - producer creates MPEG video frames, consumer renders them.
- **Event-driven graphical user interfaces**:
  - producer detects mouse clicks, mouse movements, and keyboard hits and inserts corresponding events in buffer.
  - consumer retrieves events from buffer and paints the display.

## Producer-Consumer on an n-element buffer

- require a mutex and 2 counting semaphores:
  - `mutex`: enforces mutually exclusive access to buffer.
  - `slots`: counts the available slots in buffer.
  - `items`: counts the available items in buffer.
- implemented using a shared buffer package called `sbuf`: see `code/25.../sbuf.*`

# Readers-Writers problem

## Problem statement

- reader threads only read the object.
- writer threads modify the object.
- writers must have exclusive access to the object.
- unlimited number of readers can access the object.

## Examples

- online airline reservation system.
- multithreaded caching web proxy.

## Variants

- **1st readers-writers problem (favor readers)**:
  - no reader should be kept waiting unless a writer has already been granted permission to use the object.
  - a reader that arrives after a waiting writer gets priority over the writer.
- **2nd readers-writers problem (favor writers)**:
  - once a writer is ready to write, it performs its write as soon as possible.
  - a reader that arrives after a writer must wait, even if the writer is also waiting; a writer arrives after a waiting reader gets priority over the reader.
- **starvation** (a thread waits indefinitely) is possible in both cases.

## Solution to 1st Readers-Writers problem

See `code/25.../rw1.c`

# Pre-threaded concurrent server

- **threaded concurrent server**: the main thread waits for client connections and spawn a new thread to handle each client.
- **pre-threaded concurrent server**:
  - spawn a pool of worker threads in advance.
  - the main thread accept connections and add `connfd`'s to buffer.
  - each worker thread remove a `connfd` from buffer and handle a client.
  - see `code/25.../echoservert_pre.c`

# Thread safety

- functions called from a thread must be **thread-safe**
- **def**: a function is thread-safe if it always produce correct results when called repeatedly from multiple concurrent threads.

# Thread-unsafe functions

## Class 1: functions that do not protect shared variables

- fix: use P and V semaphore operations.
- example: `code/24.../goodcnt.c`.
- issue: synchronization operations slow down code.

## Class 2: functions that rely on persistent state across multiple invocations

- example: random number generator that relies on static state.

```c
static unsigned int next = 1;

/* rand: return pseudo-random integer on 0..32767 */
int rand(void) {
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

/* srand: set seed for rand() */
void srand(unsigned int seed) {
  next = seed;
}
```

- thread-safe random number generator:

```c
// pass state as part of argument (eliminate global state)
int rand_r(int *nextp)
{
  *nextp = *nextp * 1103515245 + 12345;
  return (unsigned int)(*nextp / 65536) % 32768;
}
// consequence: program using rand_r must maintain seed
```

## Class 3: functions that return a pointer to static variable

- example:

```c
static char static_time_buffer[26];

char *ctime(const time_t *timep) {
  // ... modify static_time_buffer
  return static_time_buffer;
}
```

- fix 1: **rewrite function** so caller passes address of variable to store result _(require changes in caller and callee)_.

```c
/* ctimer_r: thread-safe rewrite. */
char *ctimer_r(const time_t *timep, char *result_buf) {
  // ... result is written directly to result_buf
  // no lock is needed since result_buf belongs to calling thread's stack
  return result_buf
}
```

- fix 2: **lock and copy** _(require simple changes in caller and none in callee; caller must free memory)_.

```c
/* thread-safe wrapper function. */
char *ctime_cs(const time_t *timep, char *privatep) {
  char *sharedp;

  P(&mutex); // lock
  sharedp = ctime(timep); // call the thread-unsafe function
  strcpy(privatep, sharedp); // copy result to calling thread's stack
  V(&mutex); // unlock
  return privatep;
}
```

## Class 4: functions that call thread-unsafe functions

- fix: modify the function so it only calls thread-safe functions.

# Reentrant functions

- **def**: a function is **reentrant** if it accesses no shared variables when called by multiple threads.
- is an important **subset** of thread-safe functions:
  - require no synchronization operations.
  - the only way to make a Class 2 function thread-safe is to make it reentrant (e.g., `rand_r`).

# Thread-safe library functions

- all functions in the standard C library are thread-safe
  - examples: `malloc`, `free`, `printf`, `scanf`, ...
- most Unix system calls are thread-safe, with a few exceptions:

```
Thread­‐unsafe function   Class   Reentrant	version
asctime                  3       asctime_r
ctime                    3       ctime_r
gethostbyaddr            3       gethostbyaddr_r
gethostbyname            3       gethostbyname_r
inet_ntoa                3       (none)
localtime                3       localtime_r
rand                     2       rand_r
```

# Race condition

- a **race** occurs when correctness of the program depends on 1 thread reaching point x before another thread reaches point y.
- example: see `code/25.../race.c`
- fix: see `code/25.../norace.c` (avoid unintended sharing of state).

# Deadlock

- **def**: a process is **deadlocked** if it is waiting for a condition that will never be true.
- **typical scenario**:
  - processes 1 and 2 needs 2 resources (A and B) to proceed.
  - process 1 acquires A, waits for B.
  - process 2 acquires B, waits for A.
  - both will wait _forever_.

## Example

- code: `code/25.../deadlock.c`.

```
Tid[0]      Tid[1]
-------------------
P(s0)       P(s1)
P(s1)       P(s0)
cnt++       cnt++
V(s0)       V(s1)
V(s1)       V(s0)
```

- **process graph**: any trajectory that enters **deadlock region** will eventually reach **deadlock state**.

```
. FR0: forbidden region for s0
. FR1: forbidden region for s1
. DR: deadlock region
. DS: deadlock state

    thread 1
       |
V(s0) -|    -------------
       |    | FR0       |
V(s1) -|    |     -------------
       |    |     |     |     |
P(s0) -|    -------<============== DS
       |    | DR  |       FR1 |
P(s1) -|    -------------------
       |
       |____________________________
s0=s1=1     '     '     '     '     thread 0
          P(s0) P(s1) V(s0) V(s1)

- Case 1:
  . Tid[0]:P(s0)
  . Tid[1]:P(s1)
  . Tid[0]:P(s1) (wait)
  . Tid[1]:P(s0) (wait)
  . both cannot proceed.

- Case 2:
  . Tid[1]:P(s1)
  . Tid[0]:P(s0)
  . Tid[1]:P(s0) (wait)
  . Tid[0]:P(s1) (wait)
  . both cannot proceed.

- Other trajectories luck out and skirt the deadlock region
```

## Avoid deadlock

- **always acquire shared resources in the same order**.
- code: `code/25.../nodeadlock.c`.

```
Tid[0]      Tid[1]
-------------------
P(s0)       P(s0)   |<- acquire locks in the same order
P(s1)       P(s1)   |
cnt++       cnt++
V(s0)       V(s1)   |<- release locks in any order
V(s1)       V(s0)   |
```

- **process graph**: no way for trajectory to get stuck

```
    thread 1
       |
V(s0) -|    -------------
       |    | FR0       |
V(s1) -|    |     -------------
       |    |     |     | FR1 |
P(s1) -|    |     -------------
       |    |           |     
P(s0) -|    -------------
       |
       |____________________________
s0=s1=1     '     '     '     '     thread 0
          P(s0) P(s1) V(s0) V(s1)
```
