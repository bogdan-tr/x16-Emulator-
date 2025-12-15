#include "catch.hpp"

extern "C" {
#include "instruction.h"
}




// ------------------------ Test Instructions ----------------------

TEST_CASE("Instruction.opcode", "[instruction]") {
    uint16_t instruction = 0x1000;
    REQUIRE(getopcode(instruction) == OP_ADD);
}

TEST_CASE("Instruction.getimmediate", "[instruction]") {
    uint16_t instruction;

    instruction = emit_add_reg(R_R1, R_R2, R_R3);
    REQUIRE(getimmediate(instruction) == 0);

    instruction = emit_add_imm(R_R1, R_R2, 10);
    REQUIRE(getimmediate(instruction) == 1);
}
