# Performance reality

- there's more to performance than asymptotic complexity.
- constant factors matter too -> must optimize at multiple levels (algorithm, data representations, procedures, loops).

# Optimizing compilers

- **provide efficient mapping of program to machine**:
  - register allocation.
  - code selection and ordering (scheduling).
  - dead code elimination.
  - eliminating minor inefficiencies.
- **don't (usually) improve asymptotic complexity**:
  - up to programmer to select best algorithm.
- **have difficulty optimizing "optimization blockers"**:
  - potential memory aliasing.
  - potential procedure side-effects.

## Limitations of optimizing compilers

- **operate under fundamental constraint**
  - must not change program behavior -> reject optimization even if that only affects behavior in highly unusual conditions.
- **behavior that may be obvious to programmer can be obfuscated by languages and coding styles**
  - e.g., actual data ranges may be more limited than variable types used.
- **most analysis is performed only within procedures**
  - whole-program analysis is too expensive in most cases.
  - newer versions of GCC do inter-procedure analysis within individual files, but not between code in different files.
- **most analysis is based only on static information**
  - compiler has difficulty anticipating run-time inputs.
- **when in double, the compiler must be conservative**.

# Generally useful optimizations

## Code motion

- reduce frequency a computation is performed:
  - if it always produce the same result.
  - especially moving code out of loop.

```c
void set_row(double *a, double *b, long i, long n)
{
    long j;
    for (j = 0; j < n; j++)
        a[n*i+j] = b[j];
}

// optimize
void set_row(double *a, double *b, long i, long n)
{
    long j;
    int ni = n*i;
    for (j = 0; j < n; j++)
        a[ni+j] = b[j];
}
```

- compiler-generated code motion (`-O1`)

```c
void set_row(double *a, double *b, long i, long n)
{
    long j;
    int ni = n*i;
    double *rowp = a + ni;
    for (j = 0; j < n; j++)
        *rowp++ = b[j];
}
```

```asm
set_row:
    testq %rcx, %rcx                # test n
    jle .L1                         # if n <= 0, goto done
    imulq %rcx, %rdx                # ni = n*i
    leaq (%rdi,%rdx,8), %rdx        # rowp = a + ni*8
    movl $0, %eax                   # j = 0
.L3:                            # loop:
    movsd (%rsi,%rax,8), %xmm0      # t = b[j]
    movsd %xmm0, (%rdx,%rax,8)      # M[a+ni*8 + j*8] = t
    addq $1, %rax                   # j++
    cmpq %rcx, %rax                 # compare j to n
    jne .L3                         # if j != n, goto loop
.L1:                            # done:
    rep ; ret
```

## Reduction in strength

- replace costly operation with simpler one.
- shift, add instead of multiply or divide.
  (e.g., `16*x` <-> `x << 4`)
- recognize sequence of products:

```c
for (i = 0; i < n; i++) {
    int ni = n*i;
    for (j = 0; j < n; j++)
        a[ni + j] = b[j];
}

// optimize
int ni = 0;
for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++)
        a[ni + j] = b[j];
    ni += n;
}
```

## Share common subexpressions

- reuse portions of expressions.
- GCC will do this with `-O1`.

```c
/* Sum neighbors of (i, j). n is number of elements in a row. */
up = val[(i-1)*n + j];
down = val[(i+1)*n + j];
left = val[i*n + j-1];
right = val[i*n + j+1];
sum = up + down + left + right;

// Assembly
// . 3 multiplications: i*n, (i-1)*n, (i+1)*n
leaq 1(%rsi), %rax      # i+1
leaq -1(%rsi), %r8      # i-1
imulq %rcx, %rsi        # i*n
imulq %rcx, %rax        # (i+1)*n
imulq %rcx, %r8         # (i-1)*n
addq %rdx, %rsi         # i*n+j
addq %rdx, %rax         # (i+1)*n+j
addq %rdx, %r8          # (i-1)*n+j
```

