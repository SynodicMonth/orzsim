# simple mips simulator

simple mips simulator for NKU computer architecture course

## how to use

```bash
$ cd src
$ make
$ ./sim hexfile.x
```
hexfile.x should in format of 32bit hex number, each line is a instruction:
```
2402000a
24080005
2509012c
240a01f4
254b0022
256b002d
0000000c
```
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