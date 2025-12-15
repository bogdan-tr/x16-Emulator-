#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}

// Beginning program counter
static int CODESTART = 300;

// ----------------- Test STI  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_sti(int offset, int location, int value) {
    x16_t* machine = x16_create();

    // Store indirect. The PC and offset gives location 318.
    // The memory at location 318 gives location 99.
    // Therefore the result should be a 88 stored at location 99.
    x16_memwrite(machine, CODESTART, emit_sti(R_R3, offset));
    x16_memwrite(machine, CODESTART + 1 + offset, location);
    x16_set(machine, R_R3, value);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}


// WB for STI
TEST_CASE("Control.WB.sti", "[control.sti]") {
    x16_t* machine = setup_test_machine_sti(17, 99, 88);

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    // This is a no-op. There is no register writeback
    // Data register should contain contents of memory
    REQUIRE(x16_memread(machine, 99) == 88);

    x16_t* machine2 = setup_test_machine_sti(-17, 99, -88);

    // Execution should be successful for one instruction
    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);

    REQUIRE(x16_reg(machine2, R_PC) == CODESTART + 1);

    // This is a no-op. There is no register writeback
    // Data register should contain contents of memory
    REQUIRE(x16_memread(machine2, 99) == (uint16_t) -88);
}