```c
long inj = i*n + j;
up = val[inj - n];
down = val[inj + n];
left = val[inj - 1];
right = val[inj + 1];
sum = up + down + left + right;

// Assembly
// . 1 multiplication: i*n
imulq %rcx, %rsi        # i*n
addq %rdx, %rsi         # i*n+j
movq %rsi, %rax         # i*n+j
subq %rcx, %rax         # i*n+j-n
leaq (%rsi,%rcx), %rcx  # i*n+j+n
```

# Optimization blocker 1: procedure calls

```c
// O(n)
size_t strlen(const char *s) {
    size_t length = 0;
    while (*s != '\0') {
        s++;
        length++;
    }
    return length;
}

// O(n^2)
void lower(char *s) {
    size_t i;
    for (i = 0; i < strlen(s); i++)
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] -= ('A' - 'a');
}

// goto form
void lower(char *s) {
    size_t i;
    if (i >= strlen(s))
        goto done;

loop:
    if (s[i] >= 'A' && s[i] <= 'Z')
        s[i] -= ('A' - 'a');
    i++;
    if (i < strlen(s))
        goto loop;
done:
}
```

- **optimize**: code motion
  - move `strlen()` call out of the loop, since result doesn't change across iterations.

```c
// O(n)
void lower(char *s) {
    size_t i;
    size_t len = strlen(s);
    for (i = 0; i < len; i++)
        if (s[i] >= 'A' && s[i] <= 'Z')
            s[i] -= ('A' - 'a');
}
```

- **why compiler couldn't move `strlen` out**:
  - procedure may have side effects.
  - function may not return the same value for given arguments:

- **remedies**:
  - use inline functions: GCC does this with `-O1` (within single file).
  - do your own code motion.

# Optimization blocker 2: memory aliasing

```c
/* sum rows of n x n matrix a and store in vector b. */
void sum_rows1(double *a, double *b, long n) {
  long i, j;
  for (i = 0; i < n; i++) {
    b[i] = 0;
    for (j = 0; j < n; j++)
      b[i] += a[i*n + j];
  }
}

// Assembly
sum_rows1:
	testq	%rdx, %rdx
	jle	.L1                       # n <= 0 -> exit
	leaq	0(,%rdx,8), %r9         # row_size_in_bytes = 8*n
	movl	$0, %r8d                # i = 0
	xorpd	%xmm1, %xmm1            # %xmm1 = 0.0
.L5:
	movq	%r8, %rax               # clone i
	movsd	%xmm1, (%rsi,%r8,8)     # b[i] = 0.0
	leaq	(%r9,%rdi), %rcx        # row_end = a + 8*n
-------------------------------------------------
.L4: // inner loop
	movsd	(%rsi,%rax,8), %xmm0    # FP load: %xmm0 = b[i]
	addsd	(%rdi), %xmm0           # FP add: %xmm0 += *a
	movsd	%xmm0, (%rsi,%rax,8)    # FP store: b[i] = %xmm0
	addq	$8, %rdi                # a++
	cmpq	%rcx, %rdi
	jne	.L4                       # a != row_end -> continue inner loop
-------------------------------------------------
	addq	$1, %r8                 # i++
	cmpq	%rdx, %r8
	je	.L1                       # i == n -> exit
	movq	%rcx, %rdi              # a = row_end
	jmp	.L5                       # continue outer loop
.L1:
	rep; ret
```

- code updates b[i] on every iteration (load, update, store).
- **why compiler couldn't optimize like this**:

```c
void sum_rows_wrong(double *a, double *b, long n) {
  long i, j;
  for (i = 0; i < n; i++) {
    double row_sum = 0; // temp
    for (j = 0; j < n; j++)
      row_sum += a[i*n + j];
    b[i] = row_sum; // access b[i] once
  }
}
```

- **reason**:
  - what if b points to same memory addresses as A (memory aliasing)

```c
double A[9] = { 0, 1, 2, 4, 8, 16, 32, 64, 128}; // 3 x 3
double B[3] = A + 3;
```

# MORE ...

TODO
