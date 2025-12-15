#include "control.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bits.h"
#include "decode.h"
#include "instruction.h"
#include "trap.h"
#include "x16.h"

// Update condition code based on result
void update_cond(x16_t *machine, reg_t reg) {
  uint16_t result = x16_reg(machine, reg);
  if (result == 0) {
    x16_set(machine, R_COND, FL_ZRO);
  } else if (is_negative(result)) {
    x16_set(machine, R_COND, FL_NEG);
  } else {
    x16_set(machine, R_COND, FL_POS);
  }
}

// Execute a single instruction in the given X16 machine. Update
// memory and registers as required. PC is advanced as appropriate.
// Return 0 on success, or -1 if an error or HALT is encountered.
int execute_instruction(x16_t *machine) {
  // Fetch the instruction and advance the program counter
  uint16_t pc = x16_pc(machine);
  uint16_t instruction = x16_memread(machine, pc);
  x16_set(machine, R_PC, pc + 1);

  if (LOG) {
    fprintf(LOGFP, "0x%x: %s\n", pc, decode(instruction));
  }

  // Variables we might need in various instructions
  reg_t dst, src1, src2, base, r7;
  uint16_t result, indirect, offset, imm, cond, jsrflag, op1, op2, address,
      addressContents, baseContents, resultContents;

  // Decode the instruction
  uint16_t opcode = getopcode(instruction);
  switch (opcode) {
    case OP_ADD:
      // grab registers
      dst = getbits(instruction, 9, 3);   // 9-11
      src1 = getbits(instruction, 6, 3);  // 6-8
      if (getbit(instruction, 5) == 0) {
        // DR = SR1 + SR2
        src2 = getbits(instruction, 0, 3);  // 0-2
        // grab values at registers
        op1 = x16_reg(machine, src1);
        op2 = x16_reg(machine, src2);
        result = op1 + op2;
      } else {
        // DR +SEXT(imm5)
        imm = getbits(instruction, 0, 5);
        // grab values at registers
        op1 = x16_reg(machine, src1);
        // sign extend imm
        imm = sign_extend(imm, 5);  // imm is 5 bits wide
        result = op1 + imm;
      }
      x16_set(machine, dst, result);
      update_cond(machine, dst);
      break;

    case OP_AND:
      dst = getbits(instruction, 9, 3);   // 9-11
      src1 = getbits(instruction, 6, 3);  // 6-8
      if (getbit(instruction, 5) == 0) {
        // DR = SR1 + SR2
        src2 = getbits(instruction, 0, 3);  // 0-2
        // grab values at registers
        op1 = x16_reg(machine, src1);
        op2 = x16_reg(machine, src2);
        result = op1 & op2;
      } else {
        // DR +SEXT(imm5)
        imm = getbits(instruction, 0, 5);  // 0-4
        // grab values at registers
        op1 = x16_reg(machine, src1);
        // sign extend imm
        imm = sign_extend(imm, 5);  // imm is 5 bits wide
        result = op1 & imm;
      }
      x16_set(machine, dst, result);
      update_cond(machine, dst);
      break;

    case OP_NOT:
      dst = getbits(instruction, 9, 3);   // 9-11
      src1 = getbits(instruction, 6, 3);  // 6-8
                                          // grab values at registers
      op1 = x16_reg(machine, src1);
      result = ~op1;
      // update conditions
      x16_set(machine, dst, result);
      update_cond(machine, dst);
      break;

    case OP_BR:
      offset = getbits(instruction, 0, 9);  // 0-8
      offset = sign_extend(offset, 9);      // sign extend
                                            // grab necessary bits
      bool n = getbit(instruction, 11);     // isnegative flag
      bool z = getbit(instruction, 10);     // iszero flag
      bool p = getbit(instruction, 9);      // ispositive flag
      uint16_t R_COND = x16_cond(machine);
      // check uncoditional case
      if (!n && !z && !p) {
        // get program counter
        uint16_t pc = x16_pc(machine);
        result = pc + offset;            // update pc
        x16_set(machine, R_PC, result);  // set pc
      } else if (n && (R_COND & FL_NEG) || z && (R_COND & FL_ZRO) ||
                 p && (R_COND & FL_POS)) {  // determine if ump necessary
        // get program counter
        uint16_t pc = x16_pc(machine);
        result = pc + offset;            // update pc
        x16_set(machine, R_PC, result);  // set pc
      }

      break;

    case OP_JMP:
      base = getbits(instruction, 6, 3);  // 6-8
      if (base == R_R7) {                 // ret
        op1 = x16_reg(machine, R_R7);
        x16_set(machine, R_PC, op1);  // set pc to r7 contents
      } else {
        op1 = x16_reg(machine, base);
        x16_set(machine, R_PC, op1);  // set pc to baseR contents
      }
      break;

    case OP_JSR:
      // save pc to r7
      pc = x16_pc(machine);
      x16_set(machine, R_R7, pc);
      // grab bit11
      bool bit11 = getbit(instruction, 11);
      if (bit11) {
        // bit 11 is 1 (JSR)
        result = getbits(instruction, 0, 11);  // 0-10
        result = sign_extend(result, 11);      // sign extend
        result += pc;
        x16_set(machine, R_PC, result);  // set pc
      } else {
        // bit 11 is 0 (JSRR)
        base = getbits(instruction, 6, 3);  // 6-8
        op1 = x16_reg(machine, base);
        x16_set(machine, R_PC, op1);  // set pc
      }
      break;

    case OP_LD:
      dst = getbits(instruction, 9, 3);     // 9-11 (DR)
      offset = getbits(instruction, 0, 9);  // 0-8
      offset = sign_extend(offset, 9);      // sign extend
      pc = x16_pc(machine);                 // grab pc
      address = offset + pc;
      addressContents = x16_memread(machine, address);
      x16_set(machine, dst, addressContents);  // update dst register
      update_cond(machine, dst);
      break;

    case OP_LDI:
      dst = getbits(instruction, 9, 3);     // 9-11 (DR)
      offset = getbits(instruction, 0, 9);  // 0-8
      offset = sign_extend(offset, 9);      // sign extend
      pc = x16_pc(machine);                 // grab pc
      address = offset + pc;
      addressContents = x16_memread(
          machine, address);  // addressContents should be itself an address
      // next we need to read addressContents and get the value
      addressContents = x16_memread(machine, addressContents);
      x16_set(machine, dst, addressContents);  // update dst register
      update_cond(machine, dst);
      break;

    case OP_LDR:
      // grab necessary stuff
      offset = getbits(instruction, 0, 6);    // 0-5
      offset = sign_extend(offset, 6);        // sign extend the offset
      base = getbits(instruction, 6, 3);      // 6-8
      dst = getbits(instruction, 9, 3);       // 9-11 (DR)
      baseContents = x16_reg(machine, base);  // get base contents
                                              // calculate new address
      result =
          offset + baseContents;  // NOTE result is an address, not register
      // get contents of new address
      resultContents = x16_memread(machine,
                                   result);   // load stuff from result
      x16_set(machine, dst, resultContents);  // update dst register
      update_cond(machine, dst);
      break;

    case OP_LEA:
      // grab necessary stuff
      dst = getbits(instruction, 9, 3);     // 9-11 (DR)
      offset = getbits(instruction, 0, 9);  // 0-8
      offset = sign_extend(offset, 9);      // sign extend the offset
      pc = x16_pc(machine);                 // grab pc
      result = pc + offset;                 // yields memory address
      x16_set(machine, dst, result);        // load result into dst register
      update_cond(machine, dst);
      break;

    case OP_ST:
      src1 = getbits(instruction, 9, 3);    // 9-11 (SR)
      op1 = x16_reg(machine, src1);         // get src1 contents
      offset = getbits(instruction, 0, 9);  // 0-8
      offset = sign_extend(offset, 9);      // sign extend the offset
      pc = x16_pc(machine);                 // grab pc
      result = pc + offset;                 // yields memory address
      // store op1 into address at result
      x16_memwrite(machine, result, op1);
      break;

    case OP_STI:
      src1 = getbits(instruction, 9, 3);    // 9-11 (SR)
      op1 = x16_reg(machine, src1);         // get src1 contents
      offset = getbits(instruction, 0, 9);  // 0-8
      offset = sign_extend(offset, 9);      // sign extend the offset
      pc = x16_pc(machine);                 // grab pc
      result = pc + offset;                 // yields memory address
                             // get what is in memory at this adress (result)
      resultContents = x16_memread(machine, result);
      x16_memwrite(machine, resultContents, op1);

      break;

    case OP_STR:
      // grab stuff
      src1 = getbits(instruction, 9, 3);      // 9-11 (SR)
      base = getbits(instruction, 6, 3);      // 6-8 (BR)
      offset = getbits(instruction, 0, 6);    // 0-5
      offset = sign_extend(offset, 6);        // sign extend the offset
      op1 = x16_reg(machine, src1);           // src1 contents
      baseContents = x16_reg(machine, base);  // base contents
      result = offset + baseContents;         // yields memory address
      // store op1 into address at result
      x16_memwrite(machine, result, op1);
      break;

    case OP_TRAP:
      // Execute the trap -- do not rewrite
      return trap(machine, instruction);

    case OP_RES:

    case OP_RTI:
    default:
      // Bad codes, never used
      abort();
  }

  return 0;
}
