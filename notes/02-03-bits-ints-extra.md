# Integer C puzzles

```
. x < 0 -> ((x*2) < 0)
-> False
x*2 can become positive if mathematical product < Tmin (negative overflow)

. ux >= 0
-> True

. x & 7 == 7 -> (x << 30) < 0
-> True
x & 7 == 7 <-> last 3 bits of x is 111 (b...b111)
w = 32 (4 bytes) -> x << 30 == (110...0) < 0

. ux > -1
-> False
-1 is cast to unsigned and become Umax

. x > y -> -x < -y
-> False
y = Tmin -> -y = Tmin
x > Tmin -> min -x = -Tmax > Tmin = -y

. x * x >= 0
-> False
positive overflow (become negative) if mathematical product > Tmax

. x > 0 && y > 0 -> x + y > 0
-> False
positive overflow (become negative) if mathematical product > Tmax

. x >= 0 -> -x <= 0
-> True

. x <= 0 -> -x >= 0
-> False
-Tmin = -Tmin (positive overflow)

. (x | -x) >> 31 == -1
-> False
x = 0 -> (0 | -0) >> 31 = 0

. (x | -x) >> 31 == -1 for x != 0:
-> True
. either x or -x has MSB = 1
. performing bitwise OR -> result has MSB = 1
. arithmetic right shift by 31 expand the MSB to all positions (w = 32) -> bit pattern with all 1's is -1 in 2's comp

. ux >> 3 == ux / 8
-> True (floor division by 2^k)

. x >> 3 == x / 8
-> False
. x >> 3 (arithmetic shift) performs floor division
. x / 8 performs truncation towards 0
. only True if x is multiple of 8

. x & (x - 1) != 0
-> False
. x = 2^k -> x & (x - 1) == 0
    H 1 0 ... 0     <- x
&   H 0 1 ... 1     <- x - 1
    H 0 0 ... 0     <- discards the lowest order '1'
. x = 2^k -> x & (x - 1) discards the only '1' bit (becomes 0)
```

# Boolean algebra

```
a^b = a & ~b | ~a & b
```

# Binary number property

- **Claim:**

```
1 + 1 + 2 + 4 + 8 + ... + 2^(w-1) = 2^w
1 + sum(2^i for i in [0..w-1]) = 2^w
```

- **Proof:** by induction

```
. w = 0 -> 1 = 2^0
. assume the claim is true for w
  1 + 1 + 2 + 4 + 8 + ... + 2^(w-1) = 2^w
. for w + 1
  1 + 1 + 2 + 4 + 8 + ... + 2^(w-1) + 2^w
  = 2^w + 2^w = 2^(w+1)
-> true for every w
```

# Negation: complement & increment

- **Claim:**

```
~x + 1 == -x
for 2's comp
```

- **Proof:**

```
x + ~x = -1 (bit pattern of all 1's)
-1 + 1 = 0 (discard w-th bit)
-> x + ~x + 1 = 0
-> ~x + 1 = -x
```

# Math properties of UAdd

Modular addition forms an **Abelian Group**

- **Closed** under addition:

```
0 <= UAddw(u, v) <= Tmax
```

- **Commutative:**

```
UAddw(u, v) = UAddw(v, u)
```

- **Associative:**

```
UAddw(t, UAddw(u, v)) = UAddw(UAddw(t, u), v)
```

- 0 is **additive identity**:

```
UAddw(u, 0) = u
```

- Every element has an **additive inverse**:

```
Let UCompw(u) = 2^w - u
then UAddw(u, UCompw(u)) = 0
```

# Math properties of TAdd

- **Isomorphic group** with UAdd
  - both have identical bit patterns

```
TAddw(u, v) = U2T(UAddw(T2U(u), T2U(v)))
```

- Closed, Commutative, Associative, 0 is additive identity
- Additive inverse:

```
TCompw(u) = -u    if u != Tmin
TCompw(u) = u     if u == Tmin
```

# Math properties of UMult

- **Closed** under addition:

```
0 <= UMultw(u, v) <= Tmax
```

- **Commutative:**

```
UMultw(u, v) = UMultw(v, u)
```

- **Associative:**

```
UMultw(t, UMultw(u, v)) = UMultw(UMultw(t, u), v)
```

- 1 is **multiplicative identity**:

```
UMultw(u, 1) = u
```

- Multiplication **distributes** over addition:

```
UMultw(t, UAddw(u, v)) = UAddw(UMultw(t, u), UMultw(t, v))
```

# Signed power-of-2 divide with shift

- `x >> k` gives `floor(x / 2^k)`
  - use arithmetic shift
  - rounds wrong direction if x < 0
- for x < 0, we want ceil(x / 2^k)
  - compute: `floor((x + 2^k - 1) / 2^k)`
  - in C: `(x + (1 << k) - 1) >> k`
  - bias dividend towards 0

- **Proof:**

```
Let x = q * 2^k + r, where 0 <= r < 2^k

===
Case 1: r == 0 (x is a multiple of 2^k)

. without bias:
x / 2^k = q * 2^k / 2^k = q

. with bias:
(x + 2^k - 1) / 2^k
= (q * 2^k + r + 2^k - 1) / 2^k
= q + (2^k - 1) / 2^k
(2^k - 1) / 2^k < 1
-> floor(q + (2^k - 1) / 2^k) = q (biasing has no effect)

===
Case 2: r > 0 (x is not a multiple of 2^k)

. without bias:
x / 2^k
= (q * 2^k + r) / 2^k
= q + r / 2^k
1 <= r < 2^k -> 0 < r / 2^k < 1
-> floor(q + r / 2^k) = q (result rounded down)
-> we need q + 1

. with bias:
(x + 2^k - 1) / 2^k
= (q * 2^k + r + 2^k - 1) / 2^k
= ((q + 1)*2^k + (r - 1)) / 2^k
= q + 1 + (r - 1) / 2^k
1 <= r < 2^k
-> 0 <= (r - 1) < 2^k - 1
-> 0 < (r - 1) / 2^k < 1
-> floor(q + 1 + (r - 1) / 2^k) = q + 1 (desired result)
```

