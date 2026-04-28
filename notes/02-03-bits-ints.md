# Representing information

- each bit is 0 or 1
- 1 byte = 8 bits
- bin, dec, hex

# Bit manipulations

## Boolean algebra

- and, or, not, xor
- operate on bit vectors: applied bitwise

## Representing sets

- **Representation:**
  - Width w bit vector represents subset {0, ..., w-1}
  - a[i] = 1 if i belongs to A
  - 01101001 <-> {0, 3, 5, 6}
  - 01010101 <-> {0, 2, 4, 6}

- **Operations:**
  - &: intersection
    - 01101001 & 01010101 = 01000001 <-> {0, 6}
  - |: union
    - 01101001 | 01010101 = 01111101 <-> {0, 2, 3, 4, 5, 6}
  - ^: symmetric difference
    - 01101001 ^ 01010101 = 00111100 <-> {2, 3, 4, 5}
  - ~: complement
    - ~01010101 = 10101010 <-> {1, 3, 5, 7}

## Logic operations

- &&, ||, !
- view 0 as False, anything non-zero as True
- always returns 0 or 1
- **early termination**

## Shift operations

- **Left shift:** x << y
  - shift bit-vector x left y positions, throws away extra bits on the left, fill with 0's on the right

- **Right shift:** x >> y
  - shift bit vector x right y positions, throws away extra bits on the right
  - **Logical shift:** fill with 0's on the left
  - **Arithmetic shift:** replicate the most significant bit on the left

- **Undefined behavior:** shift amount < 0 or >= word_size

## Conversion between base

- **bin -> dec:**

```
sum(d[i] * 2^i)
```

- **dec -> bin:** repeatedly divide by 2 and collect remainders (represent LSB -> MSB)

```python
def decimal_to_binary(n):
    if n == 0:
        return "0"

    bits: list[int] = []
    while n > 0:
        remainder = n % 2
        bits.append(remainder)
        n = n // 2

    reverse(bits)
    return "".join(bits)
```

- **dec -> hex** and **hex -> dec**: similar

# Integers

## Encoding integers

- **Unsigned:**

```
B2U(X) = sum(x[i] * 2^i for i in [0..w-1])
```

- **Two's complement:**

```
B2T(X) = -x[w-1]*2^(w-1) + sum(x[i] * 2^i for i in [0..w-2])

. x[w-1] is the sign bit: 0 for nonnegative, 1 for negative
```

## Value range

- **Unsigned:**

```
Umin = 0 = 0b0...0 = 0x0000
Umax = 2^w - 1 = 0b1...1 = 0xffff
(hex with w=4)
```

- **Two's complement:**

```
Tmin = -2^(w-1) = 0b10...0 = 0x8000
Tmax = 2^(w-1) - 1 = 0b01...1 = 0x7fff
-1 = 0b11...1 = 0xffff
(hex with w=4)
```

- **Observations:**

```
|Tmin| = Tmax + 1 (asymmetric)
Umax = 2*Tmax + 1
```

- **In C:** check <limits.h>

## Invert mappings

- U2B(x): bit pattern for unsigned integers
- T2B(x): bit pattern for two's comp integers

## Mapping between signed & unsigned

- Keep bit representations and reinterpret

```
ux = T2U(x) = B2U(T2B(x))
x = U2T(ux) = B2T(U2B(ux))
```

- **Mapping diagram:**

```
Tmin     -1 0     Tmax
|________*__+_____+_____________
            +     +    |        *
            0     Tmax Tmax+1   Umax

```

- **2's comp -> unsigned:**

```
0 <= x <= Tmax: ux = x
Tmin <= x < 0: ux = x + 2^w
```

- **unsigned -> 2's comp:**

```
0 <= ux <= Tmax: x = ux
Tmax < ux <= Umax: x = ux - 2^w
```

## Sign & unsigned in C

### Constants in C

- by default are considered signed.
- unsigned if have `U` suffix.

### Casting

- **Explicit:**

```c
(int) ux;
(unsigned) ty;
```

- **Implicit:** via assignments and procedure calls

```c
tx = ux;
uy = ty;
```

- **Surprises:** expression has both signed and unsigned -> signed values are implicitly cast to unsigned

## Expanding

Convert w-bit integer X to (w+k)-bit integer with **the same value**:

- signed: perform **sign extension**

```
make k copies of the sign bit:
. X = [x[w-1], ... x[0]]
. X' = [x[w-1], ..., x[w-1], ... x[0]]
```

- unsigned: add 0's

## Truncating

