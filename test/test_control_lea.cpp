#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}


// Beginning program counter
static int CODESTART = 900;

// ----------------- Test LEA  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_lea(int offset) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_lea(R_R7, offset));
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for LEA
TEST_CASE("Control.WB.lea", "[control.lea]") {
    x16_t* machine = setup_test_machine_lea(44);

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine, R_R7) == CODESTART + 1 + 44);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_POS);

    x16_free(machine);

    x16_t* machine2 = setup_test_machine_lea(-44);

    // Execution should be successful for one instruction
    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine2, R_R7) == CODESTART + 1 - 44);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine2, R_COND) == FL_POS);

    x16_free(machine2);
}

// ----------------- Test LEA neg  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_lea_neg() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_lea(R_R7, (uint16_t) -44));
    x16_set(machine, R_PC, CODESTART);

    return machine;
}


// WB for LEA neg
TEST_CASE("Control.WB.lea.neg", "[control.lea]") {
    x16_t* machine = setup_test_machine_lea_neg();

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine, R_R7) == CODESTART + 1 - 44);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_POS);

    x16_free(machine);
}
