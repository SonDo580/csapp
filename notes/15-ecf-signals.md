# Linux process hierarchy

```
[0] ---> init [1] ---> daemon (e.g.httpd)
            |--------> login shell -> ...
            |...
            |--------> login shell -> ...
```

- view the hierarchy using `pstree` command

# Shell programs

- a **shell** is an application program that runs programs on behalf of the user: sh, csh/zsh, bash.

## Simple shell example

- see `code/15.../shellex.c`
- correctly waits for and reaps foreground jobs.
- **problems**: background jobs
  - will become zombie when they terminate.
  - will never be reaped because shell (typically) will not terminate.
  - will create a memory leak that could run kernel out of memory.
- **solution**: exceptional control flow
  - the kernel will interrupt regular processing to alert when a background process completes.
  - in Unix, the alert mechanism is called a **signal**.

# Signals

- a **signal** is a small message that notifies a process that an event of some type has occurred in the system.
  - akin to **exceptions** and **interrupts**.
  - sent from kernel _(sometimes at the request of another process)_ to a process.
  - signal type is identified by a small integer ID (1-30).
  - only information in a signal is its ID and the fact that it arrived.
- ID, name, default action, corresponding event:
  - 2, SIGINT, terminate, user typed Ctrl-C.
  - 9, SIGKILL, terminate, kill program (cannot override or ignore).
  - 11, SIGSEGV, terminate & dump, segmentation violation.
  - 14, SIGALRM, terminate, timer signal.
  - 17, SIGCHILD, ignore, child stopped or terminated.

## Sending a signal

- kernel sends a signal to a **destination process** by updating some state in the context of the destination process.
- kernel sends a signal for 1 of the following reasons:
  - kernel has detected a system event such as divide-by-0 (send `SIGFPE`) or the termination of a child process (send `SIGCHILD`)
  - another process has invoked the `kill` system call to explicitly request the kernel to send a signal to the destination process.

## Receiving a signal

- a destination process receives a signal when it is forced by the kernel to react in some way to the delivery of the signal.
- some possible ways to react:
  - **ignore**
  - **terminate** the process (with optional core dump)
  - **catch** the signal by executing a user-level function call **signal handler**
    - akin to hardware **exception handler** being called in response to an asynchronous interrupt.
    - signal handler returns to next instruction.

## Pending and blocked signals

- a signal is **pending** if sent but not yet received
  - there can be at most 1 pending signal of any particular type (for 1 process).
  - signals are not queued: if a process has a pending signal of type k, then subsequent signals of type k that are sent to that process are discarded.
- a process can **block** the receipt of certain signals
  - blocked signals can be delivered, but will not be received until the signal is unblocked.
- a pending signal is received at most once.

## Pending/blocked bits

kernel maintains **pending** and **blocked** bit vectors in the context of each process.

- **pending**: represents the set of pending signals
  - kernel sets bit k when a signal of type k is delivered to process
  - kernel clears bit k when a signal of type k is received by process
- **blocked**: represents the set of blocked signals
  - can be set and cleared by process using the `sigprocmask` function
  - also referred to as the **signal mask**

# Sending signals

## Process groups

- every process belongs to exactly 1 process group

## Sending signals with `/bin/kill` program

- sends arbitrary signal to a process or process group.
- examples:
  - `/bin/kill -9 24818`: Send `SIGKILL` to process 24818.
  - `/bin/kill -9 -24817`: Send `SIGKILL` to process group 24817.

## Sending signals from keyboard

- typing Ctrl-C / Ctrl-Z causes the kernel to send a `SIGINT` / `SIGTSTP` to every job in the foreground process group.
  - `SIGINT`: default action is to terminate each process.
  - `SIGTSTP`: default action is to stop (suspend) each process.

## Sending signals with `kill` function

See `code/15.../forks.c` - fork12

# Receiving signals

- suppose kernel is retuning from an exception handler and is ready to pass control to process p.
- kernel computes `pnb = pending & ~blocked` - the set of pending non-blocked signals for process p.
- if `pnb == 0`: pass control to next instruction in the logical flow of p.
- else:
  - choose least nonzero bit k in `pnb` _(lower signal numbers have higher priority)_ and force p to **receive** signal k.
  - the receipt of the signal triggers some **action** by p.
  - repeat for all nonzero k in `pnb`.
  - pass control to next instruction in logical flow for p.

