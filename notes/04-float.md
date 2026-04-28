# Fractional binary number

## Representation

- bits to the right of binary point represent fractional powers of 2
- represent rational number:

```
sum(b[k]*2^k for k in [-j..i])
```

## Examples

```
5 3/4 = 2^2 + 2^0 + 1/2^1 + 1/2^2
-> representation: 101.11

2 7/8 = 2^1 + 4/8 + 2/8 + 1/8 = 2^1 + 1/2^1 + 1/2^2 + 1/2^3
-> representation: 10.111

5 3/4 = 2 * (2 7/8)
```

## Observations:

- divide by 2 by shifting right (binary point move left)
- multiply by 2 by shifting left (binary point move right)
- numbers of form: 0.1...1 are just below 1.0
  - 1/2 + 1/4 + ... + 1/2^i -> 1.0
  - use notation `1.0 - ε`

## Limitations

- Can only exactly represent numbers of form x/2^k
  - other rational numbers have repeating bit representations
  - increase the precision by using more bits

```
1/3 -> 0.0101010101[01]...
1/5 -> 0.001100110011[0011]...
1/10 -> 0.0001100110011[0011]...
```

- Just 1 setting of binary point within w bits:
  - limited range of numbers

# IEEE floating point

Supported by all major CPUs

## Representation

- Numerical form: `(-1)^s * M * 2^E`
  - sign bit `s`
  - significant `M`: normally a fractional value in range `[1.0, 2.0)`
  - exponent `E`: weighs value by power of 2

- Encoding:
  - MSB is sign bit s
  - `exp` field encodes E (but not equal to E)
  - `frac` field encodes M (but not equal to M)

```
. single precision: 32 bits
| s     | exp    | frac    |
  1 bit   8 bits   23 bits

. double precision: 64 bits
| s     | exp     | frac    |
  1 bit   11 bits   52 bits
```

## Normalized values

- Condition: `exp != 0...0 AND exp != 1...1`
- Exponent coded as a biased value: `E = Exp - Bias`
  - `Exp`: unsigned value of `exp` field
  - `Bias`: `2^(k-1) - 1`, where k is number of exponent bits
    - single precision: 2^(8-1) - 1 = 127
    - double precision: 2^(11-1) - 1 = 1023
- Significant coded with implied leading 1: `M = 1.x...x`
  - `x...x`: bit of `frac` field
  - minimum when frac=0...0 (M = 1.0)
  - maximum when frac=1...1 (M = 2.0 - ε)
  - get extra leading bit for "free"
- Example:

```c
float f = 15213.0;
15213 = 11101101101101
      = 1.1101101101101 * 2^13

// Significand:
M    =   1.1101101101101
frac =     11011011011010000000000

// Exponent:
E = 13
Bias = 127
Exp = E + Bias = 140 = 10001100

// Result:
0 10001100 11011011011010000000000
s exp      frac
```

- Logarithmic spacing: numbers get further apart as they get larger

## Denormalized values

- Condition: `exp = 0...0`
- Exponent value: `E = 1 - Bias`
- Significand coded with implied leading 0: M = 0.x...x
  - `x...x`: bits of `frac`

### Cases

- `exp = 0...0, frac = 0...0`:
  - represents 0
  - +0 and -0 are distinct values (different sign bits)
- `exp = 0...0, frac != 0...0`:
  - represents numbers closest to 0
  - **equispaced:** all denormalized numbers share the same exponent `1-Bias` -> distance between any 2 adjacent numbers is determined by their fractions -> linearly spaced

## Special values

- Condition: `exp = 1...1`

### Cases

- `exp = 1...1, frac = 0...0`:
  - represents inf -> for operation that overflows
  - the sign bit `s` determined -inf or +inf
  - examples:
    - 1.0/0.0 = -1.0/-0.0 = +inf
    - 1.0/-0.0 = -inf
- `exp = 1...1, frac != 0...0`:
  - not-a-number (NaN) -> no numeric value can be determined
  - examples: sqrt(-1), inf - inf, `inf * 0`

## Diagram

```
    -inf  -norm       -denorm      +denorm   +norm      +inf
    '___'____________'________'_'_'________'____________'___'
'___'                                                       '___'
 NaN                         -0  +0                          NaN
```

## Distribution of values

- Denser towards 0
  - denormalized values are equally spaced
  - normalized values get further apart as they get larger

## Special properties

- FP 0 is the same as Integer 0
  - bit pattern: 0...0
- Can **almost** use unsigned integer **comparison**
  - must first compare sign bit `s`
    - positive numbers: bit patterns increase as values increase
    - negative numbers: bit patterns increase as values move further from 0
  - must consider -0 = +0 = 0
  - NaN is problematic: any comparison is False, even NaN == NaN
  - otherwise OK: denormalized vs. normalized vs. inf

# Floating point operations

## Basic idea

- Compute mathematical result
- Make it fit into desired position
  - Possibly overflow if exponent too large
  - Possibly round to fit into `frac`

## Rounding

### Rounding modes

