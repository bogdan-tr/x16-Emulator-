#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}


// Beginning program counter
static int CODESTART = 600;

// ----------------- Test branch taken ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_jmp() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_jmp(R_R6));
    x16_set(machine, R_R6, 42);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for JMP
TEST_CASE("Control.WB.jmp", "[control.jmp]") {
    x16_t* machine = setup_test_machine_jmp();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // New PC should be loaded
    REQUIRE(x16_reg(machine, R_PC) == 42);

    x16_free(machine);
}


// ----------------- Test 2048 JMP ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_jmp2048() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_jmp(R_R1));
    x16_set(machine, R_R1, 420);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}


// ID for JMP
TEST_CASE("Control.ID.jmp.2048", "[control.jmp]") {
    x16_t* machine = setup_test_machine_jmp2048();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_PC) == 420);

    x16_free(machine);
}