# Reading byte-reversed listings

- **Disassembly:** Text representation of binary machine code
- Example fragment:

```
Address     Instruction Code        Assembly
8048366     81 c3 ab 12 00 00       add $0x12ab, %ebx
            ---------------->
            higher address
```

- "Deciphering" number (little endian):
  - value: 0x12ab
  - pad to 32 bits: 0x000012ab
  - split into bytes: 00 00 12 ab
  - reverse: ab 12 00 00

# Compiled code

- multiplying by constant <-> shift/add

```c
x*12
= (x * 3) * 4
= (x + x * 2) * 2^2
= (x + x*2) << 2
```

- unsigned division <-> logical right shift

```c
x / 8
= x / 2^3
= x >> 3
```

- signed division

```c
long idiv8(long x)
{
    return x/8;
}

if x < 0
    x += 7;
return x >> 3; # arithmetic shift

/* Compute x / 2^k
x >= 0:
result = floor(x / 2^3) = x >> 3

x < 0:
result = floor((x + 2^k - 1) / 2^k)
       = floor((x + 8 - 1) / 2^3)
       = (x + 7) >> 3
*/
```

- In Java:
  - logical right shift is `>>>`
  - arithmetic right shift is `>>`

# Code security examples

## 1. FreeBSD's `getpeername`

```c
/* Kernel memory region holding user-accessible data */
#define KSIZE 1024
char kbuf[KSIZE];

/* Copy at most 'maxlen' bytes from kernel region to user buffer */
int copy_from_kernel(void *user_dest, int maxlen) {
    /* Byte count 'len' is minimum of buffer size and 'maxlen' */
    int len = KSIZE < maxlen ? KSIZE : maxlen;
    memcpy(user_dest, kbuf, len);
    return len;
}
```

- Typical usage:

```c
#define MSIZE 528

void getstuff() {
    char mybuf[MSIZE];
    copy_from_kernel(mybuf, MSIZE);
    // ....
}
```

- **Malicious usage:**

```c
#define MSIZE 528

void getstuff() {
    char mybuf[MSIZE];
    copy_from_kernel(mybuf, -MSIZE);
    // ....
}
```

- **Explanation:**

```c
int copy_from_kernel(void *user_dest, int maxlen) {
    int len = KSIZE < maxlen ? KSIZE : maxlen;
    memcpy(user_dest, kbuf, len);
    return len;
}

/*
- Declaration of library function 'memcpy' is:
void *memcpy(void *dest, void *src, size_t n);

- User pass maxlen < 0:
  . int len = KSIZE < maxlen ? KSIZE : maxlen;
    -> len = maxlen since KSIZE > 0
  . memcpy(user_dest, kbuf, len);
    -> 'len' is convert to unsigned (size_t is unsigned) and overflow to a large positive number (Umax - maxlen)
    -> 'memcpy' continues reading through kernel's private memory
    -> disclose sensitive data, crash, bypass security protection, ...
*/
```

- **Fix:**

```c
int copy_from_kernel(void *user_dest, int maxlen) {
    // Explicit check
    if (maxlen < 0)
        return -1;

    int len = KSIZE < maxlen ? KSIZE : maxlen;
    memcpy(user_dest, kbuf, len);
    return len;
}
```

## 2. SUN XDR library

```c
void *copy_elements(void *ele_src[], int ele_cnt, size_t ele_size) {
    // Allocate buffer for ele_cnt objects, each of size ele_size bytes
    // and copy from locations designated by ele_src
    void *result = malloc(ele_cnt * ele_size);
    if (result == NULL)
        return NULL;

    void *next = result;
    int i;
    for (i = 0; i < ele_cnt; i++) {
        memcpy(next, ele_src[i], ele_size);
        next += ele_size;
    }

    return result;
}
```

- **Vulnerability:**

```c
malloc(ele_cnt * ele_size);

/*
What if:
. ele_cnt = 2^20 +1
. ele_size = 2^12 = 4096
-> mathematical product = 2^32 + 4096

If size_t is 4 bytes (32 bits), maximum value is 2^32 - 1
-> result overflow and wraps around: (2^32 + 4096) mod 2^32 = 4096
-> malloc only allocates 4096 bytes, which fits 1 element, but the for loop assume enough space for 1 + 2^20 elements.

=> Result:
- The loop successfully copy the 1st element
- Starting from 2nd element, memcpy begins overwriting heap memory following the allocated buffer.
*/
```

- **Fix:**

```c
void *copy_elements(void *ele_src[], int ele_cnt, size_t ele_size) {
    // Prevent negative ele_cnt
    if (ele_count < 0)
        return NULL
    
    // Check for overflow
    if (ele_size > 0 && ele_cnt > (SIZE_MAX / ele_size))
        // . SIZE_MAX is the maximum value for object of size size_t
        // . check ele_size > 0 to avoid division by 0
        return NULL;

    void *result = malloc(ele_cnt * ele_size);
    if (result == NULL)
        return NULL;

    void *next = result;
    int i;
    for (i = 0; i < ele_cnt; i++) {
        memcpy(next, ele_src[i], ele_size);
        next += ele_size;
    }

    return result;
}
```