- towards 0
- round down (towards -inf)
- round up (towards inf)
- **nearest even (default):**
  - when exactly halfway between 2 possible values, round so that the least significant digit is even
  - otherwise round to nearer value

### Rounding binary numbers

- `even`: when LSB is 0
- `halfway`:
  - when bits to right of rounding position is `10...0`
  - `0___1___`: less than halfway
  - `10___1___`: greater than halfway
- Example: round to nearest 1/4 (2 bits right to binary point)

```
(note: 1/2 denotes halfway between 2 values, not the value 1/2)

Value   Binary      Rounded   Action        Rounded Value
2 3/32  10.00|011   10.00     (<1/2—down)   2
2 3/16  10.00|110   10.01     (>1/2—up)     2 1/4
2 7/8   10.11|100   11.00     ( 1/2—up)     3
2 5/8   10.10|100   10.10     ( 1/2—down)   2 1/2
```

## FP multiplication

- `(-1)^s1 * M1 * 2^E1 * (-1)^s2 * M2 * 2^E2`
- Math result: `(-1)^s * M * 2^E`
  - `s`: s1 ^ s2 (result is negative if signs are different)
  - `M`: `M1 * M2`
  - `E`: E1 + E2
- Fix:
  - If M >= 2, shift M right, increment E
  - If E out of range, overflow
  - Round M to fit `frac` precision: use **nearest even**

## FP addition

- `(-1)^s1 * M1 * 2^E1 + (-1)^s2 * M2 * 2^E2`
  - assume E1 > E2
- Math result: `(-1)^s * M * 2^E`
  - `s`, `M`: result of signed align & add
  - `E`: E1

```
- Get binary points lined up:

                 '<-- E1-E2 -->'
  | (-1)^s1 * M1 |
+              | (-1)^s2 * M2  |
________________________________
  |       (-1)^s * M           |
```

- Fix:
  - If M >= 2, shift M right, increment E
  - If M < 1, shift M left, decrement E
  - Overflow if E out of range
  - Round M to fit `frac` precision: use **nearest even**

### Math properties of FP Add

- **Closed** under addition
  - **But** may generate inf or NaN
- **Commutative**
- **NOT Associative**
  - due to overflow and inexactness of rounding
  - example:
    - (3.14 + 1e10) - 1e10 = 0
    - 3.14 + (1e10 - 1e10) = 3.14
- 0 is **additive identity**
- Every element has an **additive inverse**
  - except for inf & NaN
- **Monotonicity**: a >= b -> a+c >= b+c
  - except for inf & NaN

### Math properties of FP Mult

- **Closed** under multiplication
  - **But** may generate inf or NaN
- **Commutative**
- **NOT Associative**
  - due to overflow and inexactness of rounding
  - example:
    - `(1e20 * 1e20) * 1e-20 = inf`
    - `1e20 * (1e20 * 1e-20) = 1e20`
- 1 is **multiplicative identity**
- **NOT Distributes over addition**
  - due to overflow and inexactness of rounding
  - example:
    - `1e20 * (1e20 - 1e20) = 0.0`
    - `1e20 * 1e20 - 1e20 * 1e20 = NaN`
- **Monotonicity**: `a >= b & c >= 0 -> a*c >= b*c`
  - except for inf & NaN

# Floating point in C

- C guarantees 2 levels:
  - `float`: single precision
  - `double`: double precision

## Conversion & Casting

- **Casting** between `int`, `float`, `double` **changes bit representation**
- **double/float -> int:**
  - truncates fractional part
  - like rounding towards 0
  - not defined when out of range or NaN; generally set to Tmin
- **int -> double:**
  - exact conversion, as long as `int` has <= 53 bit word size
  - why:
    - double precision (64-bit) has a 52-bit fraction field
    - including the implied leading bit, a double effectively has 53 bits of precision
- **int -> float:**
  - will round according to rounding mode (use **nearest even** by default)
- **float -> double:**
  - always exact
- **double -> float:**
  - can results in precision loss
  - overflow: results in +inf or -inf if value is too large

# Puzzles

```c
int x = ...;
float f = ...;
double d = ...;
// assume neither d nor f is NaN
```

```
. x == (int) (float) x
-> False
. x can be rounded during (float)x

. x == (int) (double) x
-> True
. no rounding occurs during (double)x

. f = (float) (double) f
-> True

. d = (double) (float) d
-> False
. (float) d does not preserve precision

. f == -(-f);
-> True
. unary minus (-) just flips the sign bit

. 2/3 == 2/3.0
-> False
. 2/3 = 0 (integer division, round towards 0)
. 2/3.0 = 0.(6)... (FP division)

. d < 0.0 -> d*2 < 0.0
-> True
. Cannot compare if d is NaN

. d > f -> -f > -d
-> True

. d * d >= 0.0
-> False
. d is NaN -> d*d is NaN -> any comparison with d*d is False
. True for other cases

. (d + f) - d == f
-> False
. due to rounding: d is very large and f is very small
. d == +inf and f is finite
  (+inf + f) - (+inf) = (+inf) - (+inf) = NaN
  NaN == f is False
```
