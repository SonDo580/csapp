# Control flow

- from startup to shutdown, the CPU simply reads and executes a sequence of instructions, one at a time.
- this sequence is the CPU's **control flow**.

# Altering control flow

- 2 mechanisms (react to changes in **program state**):
  - jumps and branches.
  - call and return.
- insufficient, difficult to react to changes in **system state**:
  - data arrives from a disk or a network adapter.
  - instruction divides by 0.
  - use hits Ctrl-C at the keyboard.
  - system timer expires.
- system needs mechanisms for **"exceptional control flow"**.

# Exceptional control flow

- exists at all levels of a computer system.

## Low-level mechanisms

1. **Exceptions**

- change in control flow in response to a system event.
- implemented using combination of hardware and OS software.

## Higher-level mechanisms

2. **Process context switch**

- implemented by OS software and hardware timer.

3. **Signals**

- implemented by OS software.

4. **Non-local jumps**: `setjmp()` and `longjmp()`

- implemented by C runtime library.

# Exceptions

- an **exception** is a transfer of control to the OS kernel in response to some event _(change in processor state)_.
  - kernel is the memory-resident part of the OS.
  - example events: divide by 0, arithmetic overflow, page fault, I/O request completes, typing Ctrl-C.

# Exception table

- each type of event has a unique exception number k.
- k = index into exception table (interrupt vector)
- handler k is called each time exception k occurs.

# Asynchronous exceptions (interrupts)

- caused by external events to the processor.
  - indicated by setting the processor's **interrupt pin**.
  - handler returns to next instruction.
- examples:
  - timer interrupt:
    - every few ms, an external timer chip triggers an interrupt.
    - used by the kernel to take back control from user programs.
  - I/O interrupt from external device:
    - hitting Ctrl-C at the keyboard.
    - arrival of a packet from a network.
    - arrival of data from a disk.

# Synchronous exceptions

- caused by events that occur as a result of executing an instruction:
  - **traps**:
    - intentional
    - examples: **system calls**, breakpoint traps, special instructions
    - return control to next instruction.
  - **faults**:
    - unintentional but possibly recoverable.
    - examples: page faults (recoverable), protection faults (unrecoverable), floating point exceptions.
    - either re-executes faulting (current) instruction or aborts.
  - **aborts**:
    - unintentional and unrecoverable.
    - examples: illegal instruction, parity error, machine check.
    - aborts current program.

# System calls

- each x86-64 system call has a unique ID number.
- examples: read, write, open, close, stat, fork, execve, kill, ...

## System call example: opening file

- user calls: `open(filename, options)`
- calls `__open` function, which invokes system call instruction `syscall`

```asm
<__open>:
    ...
    mov $0x2,%eax                   # open is syscall #2
    syscall                         # Return value in %rax
    cmp $0xfffffffffffff001,%rax    # (error range: -1 -> -4095)
    ...
    retq
```

## Fault example: page fault

- user write to memory location; that portion (page) of user's memory is currently on disk.

```c
int a[1000];
main ()
{
    a[500] = 13;
}

// assembly
movl $0xd,0x8049d10

/*
user code                       kernel code
    |
movl --- exception: page fault --->
    |<---                         | copy page from disk to memory
    |   |------ return & ---------v
    v       re-execute movl
*/
```

## Fault example: invalid memory reference

```c
int a[1000];
main ()
{
    a[5000] = 13;
}

// assembly
movl $0xd,0x804e360

/*
user code                       kernel code
    |
movl --- exception: page fault --->
                                  | detect invalid address
                                  |
                                  ---> signal process
*/
```

- send `SIGSEGV` signal to user process.
- user process exits with **segmentation fault**.

# Processes

- def: a **process** is an instance of a running program.
- provides each program with 2 key **abstractions**:
  - **logical control flow**:
    - each program seems to have exclusive use of the CPU.
    - provided by kernel mechanism call **context switching**.
  - **private address space**:
    - each program seems to have exclusive use of main memory.
    - provided by kernel mechanism call **virtual memory**.
- identified by PID (process ID).

# Multi-processing: the illusion

- computer runs many processes simultaneously.
  - applications for 1 or more users: web browsers, editors, ...
  - background tasks: monitoring network & I/O devices, ...

# Multi-processing: traditional reality

- single processor executes multiple processes simultaneously.
  - process executions interleaved (multitasking).
  - address spaces managed by virtual memory system.
  - register values for non-executing processes saved in memory.
- context switch:
  - save current registers in memory.
  - schedule next process for execution.
  - load saved registers and switch address space.

# Multi-processing: modern reality

- multi-core processors:
  - multiple CPUs on a single chip.
  - share main memory (and some of the caches).
  - each can execute a separate process.
  - scheduling of processes on to cores done by kernel.

