#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}

// Beginning program counter
static int CODESTART = 300;

// ----------------- Test STR  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_str(int offset, int indirect, int value) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_str(R_R3, R_R4, offset));
    x16_set(machine, R_R3, value);
    x16_set(machine, R_R4, (uint16_t) indirect);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for STR
TEST_CASE("Control.WB.str", "[control.str]") {
    x16_t* machine = setup_test_machine_str(19, -5, 88);

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    // This is a no-op. There is no register writeback
    // Data register should contain contents of memory
    REQUIRE(x16_memread(machine, -5 + 19) == 88);

    x16_free(machine);


    x16_t* machine2 = setup_test_machine_str(-19, 5, -88);

    // Execution should be successful for one instruction
    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);

    REQUIRE(x16_reg(machine2, R_PC) == CODESTART + 1);

    // This is a no-op. There is no register writeback
    // Data register should contain contents of memory
    REQUIRE(x16_memread(machine2, 5 - 19) == (uint16_t) -88);

    x16_free(machine2);
}
