#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}


// Beginning program counter
static int CODESTART = 300;

// ----------------- Test LDR  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_ldr(int offset, int location, int value) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_ldr(R_R0, R_R1, offset));
    x16_memwrite(machine, location + offset, (uint16_t) value);
    x16_set(machine, R_R1, location);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for LDR
TEST_CASE("Control.WB.ldr", "[control.ldr]") {
    x16_t* machine = setup_test_machine_ldr(-10, 22, -77);

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine, R_R0) == (uint16_t) -77);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_NEG);

    x16_free(machine);

    x16_t* machine2 = setup_test_machine_ldr(10, 33, 77);

    // Execution should be successful for one instruction
    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine2, R_R0) == (uint16_t) 77);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine2, R_COND) == FL_POS);

    x16_free(machine2);
}
