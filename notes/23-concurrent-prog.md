# Classical problem classes of concurrent program

- **Race condition**: multiple threads access shared data simultaneously, outcome depends on unpredictable order of their executions.
- **Deadlock**: threads are blocked forever, each waiting for a resource that the other thread holds.
- **Livelock**: threads are not blocked but continuously change their states without making actual progress.
- **Starvation**: a thread is perpetually denied the resources it needs to execute.

# Iterative server

- process 1 request at a time.

# Approaches for writing concurrent server

1. **Process-based**

- kernel automatically interleaves multiple logical flows.
- each flow has its own private address space.

2. **Event-based**

- programmer manually interleaves multiple logical flows.
- all flows share the same address space.
- uses technique called I/O multiplexing.

3. **Thread-based**

- kernel automatically interleaves multiple logical flows.
- each flow shares the same address space.

# Process-based concurrent server

- spawn separate process for each client (use `fork()`).

## Execution model

- fork a child to handle each client.
- no shared state between child processes.
- both parent & child have copies of `listenfd` and `connfd`.
  - parent must close `connfd`.
  - child must close `listenfd`.

## Issues

- parent must **reap** zombie children to avoid memory leak.
- parent must close `connfd`; child must close `listenfd`.
  - kernel keeps reference count for each socket.
  - after fork, `refcnt(connfd) = refcnt(listenfd) = 2`.
  - connection will not be closed until `refcnt(connfd) = 0`.

## Pros

- handle multiple connections concurrently.
- simple and straightforward.
- clean sharing model:
  - file descriptor table: no
  - open file table: yes
  - global variables: no

## Cons

- additional overhead for process control.
- nontrivial to share data between process:
  - requires IPC (**inter-process communication**) mechanisms: FIFOs (named pipes), System V shared memory and semaphores.

# Threads

- similar to approach 1, but use thread instead of process.

## Traditional view of process

- process = process context + code, data, stack
- process context:
  - program context: data registers, condition codes, stack pointer (SP), program counter (PC)
  - kernel context: VM structures, descriptor table, `brk` pointer
- code, data, stack:
  - stack <- SP
  - shared libraries
  - runtime heap <- brk
  - read/write data
  - read-only code/data <- PC

## Alternate view of process

- process = thread(s) + code, data, kernel context
- thread:
  - stack <- SP
  - thread context: data registers, condition codes, stack pointer (SP), program counter (PC)
- code, data: - shared libraries
  - runtime heap <- brk
  - read/write data
  - read-only code/data <- PC
- kernel context: VM structures, descriptor table, `brk` pointer

## A process with multiple threads

- each thread has its own logical control flow.
- threads share the same code, data, kernel context.
- each thread has its own stack for local variables (but not protected from other thread).
- each thread has its own thead id (TID).

## Local view of threads

- threads associated with process form a pool of peers (unlike processes which form a tree hierarchy).

## Concurrent threads

- 2 threads are concurrent if their flows overlap in time; otherwise they are sequential.

## Concurrent thread execution

- single-core processor: simulate parallelism by time slicing.
- multi-core processor: can have true parallelism.

## Threads vs. processes

- **Similarities**:
  - each has its own logical control flow.
  - each can run concurrently with others (possibly on different cores).
  - each is context-switched.
- **Differences**:
  - threads share all code and data (except local stack), processes (typically) do not.
  - thread control (creating and reaping) are less expensive than process control.

## POSIX threads (`Pthreads`) interface

- creating and reaping threads: `pthread_create`, `pthread_join`
- determining current thread ID: `pthread_self`
- terminating threads: `pthread_cancel`, `pthread_exit`
  - `exit` (terminate process and all associated threads)
- synchronizing access to shared variables: `pthread_mutex_init`, `pthread_mutex_lock`, `pthread_mutex_unlock`

# Thread-based concurrent server

## Execution model

- each client handled by individual peer thread.
- threads share all process state except TID.
- each thread has a separate stack for local variables.

## Issues

- **must run detached to avoid memory leak**:
  - at any point in time, a thread is either **joinable** or **detached**.
  - joinable thread can be reaped and killed by other threads: must be reaped with `pthread_join` to free memory resource.
  - detached thread cannot be reaped or killed by other threads: resources are automatically reaped on termination.
  - default state is joinable: use `pthread_detach(pthread_self())` to make detached.
- **must be careful to avoid unintended sharing**:
  - [example] passing pointer to main thread's stack:
    `pthread_create(&tid, NULL, thread, (void *)&connfd);`
- **all functions called by a thread must be thread-safe** _(next lecture)_

## Pros

- easy to share data between threads.
- more efficient than processes.

## Cons

- unintentional sharing can produce subtle and hard-to-reproduce errors.

# Event-based server

- server maintains set of active connection (array of `connfd`'s).
- repeat:
  - determine which descriptors (`connfd`'s or `listenfd`) have pending inputs:
    - use `select` or `epoll` functions.
    - arrival of pending input is an **event**.
  - if `listenfd` has input, `accept` connection and add new `connfd` to array.
  - service all `connfd`'s with pending input.

## Pros

- 1 logical control flow and address space.
- can single-step with a debugger.
- no process or thread control overhead.

## Cons

- significantly more complex to implement than process-based and thread-based designs.
- hard to provide fine-grained concurrency.
- cannot take advantage of multi-core CPU.
