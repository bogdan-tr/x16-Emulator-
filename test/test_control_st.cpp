#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}

// Beginning program counter
static int CODESTART = 777;

// ----------------- Test ST  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_st(int offset, int location, int value) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_st(R_R3, offset));
    x16_memwrite(machine, CODESTART + 1 + offset, location);
    x16_set(machine, R_R3, value);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for ST
TEST_CASE("Control.WB.st", "[control.st]") {
    x16_t* machine = setup_test_machine_st(17, 99, 88);

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    REQUIRE(x16_memread(machine, CODESTART + 1 + 17) == 88);

    x16_t* machine2 = setup_test_machine_st(-17, 99, -88);

    // Execution should be successful for one instruction
    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);

    REQUIRE(x16_memread(machine2, CODESTART + 1 - 17) == (uint16_t) -88);
}