Convert w-bit integer to k-bit integer (k < w):

- keep k least significant bits, then re-interpret result.
- unsigned: `X' = X mod 2^k`
- signed: interpret remaining bits as 2's comp.

## Arithmetic

- **Unsigned addition:**

```
0 <= u, v < 2^w
-> 0 <= u + v < 2^(w+1)

true sum requires w+1 bits -> drop off w-th bit:
. UAddw(u, v) = (u + v) mod 2^w
```

- **2's comp addition:**

```
-2^(w-1) <= u, v <= 2^(w-1) - 1
-> -2^w <= u + v <= 2^w - 2

true sum requires w+1 bits -> drop off w-th bit and treat remaining bits as 2's comp:
. TAddw(u, v) = s         if Tminw <= s <= Tmaxw
. TAddw(u, v) = s - 2^w   if s > Tmaxw   (positive overflow)
. TAddw(u, v) = s + 2^w   if s < Tminw   (negative overflow)
```

- `TAdd` and `UAdd` have identical bit-level behavior:

```c
int s, t, u, v;
s = (int) ((unsigned) u + (unsigned) v);
t = u + v;
// => s == t
```

- **Unsigned multiplication:**

```
0 <= u, v <= 2^w - 1
-> 0 <= u.v <= 2^(2*w) - 2^(w+1) + 1

true product requires up to 2*w bits
-> ignore high order w bits
. UMultw(u, v) = u*v mod 2
```

- **Signed multiplication:**

```
-2^(w-1) <= u, v <= 2^(w-1) - 1
min product (negative): -2^(w-1) * (2^(w-1) - 1) = -2^(2*w-2) + 2^(w-1)
max product (positive): (-2^(w-1))^2 = 2^(2*w - 2)

true product requires up to 2*w bits
-> ignore high order w bits, some of which are different for signed and unsigned multiplication; the lower bits are the same:
. TMultw(u, v) = U2T(UMultw(u, v))
```

- **Power-of-2 multiply with shift**:

```
u << k <-> u * 2^k  (both signed and unsigned)
```

- **Unsigned power-of-2 divide with shift**:

```
u >> k <-> floor(u / 2^k)
. use logical shift
```

### Arithmetic: Summary

- Addition:
  - unsigned/signed: normal addition followed by truncate, same operation on bit level
  - unsigned: addition mod 2^w
    - mathematical addition + possible subtraction of 2^w
  - signed:
    - mathematical addition + possible addition or subtraction of 2^w

- Multiplication:
  - unsigned/signed: normal multiplication followed by truncate, same operation on bit level
  - unsigned: multiplication mod 2^w
  - signed: perform unsigned multiplication then interpret the resulting bits as signed

## When to use unsigned

- modular arithmetic
- using bits to represent sets (logical right shift)

### Example mistakes

```c
unsigned i;
for (i = cnt-2; i >= 0; i--)
  a[i] += a[i+1];
// -> i can wrap around to Umax
// -> probably will cause memory error

// ===== FIX =====
unsigned i;
for (i = cnt-2; i < cnt; i--)
  a[i] += a[i+1];
// when i becomes -1 and wrap around to Umax, i < cnt becomes false
```

```c
#define DELTA sizeof(int)
int i;
for (i = CNT; i-DELTA >= 0; i-= DELTA)
  ...
// sizeof() returns size_t, an unsigned integer type
// -> i is cast to unsigned when calculating i-DELTA
// -> i-DELTA cannot become negative (wrap around)
// -> infinite loop or memory error
```

# Representation in memory, pointers, strings

## Byte-oriented memory organization

- Conceptually, envision memory as a large array of bytes. Address is like index into that array.
- Programs refer to data by address. System provides private address space to each process.

## Machine words

- Each computer has a given **word size**
  - 32 bits (4 bytes): 4GB (2^32 bytes) addressable memory
  - 64 bits (8 bytes): 18EB (exabytes) addressable memory

## Word-oriented memory organization

- Addresses specify byte locations:
  - address of 1st byte in word
  - address of successive words differ by 4 or 8 bytes

## Byte ordering

- **Big Endian:** least significant byte has highest address
- **Little Endian:** least significant byte has lowest address
- Example:

```
x = 0x01234567 (4 bytes)
&x is 0x100

Big Endian:
0x100 0x101 0x102 0x103
01    23    45    67

Little Endian:
0x100 0x101 0x102 0x103
67    45    23    01
```

## Strings in C

- represented by array of characters
- ASCII characters:
  . digit i has code 0x30+i
- NULL terminated: final character = 0 ('\0')
