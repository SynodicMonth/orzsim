# simple mips simulator

**Code at Github: https://github.com/SynodicMonth/orzsim**

Simple instruction level mips simulator without pipeline support for **NKU computer architecture course.**

Also a simplest assembler `asm2hex.py` for testcase generation.

## how to use

```bash
python asm2hex.py asmfile.s hexfile.x
```
then
```bash
cd src
make
./sim hexfile.x
```

asmfile.s should in format of mips assembly, with only one .text section, for example:
```
	.text
main:	
	addiu $v0, $zero, 10
	addiu $t0, $zero, 5
	addiu $t1, $t0, 300
	addiu $t2, $zero, 500
	addiu $t3, $t2, 34
	addiu $t3, $t3, 45
	syscall
```
note that `asm2hex.py` only support a subset of mips instruction, and only assemble .text section, no .data section support. Also, it does not support label and instrution on the same line. so the old `addiu.s` will not work unless you add an `\n` after `main:`. There are many other limitations, but it is enough for this course.

then you will get a hexfile.x, which can be used by simulator.

hexfile.x should in format of 32bit hex number, each line is a instruction, for example:
```
2402000a
24080005
2509012c
240a01f4
254b0022
256b002d
0000000c
```

## implementation

```c
void process_instruction()
{
    // fetch instruction
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    // decode instruction
    struct Instruction i = decode(instruction);
    // execute instruction
    execute(i);
}
```

simulate three steps of mips pipeline, fetch, decode, execute (including memory access and register write back)
### fetch instruction

```c
uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
```
fetch an instruction from current PC, and store it in `instruction`.

### decode instruction

```c
struct Instruction i = decode(instruction);
```

```c
struct Instruction {
    uint32_t opcode;
    uint32_t rs;
    uint32_t rt;
    uint32_t rd;
    uint32_t shamt;
    uint32_t funct;
    uint32_t imm;
    uint32_t addr;
};

struct Instruction decode(uint32_t instruction)
{
    struct Instruction i;
    i.opcode = instruction >> 26;
    i.rs = (instruction >> 21) & 0x1F;
    i.rt = (instruction >> 16) & 0x1F;
    i.rd = (instruction >> 11) & 0x1F;
    i.shamt = (instruction >> 6) & 0x1F;
    i.funct = instruction & 0x3F;
    i.imm = (instruction & 0xFFFF);
    i.addr = instruction & 0x3FFFFFF;
    return i;
}
```
decode instruction to a struct, all fields are extracted from instruction.

### execute instruction

```c
execute(i);
```

according to opcode, instruction will be executed in different ways.

For example, sb instruction, we only need to store the lowest 8 bits of `rt` to memory address `rs + sign_extend(imm)`. In order to do this, we need to read the memory first using `mem_read_32`, then modify the lowest 8 bits, then write back to memory using `mem_write_32`.
```c
case 0x28: // sb
{
    uint32_t addr = CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm);
    uint32_t data = CURRENT_STATE.REGS[i.rt] & 0xFF;
    data = data | (mem_read_32(addr) & 0xFFFFFF00);
    mem_write_32(addr, data);
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    break;
}
```

In mips, sometimes immediate field needs to be sign_extended, which is implemented as:

```c
uint32_t sign_extend(uint32_t imm)
{
    if (imm & 0x8000) {
        return imm | 0xFFFF0000;
    } else {
        return imm;
    }
}
```

We only support halt syscall, so syscall is implemented as:

```c
case 0x0C: // syscall
  if (CURRENT_STATE.REGS[2] == 0x0A) {
      RUN_BIT = 0;
  }
  break;
```

Note that we don't simulate branch delay slot, so branch instructions can update `NEXT_STATE.PC` directly to the branch target when the branch is taken. 
“Jump-and-link” instructions (JAL, JALR, BLTZAL, BGEZAL) store `PC + 4` in `R31`, rather than `PC + 8` as specified in the manual in these instructions’ descriptions.

```c
case 0x03: // jal
  NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
  NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | (i.addr << 2);
  break;
```
For detail implementation for all the instructions, see `sim.c`.

## tests

I've tested the simulator with all the testcases provided by the course, and all of them passed. (same output as the junimay's simulator: https://github.com/JuniMay/mips-simulator)

## the assembler `asm2hex.py`

It is a simplest assembler, which only support a subset of mips instruction, and only addiu pseudo instruction is supported.

Below is an example of how to use it:
```
synodic@SynoDual:~/orzsim$ python3 inputs/asm2hex.py inputs/addiu.s inputs/addiu.x
.text
main:
addiu $v0, $zero, 10
addiu $t0, $zero, 5
addiu $t1, $t0, 300
addiu $t2, $zero, 500
addiu $t3, $t2, 34
addiu $t3, $t3, 45
syscall
---------LABEL ADDRESSES----------
main: 0x00000000
-----------CLEAN LINES----------
addiu $v0, $zero, 10
addiu $t0, $zero, 5
addiu $t1, $t0, 300
addiu $t2, $zero, 500
addiu $t3, $t2, 34
addiu $t3, $t3, 45
syscall
-----------TRANSLATED LINES----------
addiu $v0, $zero, 10
addiu $t0, $zero, 5
addiu $t1, $t0, 300
addiu $t2, $zero, 500
addiu $t3, $t2, 34
addiu $t3, $t3, 45
syscall
-----------ASSEMBLING----------
0x00000000 addiu $2, $0, 10                  ->  2402000a
0x00000004 addiu $8, $0, 5                   ->  24080005
0x00000008 addiu $9, $8, 300                 ->  2509012c
0x0000000c addiu $10, $0, 500                ->  240a01f4
0x00000010 addiu $11, $10, 34                ->  254b0022
0x00000014 addiu $11, $11, 45                ->  256b002d
0x00000018 syscall                           ->  0000000c
------------DONE------------
Machine code written to inputs/addiu.x
```

**since its not related to the simulator, I will not explain the implementation here.**

## supported instruction
 - sll
 - srl
 - sra
 - sllv
 - srlv
 - srav
 - jr
 - jalr
 - syscall* (only halt)
 - mfhi
 - mthi
 - mflo
 - mtlo
 - mult
 - multu
 - div
 - divu
 - add
 - addu
 - sub
 - subu
 - and
 - or
 - xor
 - nor
 - slt
 - sltu
 - bltz
 - bgez
 - bltzal
 - bgezal
 - j
 - jal
 - beq
 - bne
 - blez
 - bgtz
 - addi
 - addiu
 - slti
 - sltiu
 - andi
 - ori
 - xori
 - lui
 - lb
 - lh
 - lw
 - lbu
 - lhu
 - sb
 - sh
 - sw