# Default actions

each signal type has a predefined default action, which is one of:

- the process terminates.
- the process terminates and dumps core.
- the process stops until restarted by a SIGCONT signal.
- the process ignores the signal.

# Installing signal handlers

- the `signal` function modifies the default action associated with the receipt of signal `signum`:
  `handler_t *signal(int signum, handler_t *handler)`
- different values for `handler`:
  - `SIG_IGN`: ignore signals of type `signum`
  - `SIG_DFL`: revert to default action on receipt of signals of type `signum`
  - otherwise, `handler` is the address of a user-level **signal handler**
    - called when process receives signal of type `signum`.
    - referred to as **installing** the handler
    - executing handler is called **catching** or **handling** the signal.
    - when a signal handler returns, control passes back to the instruction of the process that was interrupted.
- signal handling example: `code/15.../sigint.c`

# Signal handlers as concurrent flows

- a signal handler is a separate logical flow (not process) that runs concurrently with the main program.

# Nested signal handlers

- handlers can be interrupted by other handlers.

# Blocking and unblocking signals

- implicit blocking mechanism: kernel blocks any pending signals of type currently being handled.
- explicit blocking and unblocking mechanism: `sigprocmask` function.
- supporting functions:
  - `sigemptyset`: create empty set.
  - `sigfillset`: add every signal number to set.
  - `sigaddset`: add signal number to set.
  - `sigdelset`: delete signal number from set.
- temporarily blocking signals:

```c
sigset_t mask, prev_mask; // represents blocked signals

Sigemptyset(&mask);
Sigaddset(&mask, SIGINT);

// block SIGINT and save previous blocked set
Sigprocmask(SIG_BLOCK, &mask, &prev_mask);

// ... code region that will not be interrupted by SIGINT

// restore previous blocked set, unblocking SIGINT
Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
```

# Safe signal handling

- keep handlers as simple as possible
  - e.g., set a global flag and return.
- call only **async-signal-safe** functions in handlers:
  - `printf`, `sprintf`, `malloc`, `exit` are not safe.
- save and restore `errno` on entry and exit
  - so that handlers don't overwrite `errno` value of main flow.
- protect accesses to shared data structures by temporarily blocking all signals
  - to prevent possible corruption.
- declare global variable as `volatile`
  - to prevent compiler from storing them in a register.
- declare global flags as `volatile sig_atomic_t`
  - flag: variable that is only read or written (e.g., `flag = 1`, not `flag++`)
  - flag declared this way does not need to be protected like other globals.

# Async-signal-safety

- function is `async-signal-safe` if either **reentrant** (e.g., all variables stored on stack frame) or cannot be interrupted by signals.
- async-signal-safe functions (POSIX):
  - popular functions on the list: `_exit`, `write`, `wait`, `waitpid`, `sleep`, `kill`.
  - popular functions not on the list: `printf`, `sprintf`, `malloc`, `exit`.

# Safely generating formatting output

- Use reentrant SIO (safe I/O library, see `csapp.c`) in signal handlers.

```c
ssize_t sio_puts(char s[]) /* Put string */
ssize_t sio_putl(long v) /* Put long */
void sio_error(char s[]) /* Put msg & exit */
```

- Example: `code/15.../sigintsafe.c`

# Incorrect signal handling

- Example: `code/15.../forks.c` - fork14().
- Pending signals are not queued:
  - there can be at most 1 pending signal of any type.
- Can't use signals to count events, such as children terminating in fork14().

# Correct signal handling

- Must wait for all terminated child processes:
  - Put `wait` in a loop to reap all terminated children.
- Example: `code/15.../forks.c` - fork15().

# Portable signal handling with `sigaction`

TODO

# Synchronizing flows to avoid races

TODO

# Explicitly waiting for signals

TODO

# Waiting for signals with `sigsuspend`

TODO

# Non-local jumps

TODO
