#include <stdio.h>
#include "shell.h"

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

uint32_t sign_extend(uint32_t imm)
{
    if (imm & 0x8000) {
        return imm | 0xFFFF0000;
    } else {
        return imm;
    }
}

void execute(struct Instruction i)
{
    switch (i.opcode) {
        case 0x00: // R-type
            switch (i.funct) {
                case 0x00: // sll
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rt] << i.shamt;
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x02: // srl
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rt] >> i.shamt;
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x03: // sra
                    NEXT_STATE.REGS[i.rd] = (int32_t) CURRENT_STATE.REGS[i.rt] >> i.shamt;
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x04: // sllv
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rt] << (CURRENT_STATE.REGS[i.rs] & 0x1F);
                    break;
                case 0x06: // srlv
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rt] >> (CURRENT_STATE.REGS[i.rs] & 0x1F);
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x07: // srav
                    NEXT_STATE.REGS[i.rd] = (int32_t) CURRENT_STATE.REGS[i.rt] >> (CURRENT_STATE.REGS[i.rs] & 0x1F);
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x08: // jr
                    NEXT_STATE.PC = CURRENT_STATE.REGS[i.rs];
                    break;
                case 0x09: // jalr
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.PC + 4;
                    NEXT_STATE.PC = CURRENT_STATE.REGS[i.rs];
                    break;
                case 0x0C: // syscall
                    if (CURRENT_STATE.REGS[2] == 0x0A) {
                        RUN_BIT = 0;
                    }
                    break;
                case 0x10: // mfhi
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.HI;
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x11: // mthi
                    NEXT_STATE.HI = CURRENT_STATE.REGS[i.rs];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x12: // mflo
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.LO;
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x13: // mtlo
                    NEXT_STATE.LO = CURRENT_STATE.REGS[i.rs];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x18: // mult
                {
                    int64_t result = (int64_t)((int32_t) CURRENT_STATE.REGS[i.rs] * (int32_t) CURRENT_STATE.REGS[i.rt]);
                    NEXT_STATE.LO = (uint32_t)(result & 0xFFFFFFFF);
                    NEXT_STATE.HI = (uint32_t)(result >> 32);
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                }
                case 0x19: // multu
                {
                    uint64_t result = (uint64_t) CURRENT_STATE.REGS[i.rs] * (uint64_t) CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.LO = (uint32_t)(result & 0xFFFFFFFF);
                    NEXT_STATE.HI = (uint32_t)(result >> 32);
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                }
                case 0x1A: // div
                    NEXT_STATE.LO = (int32_t) CURRENT_STATE.REGS[i.rs] / (int32_t) CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.HI = (int32_t) CURRENT_STATE.REGS[i.rs] % (int32_t) CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x1B: // divu
                    NEXT_STATE.LO = CURRENT_STATE.REGS[i.rs] / CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.HI = CURRENT_STATE.REGS[i.rs] % CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x20: // add
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rs] + CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x21: // addu
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rs] + CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x22: // sub
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rs] - CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x23: // subu
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rs] - CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x24: // and
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rs] & CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x25: // or
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rs] | CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x26: // xor
                    NEXT_STATE.REGS[i.rd] = CURRENT_STATE.REGS[i.rs] ^ CURRENT_STATE.REGS[i.rt];
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x27: // nor
                    NEXT_STATE.REGS[i.rd] = ~(CURRENT_STATE.REGS[i.rs] | CURRENT_STATE.REGS[i.rt]);
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x2A: // slt
                    NEXT_STATE.REGS[i.rd] = ((int32_t) CURRENT_STATE.REGS[i.rs] < (int32_t) CURRENT_STATE.REGS[i.rt]) ? 1 : 0;
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                case 0x2B: // sltu
                    NEXT_STATE.REGS[i.rd] = (CURRENT_STATE.REGS[i.rs] < CURRENT_STATE.REGS[i.rt]) ? 1 : 0;
                    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    break;
                default:
                    printf("Invalid instruction at %08x\n", CURRENT_STATE.PC);
                    RUN_BIT = 0;
                    break;
            }
            break;
        case 0x01: // bgez, bgezal, bltz, bltzal
            switch (i.rt) {
                case 0x00: // bltz
                    if ((int32_t) CURRENT_STATE.REGS[i.rs] < 0) {
                        NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extend(i.imm) << 2);
                    } else {
                        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    }
                    break;
                case 0x01: // bgez
                    if ((int32_t) CURRENT_STATE.REGS[i.rs] >= 0) {
                        NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extend(i.imm) << 2);
                    } else {
                        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    }
                    break;
                case 0x10: // bltzal
                    if ((int32_t) CURRENT_STATE.REGS[i.rs] < 0) {
                        NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                        NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extend(i.imm) << 2);
                    } else {
                        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    }
                    break;
                case 0x11: // bgezal
                    if ((int32_t) CURRENT_STATE.REGS[i.rs] >= 0) {
                        NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
                        NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extend(i.imm) << 2);
                    } else {
                        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                    }
                    break;
                default:
                    printf("Invalid instruction at %08x\n", CURRENT_STATE.PC);
                    RUN_BIT = 0;
                    break;
            }
            break;
        case 0x02: // j
            NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | (i.addr << 2);
            break;
        case 0x03: // jal
            NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
            NEXT_STATE.PC = (CURRENT_STATE.PC & 0xF0000000) | (i.addr << 2);
            break;
        case 0x04: // beq
            if (CURRENT_STATE.REGS[i.rs] == CURRENT_STATE.REGS[i.rt]) {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extend(i.imm) << 2);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0x05: // bne
            if (CURRENT_STATE.REGS[i.rs] != CURRENT_STATE.REGS[i.rt]) {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extend(i.imm) << 2);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0x06: // blez
            if ((int32_t) CURRENT_STATE.REGS[i.rs] <= 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extend(i.imm) << 2);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0x07: // bgtz
            if ((int32_t) CURRENT_STATE.REGS[i.rs] > 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4 + (sign_extend(i.imm) << 2);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case 0x08: // addi
            NEXT_STATE.REGS[i.rt] = CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x09: // addiu
            NEXT_STATE.REGS[i.rt] = CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x0A: // slti
            NEXT_STATE.REGS[i.rt] = ((int32_t) CURRENT_STATE.REGS[i.rs] < sign_extend(i.imm)) ? 1 : 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x0B: // sltiu
            NEXT_STATE.REGS[i.rt] = (CURRENT_STATE.REGS[i.rs] < sign_extend(i.imm)) ? 1 : 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x0C: // andi
            NEXT_STATE.REGS[i.rt] = CURRENT_STATE.REGS[i.rs] & i.imm;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x0D: // ori
            NEXT_STATE.REGS[i.rt] = CURRENT_STATE.REGS[i.rs] | i.imm;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x0E: // xori
            NEXT_STATE.REGS[i.rt] = CURRENT_STATE.REGS[i.rs] ^ i.imm;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x0F: // lui
            NEXT_STATE.REGS[i.rt] = i.imm << 16;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x20: // lb
        {
            uint8_t byte = mem_read_32(CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm)) & 0xFF;
            if (byte & 0x80) {
                NEXT_STATE.REGS[i.rt] = (int32_t)(byte) | 0xFFFFFF00;
            } else {
                NEXT_STATE.REGS[i.rt] = (int32_t)(byte);
            }
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }
        case 0x21: // lh
        {
            uint16_t halfword = mem_read_32(CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm)) & 0xFFFF;
            if (halfword & 0x8000) {
                NEXT_STATE.REGS[i.rt] = (int32_t)(halfword) | 0xFFFF0000;
            } else {
                NEXT_STATE.REGS[i.rt] = (int32_t)(halfword);
            }
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }
        case 0x23: // lw
            NEXT_STATE.REGS[i.rt] = mem_read_32(CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm));
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x24: // lbu
            NEXT_STATE.REGS[i.rt] = mem_read_32(CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm)) & 0xFF;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x25: // lhu
            NEXT_STATE.REGS[i.rt] = mem_read_32(CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm)) & 0xFFFF;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        case 0x28: // sb
        {
            uint32_t addr = CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm);
            uint32_t data = CURRENT_STATE.REGS[i.rt] & 0xFF;
            data = data | (mem_read_32(addr) & 0xFFFFFF00);
            mem_write_32(addr, data);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }
        case 0x29: // sh
        {
            uint32_t addr = CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm);
            uint32_t data = CURRENT_STATE.REGS[i.rt] & 0xFFFF;
            data = data | (mem_read_32(addr) & 0xFFFF0000);
            mem_write_32(addr, data);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }
        case 0x2B: // sw
            mem_write_32(CURRENT_STATE.REGS[i.rs] + sign_extend(i.imm), CURRENT_STATE.REGS[i.rt]);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        default:
            printf("Invalid instruction at %08x\n", CURRENT_STATE.PC);
            RUN_BIT = 0;
            break;
    }
}

void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    // fetch instruction
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    // decode instruction
    struct Instruction i = decode(instruction);
    // execute instruction
    execute(i);
}
