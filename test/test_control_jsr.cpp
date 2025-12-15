#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}

// Beginning program counter
static int CODESTART = 300;

// ----------------- Test JSR  ----------------------

// This function initializes the machine to a known state for
static x16_t* setup_test_machine_jsr(int offset) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_jsr(offset));
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for JSR
TEST_CASE("Control.WB.jsr", "[control.jsr]") {
    x16_t* machine = setup_test_machine_jsr(66);

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_R7) == CODESTART + 1);

    // This is a no op for a jsr
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1 + 66);

    x16_free(machine);

    x16_t* machine2 = setup_test_machine_jsr(-33);

    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);

    REQUIRE(x16_reg(machine2, R_R7) == CODESTART + 1);

    // This is a no op for a jsr
    REQUIRE(x16_reg(machine2, R_PC) == CODESTART + 1 - 33);

    x16_free(machine2);
}


// ----------------- Test JSRR ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_jsrr() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_jsrr(R_R6));
    x16_set(machine, R_R6, 42);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for JSRR
TEST_CASE("Control.WB.jsrr", "[control.jsr]") {
    x16_t* machine = setup_test_machine_jsrr();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // This is a no op for a jsrr
    REQUIRE(x16_reg(machine, R_PC) == 42);
    REQUIRE(x16_reg(machine, R_R7) == CODESTART + 1);

    x16_free(machine);
}
