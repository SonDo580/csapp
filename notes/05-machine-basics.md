# History

- CISC: complex instruction set computers
- RISC: reduced instruction set computers
- Intel x86 family
  - IA32 (Intel architecture, 32-bit): traditional
  - **x86-64**: standard
- AMD: advanced micro devices

# Definitions

- **Architecture** (ISA; instruction set architecture): the parts of a processor design one needs to understand or write assembly/machine code
  - example: instruction set specification, registers
- **Micro-architecture**: Implementation of the architecture
  - example: cache size, core frequency
- **Code forms:**
  - Machine code: byte-level programs that processor executes
  - Assembly code: text representation of machine code
- Example ISAs:
  - Intel: IA32, x86-64
  - ARM (Acorn RISC machine): used in almost all mobile phones

# Assembly, machine code view

```
 ___________________                     ________
|CPU                |                   | MEMORY |
|     Registers     |-----addresses---->|  code  |
| PC                |<------data------->|  data  |
|     Cond_codes    |<---instructions---|  stack |
|___________________|                   |________|
```

Programmer-visible state:

- **PC** (program counter):
  - address of next instruction
  - called "RIP" (x86-64)
- **Register file:**
  - heavily used program data
- **Condition codes:**
  - store status information about most recent arithmetic/logical operation
  - used for conditional branching
- **Memory:**
  - byte addressable array
  - code and user data
  - stack to support procedures

# C to object code

```bash
gcc -Og p1.c p2.c -o p
# -Og: use basic optimization (for debugging)
# (letter O, not digit 0)
```

- **"Pipeline"**:

```
text: C program (p1.c, p2.c)
        | compiler (gcc -Og - S)
        v
text: Assembly program (p1.s, p2.s)
        | assembler (gcc OR as)
        v
binary: Object program (p1.o, p2.o)
        | linker (gcc OR ld)
        |
        | <------------------------ static libraries (.a)
        v
binary: Executable program (p)
```

- **Compile C to assembly:**

```bash
gcc -Og -S sum.c
# generate sum.s
```

- **Assembly to object code:**

```bash
gcc -c sum.s -o sum.o
# assemble but do not link; generate sum.o
```

- **Assembler:**
  - translate `.s` into `.o`
  - binary encoding of each instruction
  - nearly-complete image of executable code
  - missing linkages between code in different files

- **Linker:**
  - resolve references between files
  - combine with static run-time libraries
  - some libraries are **dynamically linked**
    - linking occurs when program begins execution

- Why use **dynamic linking**:
  - multiple running programs share a single copy of the library
  - update the library without re-compiling user programs
  - allow programs to load features or plugins at runtime, keeping the initial executable lightweight and extensible

# Assembly characteristics

## Data types

- "integer" data of 1, 2, 4, 8 bytes
  - data values
  - addresses (untyped pointers)
- floating point data of 4, 8, 10 bytes
- code: byte sequences encoding series of instructions
- NO aggregate types such as arrays or structures, just contiguously allocated bytes in memory

## Operations

- perform arithmetic function on register or memory data
- transfer data between memory and register:
  - load data from memory into register
  - store register data into memory
- transfer control:
  - unconditional jumps to/from procedures
  - conditional branches

# Machine instruction example

- C:

```c
*dest = t;
// store value t where designated by dest
```

- Assembly:

```asm
movq %rax, (%rbx)

- move 8-byte value to memory (quad words in x86-64 parlance)
- operand:
  . t: register %rax
  . dest: register %rbx
  . *dest: memory M[%rbx]
```

- Object code:

```
0x40059e: 48 89 03

- 3-byte instruction
- instruction stored at address 0x40059e
```

# Disassembler

- tool to examine object code
- analyzes bit patterns of series of instructions
- produce approximate rendition of assembly code
- can be run on complete executable or `.o` file

```bash
objdump -d sum.o
objdump -d sum
```

- with GDB:

```bash
gdb sum
disassemble sumstore # disassemble procedure
x/14xb sumstore # examine 14 bytes starting at sumstore
```

# x86-64 integer registers

- **16 registers**: %rax, %rbx, %rcx, %rdx, %rsi, %rdi, %rsp, %rbp, %r8 -> %r15
- reference low-order 4 bytes: %eax, %ebx, %ecx, %edx, %esi, %edi, %esp, %ebp, %r8d -> %r15d
- can also reference low-order 1 or 2 bytes (backward compatibility)
- **word**:
  - byte: 8 bits
  - word: 16 bits
  - double word: 32 bits
  - quad word: 64 bits

