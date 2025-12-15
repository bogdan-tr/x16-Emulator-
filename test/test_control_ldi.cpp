#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}


// Beginning program counter
static int CODESTART = 700;

// ----------------- Test LDI  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_ldi(int offset, int indirect, int value) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_ldi(R_R5, offset));
    x16_memwrite(machine, CODESTART + offset + 1, indirect);
    x16_memwrite(machine, indirect, (uint16_t) value);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for LDI
TEST_CASE("Control.WB.ldi", "[control.ldi]") {
    x16_t* machine = setup_test_machine_ldi(10, 55, -33);

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine, R_R5) == (uint16_t) -33);
    // Condition variables are set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_NEG);

    x16_free(machine);

    x16_t* machine2 = setup_test_machine_ldi(-10, 66, 22);

    // Execution should be successful for one instruction
    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);

    REQUIRE(x16_reg(machine2, R_PC) == CODESTART + 1);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine2, R_R5) == (uint16_t) 22);
    // Condition variables are set correctly
    REQUIRE(x16_reg(machine2, R_COND) == FL_POS);

    x16_free(machine2);
}
