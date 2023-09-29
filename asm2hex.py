import argparse

class MIPSAssembler:
    def __init__(self):
        self.instruction_map = {
            'add': self.encode_r_type(32),
            'addu': self.encode_r_type(33),
            'sub': self.encode_r_type(34),
            'subu': self.encode_r_type(35),
            'and': self.encode_r_type(36),
            'or': self.encode_r_type(37),
            'xor': self.encode_r_type(38),
            'nor': self.encode_r_type(39),
            'slt': self.encode_r_type(42),
            'sltu': self.encode_r_type(43),
            'sll': self.encode_shift(0),
            'srl': self.encode_shift(2),
            'sra': self.encode_shift(3),
            'sllv': self.encode_r_type(4),
            'srlv': self.encode_r_type(6),
            'srav': self.encode_r_type(7),
            'jr': self.encode_jr(8),
            'jalr': self.encode_jalr(9),
            'syscall': self.encode_syscall(12),
            'mfhi': self.encode_r_type_no_args(16, 'rd'),
            'mthi': self.encode_r_type_no_args(17, 'rs'),
            'mflo': self.encode_r_type_no_args(18, 'rd'),
            'mtlo': self.encode_r_type_no_args(19, 'rs'),
            'mult': self.encode_mult_div(24),
            'multu': self.encode_mult_div(25),
            'div': self.encode_mult_div(26),
            'divu': self.encode_mult_div(27),

            'j': self.encode_j_type(2),
            'jal': self.encode_j_type(3),
            'beq': self.encode_branch(4),
            'bne': self.encode_branch(5),
            'blez': self.encode_single_source_branch(6),
            'bgtz': self.encode_single_source_branch(7),
            'bgez': self.encode_single_register_branch(1, 1),    # opcode=1, rt=1
            'bgezal': self.encode_single_register_branch(1, 17), # opcode=1, rt=17
            'bltz': self.encode_single_register_branch(1, 0),    # opcode=1, rt=0
            'bltzal': self.encode_single_register_branch(1, 16),  # opcode=1, rt=16
            'addi': self.encode_i_type(8),
            'addiu': self.encode_i_type(9),
            'slti': self.encode_i_type(10),
            'sltiu': self.encode_i_type(11),
            'andi': self.encode_i_type(12),
            'ori': self.encode_i_type(13),
            'xori': self.encode_i_type(14),
            'lui': self.encode_lui(15),

            'lb': self.encode_memory_access(32),
            'lh': self.encode_memory_access(33),
            'lw': self.encode_memory_access(35),
            'lbu': self.encode_memory_access(36),
            'lhu': self.encode_memory_access(37),
            'sb': self.encode_memory_access(40),
            'sh': self.encode_memory_access(41),
            'sw': self.encode_memory_access(43),
        }
        self.labels = {}
        self.registers = {
            "$zero": 0, "$at": 1, "$v0": 2, "$v1": 3, "$a0": 4, "$a1": 5, "$a2": 6, "$a3": 7,
            "$t0": 8, "$t1": 9, "$t2": 10, "$t3": 11, "$t4": 12, "$t5": 13, "$t6": 14, "$t7": 15,
            "$s0": 16, "$s1": 17, "$s2": 18, "$s3": 19, "$s4": 20, "$s5": 21, "$s6": 22, "$s7": 23,
            "$t8": 24, "$t9": 25, "$k0": 26, "$k1": 27, "$gp": 28, "$sp": 29, "$fp": 30, "$ra": 31
        }

    def encode_r_type(self, funct):
        def encoder(args):
            rd = int(args[0][1:])
            rs = int(args[1][1:])
            rt = int(args[2][1:])
            shamt = 0  # Shift amount is typically 0 for these instructions
            return (0 << 26) | (rs << 21) | (rt << 16) | (rd << 11) | (shamt << 6) | funct
        return encoder

    def encode_shift(self, funct):
        def encoder(args):
            rd = int(args[0][1:])
            rt = int(args[1][1:])
            shamt = int(args[2])
            return (0 << 26) | (0 << 21) | (rt << 16) | (rd << 11) | (shamt << 6) | funct
        return encoder

    def encode_jr(self, funct):
        def encoder(args):
            rs = int(args[0][1:])
            return (0 << 26) | (rs << 21) | funct
        return encoder

    def encode_jalr(self, funct):
        def encoder(args):
            rs = int(args[0][1:])
            rd = int(args[1][1:]) if len(args) > 1 else 31  # default to $ra if rd is not provided
            return (0 << 26) | (rs << 21) | (rd << 11) | funct
        return encoder

    def encode_syscall(self, funct):
        def encoder(args):
            code = int(args[0]) if args else 0  # syscall code, default to 0 (halt) if not provided
            return (0 << 26) | (code << 6) | funct
        return encoder

    def encode_r_type_no_args(self, funct, target):
        def encoder(args):
            reg = int(args[0][1:])
            targets = {'rd': 11, 'rs': 21, 'rt': 16}
            return (0 << 26) | (reg << targets[target]) | funct
        return encoder

    def encode_mult_div(self, funct):
        def encoder(args):
            rs = int(args[0][1:])
            rt = int(args[1][1:])
            return (0 << 26) | (rs << 21) | (rt << 16) | funct
        return encoder

    def encode_i_type(self, opcode):
        def encoder(args):
            rt = int(args[0][1:])
            rs = int(args[1][1:])
            imm = int(args[2]) & 0xFFFF
            return (opcode << 26) | (rs << 21) | (rt << 16) | imm
        return encoder

    def encode_lui(self, opcode):
        def encoder(args):
            rt = int(args[0][1:])
            imm = int(args[1]) & 0xFFFF
            return (opcode << 26) | (0 << 21) | (rt << 16) | imm
        return encoder

    def encode_j_type(self, opcode):
        def encoder(args):
            address = int(args[0]) & 0x3FFFFFF
            return (opcode << 26) | (address >> 2)
        return encoder

    def encode_branch(self, opcode):
        def encoder(args):
            rs = int(args[0][1:])
            rt = int(args[1][1:])
            offset = int(args[2]) & 0xFFFF  # This should be label-address calculation in practical
            return (opcode << 26) | (rs << 21) | (rt << 16) | offset
        return encoder

    def encode_single_source_branch(self, opcode):
        def encoder(args):
            rs = int(args[0][1:])
            offset = int(args[1]) & 0xFFFF  # This should be label-address calculation in practical
            return (opcode << 26) | (rs << 21) | offset
        return encoder

    def encode_memory_access(self, opcode):
        def encoder(args):
            rt = int(args[0][1:])
            offset, base = args[1].split('(')
            base = int(base.rstrip(')')[1:])
            offset = int(offset) & 0xFFFF
            return (opcode << 26) | (base << 21) | (rt << 16) | offset
        return encoder

    def encode_single_register_branch(self, opcode, funct_rt):
      def encoder(args):
          rs = int(args[0][1:])
          offset = int(args[1]) & 0xFFFF  # This should be label-address calculation in practical scenarios
          return (opcode << 26) | (rs << 21) | (funct_rt << 16) | offset
      return encoder

    def assemble(self, asm_code):
        lines = asm_code.strip().split('\n')

        # First pass to collect labels and clean comments
        clean_lines = []
        address = 0
        for line in lines:
            line = line.split('#')[0].strip()  # Remove comments
            if not line:  # Skip empty lines or full comment lines
                continue

            if line.endswith(':'):  # This is a label
                label = line[:-1]
                self.labels[label] = address
            else:
                clean_lines.append(line)
                address += 4  # Each instruction is 4 bytes long
        print("---------LABEL ADDRESSES----------")
        for label, address in self.labels.items():
            print(f"{label}: 0x{address:08x}")

        print("-----------CLEAN LINES----------")
        for line in clean_lines:
            print(line)

         # Second pass to translate pseudo-instructions
        translated_lines = []
        for line in clean_lines:
            parts = line.split()
            instr = parts[0]
            args = parts[1:] if len(parts) > 1 else []
            if instr == 'addiu' and int(args[2], 0) > 0x7FFF:  # Check if it's a pseudo-instruction
                # Break it down into lui and ori
                upper_16_bits = (int(args[2], 0) & 0xFFFF0000) >> 16
                lower_16_bits = int(args[2], 0) & 0x0000FFFF
                print(args[1])
                if args[1] == '$zero,' or args[1] == '$0,':
                    translated_lines.append(f'lui $1 {upper_16_bits}')
                    translated_lines.append(f'ori {args[0]} $1 {lower_16_bits}')
                else:
                  translated_lines.append(f'lui $1 {upper_16_bits}')
                  translated_lines.append(f'ori $1 $1 {lower_16_bits}')
                  translated_lines.append(f'addu {args[0]} {args[1]} $1')
            else:
                translated_lines.append(line)
                
        print("-----------TRANSLATED LINES----------")
        for line in translated_lines:
            print(line)

        print("-----------ASSEMBLING----------")
        # Second pass to encode instructions
        machine_code = []
        address = 0
        for line in translated_lines:
            parts = line.split()
            instr = parts[0]
            args = [self.clean_arg(arg) for arg in parts[1:]] if len(parts) > 1 else []
            raw_line = instr + ' ' + ', '.join(args)
            print(f"0x{address:08x} {raw_line:32}  ->  ", end='')
            if instr in self.instruction_map:
                if args and args[-1] in self.labels:  # Replace label with actual address or offset
                    label_address = self.labels[args[-1]]
                    if 'b' in instr:
                        offset = (label_address - address - 1) // 4
                    else:
                        offset = 0x00400000 + label_address # Calculate offset for branch instructions
                    args[-1] = str(offset)  # Use offset or address based on instruction type

                encoded_instr = self.instruction_map[instr](args)
                print(f'{encoded_instr:08x}')
                machine_code.append(f'{encoded_instr:08x}')
            else:
                print(f"Warning: Instruction '{instr}' not supported.")
            address += 4  # Increment address for each instruction

        return machine_code
    
    def clean_arg(self, arg):
        arg = arg.replace(',', '').strip()
        if arg.startswith('0x'):  # Check if it's a hex immediate
            return str(int(arg, 16))
        elif arg in self.registers:  # Check if it's an ABI register name
            return f'${self.registers[arg]}'
        return arg
    
    def assemble_file(self, input_filename, output_filename):
        with open(input_filename, 'r') as file:
            lines = file.readlines()

        # Removing comments and cleaning lines
        clean_lines = [line.split('#')[0].strip() for line in lines if line.strip() and not line.startswith('#')]

        for line in clean_lines:
            print(line)
        # Extracting lines under .text section
        try:
            start_index = clean_lines.index('.text') + 1
        except ValueError:
            raise ValueError(".text section not found in the input file")

        asm_code = '\n'.join(clean_lines[start_index:])

        # Assembling the code
        machine_code = self.assemble(asm_code)

        # Writing machine code to output file
        with open(output_filename, 'w') as file:
            for code in machine_code:
                file.write(code + '\n')

        print("------------DONE------------")
        print(f"Machine code written to {output_filename}")


def main():
    parser = argparse.ArgumentParser(description="Convert MIPS assembly code to machine code.")
    parser.add_argument('input_file', type=str, help='The input .s file containing MIPS assembly code.')
    parser.add_argument('output_file', type=str, help='The output .x file to write the machine code.')

    args = parser.parse_args()

    assembler = MIPSAssembler()
    assembler.assemble_file(args.input_file, args.output_file)


if __name__ == "__main__":
    main()