# Moving data

`movq Source Dest`

## Operand types

- **Immediate:** integer constant
  - example: $0x400, $-533
  - encoded with 1,2,4 bytes
- **Register:** 1 of 16 integer registers
  - %rsp reserved for special use
  - others have special uses for particular instructions
- **Memory:** 8 consecutive bytes of memory at address given by register
  - example: (%rax)
  - various addressing modes

## Operand combinations

```
Source  Dest    Assembly            C
----------------------------------------------------
Imm     Reg     movq $0x4, %rax     temp = 0x4;
        Mem     movq $-147, (%rax)  *p = -147;
Reg     Reg     movq %rax, %rdx     temp2 = temp1;
        Mem     movq %rax, (%rdx)   *p = temp;
Mem     Reg     movq (%rax), %rdx   temp = *p;
```

- CANNOT do memory-memory transfer with a single instruction

## Simple memory addressing modes

- **Normal:** (R) -> Mem(Reg[R])
  - register R specifies memory address
  - pointer dereferencing in C
  - `movq (%rcx), %rax`
- **Displacement:** D(R) -> Mem(Reg[R]+D)
  - register R specifies start of memory region
  - constant D specifies offset
  - `movq 8(%rbp), %rdx`

## Complete memory addressing modes

- **General form:** `D(Rb, Ri, S)` -> `Mem[Reg[Rb] + S*Reg[Ri] + D]`
  - D: constant displacement: 1, 2, 4 bytes
  - Rb: base register: any of 16 integer registers
  - Ri: index register: any, except for %rsp
  - S: scale: 1, 2, 4, 8

- **Special cases:**
  - (Rb, Ri) -> `Mem[Reg[Rb] + Reg[Ri]]`
  - D(Rb, Ri) -> `Mem[Reg[Rb] + Reg[Ri] + D]`
  - (Rb, Ri, S) -> `Mem[Reg[Rb] + S*Reg[Ri]]`

- Address computation **example**:

```
%rdx contains 0xf000
%rcx contains 0x0100

Expression            Address
------------------------------------------------
0x8 (%rdx)            0xf000 + 0x8 = 0xf008
(%rdx, %rcx)          0xf000 + 0x100 = 0xf100
(%rdx, %rcx, 4)       0xf000 + 4*0x100 = 0xf400
0x80 (, %rdx, 2)      2*0xf000 + 0x80 = 0x1e080
```

# Arithmetic and logical operations

## Address computation instruction

- `leaq Src, Dst`
  - `Src`: address mode expression
  - Set `Dst` to address denoted by expression

### Use cases

- Compute addresses without a memory reference

```c
// Example: translation of p = &x[i];
long get_addr(long *x, long i) {
  return &x[i];
}

// Assembly
leaq (%rdi, %rsi, 8), %rax
```

- Compute arithmetic expressions of form `x + k*y`
  - k = 1, 2, 4, 8

```c
long m12(long x)
{
  return x*12;
}
// x*12 = x*3*4 = (x + x*2) << 2

// Assembly
leaq (%rdi, %rdi, 2), %rax
salq $2, %rax
```

## Some arithmetic operations

- **2-operand instructions:**
  - watch out for argument order
  - no distinction between signed and unsigned int

```
Format          Compute
------------------------------------------
addq  Src,Dest  Dest = Dest	+	Src
subq  Src,Dest  Dest = Dest - Src
imulq Src,Dest  Dest = Dest	*	Src
salq  Src,Dest  Dest = Dest	<< Src  (also shlq)
sarq  Src,Dest  Dest = Dest	>> Src  arithmetic
shrq  Src,Dest  Dest = Dest	>> Src  logical
xorq  Src,Dest  Dest = Dest	^	Src
andq  Src,Dest  Dest = Dest	&	Src
orq   Src,Dest  Dest = Dest	|	Src
```

- **1-operand instruction:**

```
Format        Compute
------------------------------------------
incq Dest     Dest = Dest	+ 1
decq Dest     Dest = Dest ­‐ 1
negq Dest     Dest = -­Dest
notq Dest     Dest = ~Dest
```
