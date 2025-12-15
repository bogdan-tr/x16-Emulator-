#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}


// Beginning program counter
static int CODESTART = 300;

// ----------------- Test NOT ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_not() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_not(R_R1, R_R7));
    x16_set(machine, R_R7, 55);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for NOT
TEST_CASE("Control.WB.not", "[control.not]") {
    x16_t* machine = setup_test_machine_not();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // Result loaded in WB
    REQUIRE(x16_reg(machine, R_R1) == (uint16_t) ~55);
    // Check for negative condition code
    REQUIRE(x16_reg(machine, R_COND) == FL_NEG);

    x16_free(machine);
}
