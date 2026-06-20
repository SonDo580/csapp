# Shared variables in threaded C programs

- a variable `x` is shared <-> multiple threads reference one instance of `x`.

# Thread memory model

## Conceptual

- multiple threads run within the context of a single process
- each thread has its own separate thread context:
  - thread ID, stack, stack pointer, program counter, condition codes, general-purpose registers
- all threads share the same remaining process context:
  - code, data, heap, shared library segments of the process's virtual address space.
  - open files and installed handlers.

## Operational

- conceptual model is not strictly enforced.
- any thread can read and write the stack of of any other thread.

# Mapping variable instances to memory

- **Global variables**:
  - def: variable declared outside of a function.
  - virtual memory contains exactly 1 instance of any global variable.
- **Local variables**:
  - def: variable declared inside function without `static` attribute.
  - each thread stack contains 1 instance of each local variable.
- **Local static variables**:
  - def: variable declared inside function with `static` attribute.
  - virtual memory contains exactly 1 instance of any local static variable.

# Shared variable analysis

See `code/24.../sharing.c`

# Improper synchronization

See `code/24.../badcnt.c`

- **C code for thread x**:

```c
void *thread(void *vargp)
{
    long i, niters = *((long *)vargp);
    for (i = 0; i < niters; i++)
        cnt++;
    return NULL;
}
```

- **Assembly code for thread x**:

```asm
thread:                 ; Hi: Head
	movq	(%rdi), %rcx    ; - load 'niters' from memory
	testq	%rcx, %rcx      ; - perform 'niters & niters', discard result, set flags
	jle	.L2               ; - if 'niters <= 0', jump to .L2
	movl	$0, %edx        ; - init 'i = 0'
-----------------------
.L3:
	movq	cnt(%rip), %rax ; Li: load 'cnt'
	addq	$1, %rax        ; Ui: update 'cnt'
	movq	%rax, cnt(%rip) ; Si: store 'cnt'
-----------------------
                        ; Ti: Tail
	addq	$1, %rdx        ; - increment i
	cmpq	%rdx, %rcx      ; - perform 'niters - i', discard result, set flags
	jne	.L3               ; - if i != niters, jump back to .L3
-----------------------
.L2:                    ; return NULL
	movl	$0, %eax
	ret
```

# Process graph

- depicts the **execution state space** of concurrent threads.
- each axis corresponds to the sequential order of instructions in a thread.
- each point corresponds to a possible execution state: (L1, S2) denotes state where thread 1 has completed L1 and thread 2 has completed S2.

## Trajectory in process graph

- a sequence of legal state transitions that describes 1 possible concurrent execution.
- example: H1, L1, U1, H2, L2, S1, T1, U2, S2, T2

## Critical section & Unsafe region

- L, U, S form a **critical section** with respect to the shared variable `cnt`.
- instructions in critical sections should not be interleaved.
- set of states where such interleaving occurs form **unsafe regions**
- a trajectory is **safe** if it does not enter any unsafe region.
- a trajectory is correct (wrt. `cnt`) if it is safe.

# Enforcing mutual exclusion

- Q: how to guarantee a safe trajectory?
- A: **synchronize** the execution of the threads so that they can never have an unsafe trajectory
  - i.e., guarantee **mutual exclusive access** for each critical section.
- **classic solution**:
  - semaphores
- **other approaches**:
  - mutex and condition variables (Pthreads)
  - monitors (Java)

# Semaphore

- a non-negative global integer synchronization variable.
- manipulated by the P and V operations.
- `P(s)`:
  - if s is nonzero, decrement s by 1 and return immediately.
    - test and decrement operations occur atomically.
  - if s is zero, suspend the thread until s becomes nonzero and the thread is restarted by a V operation.
  - after restarting, decrement s and return control to the caller.
- `V(s)`:
  - increment s by 1.
    - increment operation occurs atomically.
  - if there are any threads blocked in a P operation waiting for s to become nonzero, then restart exactly one of those threads, which then completes its P operation by decrementing s.
- **semaphore invariant**: `s >= 0`

# C semaphore operations

- `Pthreads` functions:

```c
#include <semaphore.h>

int sem_init(sem_t *s, 0, unsigned int val); // s = val
int sem_wait(sem_t *s); // P(s)
int sem_post(sem_t *s); // V(s)
```

- `CSAPP` wrapper functions:

```c
#include "csapp.h"

void P(sem_t *s); // wrapper function for sem_wait
void V(sem_t *s); // wrapper function for sem_post
```

# Using semaphores for mutual exclusion

## Basic idea

- associate a unique **semaphore mutex**, initially 1, with each shared variable (or related set of shared variables).
- surround corresponding critical sections with `P(mutex)` and `V(mutex)` operations.

## Terminology

- **binary semaphore**: semaphore whose value is always 0 or 1.
- **mutex**: binary semaphore used for mutual exclusion.
  - P operation: **locking** the mutex.
  - V operation: **releasing** the mutex.
  - holding a mutex: locked and not yet released.
- **counting semaphore**: used as a counter for a set of available resources.

# Proper synchronization

See `code/24.../goodcnt.c`

# Why mutex works

- provide mutually exclusive access to shared variable by surrounding critical section with P and V operations on semaphore s (initially set to 1).
- semaphore invariant creates a **forbidden region** that encloses the unsafe region and that cannot be entered by any trajectory.