# Concurrent processes

- each process is a logical control flow.
- 2 processes are **concurrent** if their flows overlap in time.
- otherwise, they are **sequential**

# User view of concurrent processes

- control flows for concurrent processes are physically disjoint in time.
- we can think of concurrent processes as running in parallel with each other.

# Context switching

- processes are managed by a shared chunk of memory-resident OS code called the **kernel** (the kernel is not a separate process, but runs as part of some exiting process).
- control flow passes from 1 process to another via a **context switch**

# System call error handling

- on error, Linux system-level functions typically return -1 and set global variable `errno` to indicate cause.
- hard and fast **rule**:
  - must check the return status of every system-level function.
  - only exception is the functions that return `void`.
- example:

```c
if ((pid = fork()) < 0) {
  fprintf(stderr, "fork error: %s\n", strerror(errno));
  exit(0);
}
```

## Error-reporting functions

```c
void unix_error(char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(0);
}

if ((pid = fork()) < 0)
  unix_error("fork error");
```

## Error-reporting wrappers

```c
pid_t Fork(void) {
  pid_t pid;
  if((pid = fork()) < 0)
    unix_error("fork error");
  return pid;
}

// usage
pid = Fork();
```

# Obtaining process ID

- `pid_t getpid(void)`: returns PID of current process.
- `pid_t getppid(void)`: returns PID of parent process.

# Creating and terminating processes

we can think of a process as being in 1 of 3 states

- **running**: process is either **executing**, or **waiting** to be executed and will eventually be **scheduled** by the kernel.
- **stopped**: process execution is **suspended** and will not be scheduled until further notice.
- **terminated**: process is stopped permanently.

# Terminating processes

- process becomes terminated for 1 of 3 reasons:
  - receiving a signal whose default action is to terminate.
  - returning from the `main` routine.
  - calling the `exit` function.
- `void exit(int status)`:
  - terminate with exit status of `status`.
  - convention: normal return status is 0, nonzero on error.
  - another way to explicit set the exit status is to return an integer from the `main` routine.
  - `exit` is called once but never returns.

# Creating processes

- parent process creates a new running child process by calling `fork`
- `int fork(void)`:
  - returns 0 to child process, child's PID to parent process.
  - child is _almost_ identical to parent:
    - child gets an identical (but separate) copy of parent's virtual address space.
    - child gets identical copies of parent's open file descriptors.
    - child has different PID than parent.
- `fork` is called once but return twice.

## Modeling `fork` with process graphs

- **process graph**:
  - each vertex is the execution of a statement.
  - a -> b means a happens before b.
  - edges can be label with current value of variables.
  - `printf` vertices can be labeled with output.
  - each graph begins with a vertex with no in-edges.
- any topological sort of the graph corresponds to a feasible total ordering.

## `fork` examples:

- simple: `code/14.../fork.c`
- 2 consecutive forks: `code/14.../forks.c` - fork2
- nested forks in parent: `code/14.../forks.c` - fork4

# Reaping child processes

- when process terminates, it still consume system resources (zombie process).
- **reaping**:
  - performed by parent on terminated child (using `wait` or `waitpid`)
  - parent is given status information.
  - kernel then deletes zombie child process.
- what if parent doesn't reap:
  - the orphaned child will be reaped by `init` process (pid == 1).
  - so, only need explicit reaping in long-running process (shells, servers).
- zombie example: `code/14.../forks.c` - fork7
- non-terminating child example: `code/14.../forks.c` - fork8

# `wait`: synchronizing with children

- parent reaps a child by calling the wait function.
- `int wait(int *child_status)`:
  - suspend current process until 1 of its children terminates.
  - return value is pid of the child process that terminated.
  - if `child_status != NULL`, the integer it points to will be set to a value that indicates reason the child terminated and the exit status.
    - checked using macros defined in `wait.h`: WIFEXITED, WEXITSTATUS, WIFSIGNALED, WTERMSIG, WIFSTOPPED, WSTOPSIG, WIFCONTINUED
- example: `code/14.../forks.c` - fork9, fork10

# `waitpid`: waiting for a specific process

- `pid_t waitpid(pid_t pid, int &status, int options)`:
  - suspend current process until specific process terminates.
- example: `code/14.../forks.c` - fork11

# `execve`: loading and running program

- `int execve(char *filename, char *argv[], char *envp[]`
- loads and run in current process:
  - executable `filename`: object file or script file (e.g., #!/bin/bash)
  - ...with argument list `argv` (argv[0] = filename)
  - ...and environment variable list `envp`:
    - `name=value` strings
    - manipulate with: `getenv`, `putenv`, `printenv`
- overwrites code, data, stack; retains PID, open files and signal context.
- called once and never returns (except if there is an error).
