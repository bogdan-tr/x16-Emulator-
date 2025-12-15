#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}

// Beginning program counter
static int CODESTART = 800;

// ----------------- Test LD  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_ld(int offset, int value) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_ld(R_R5, offset - 1));
    x16_memwrite(machine, CODESTART + offset, value);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for LD
TEST_CASE("Control.WB.ld", "[control.ld]") {
    x16_t* machine = setup_test_machine_ld(11, 55);

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine, R_R5) == 55);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_POS);

    x16_free(machine);

    x16_t* machine2 = setup_test_machine_ld(-22, 33);

    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);

    REQUIRE(x16_reg(machine2, R_PC) == CODESTART + 1);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine2, R_R5) == 33);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine2, R_COND) == FL_POS);

    x16_free(machine2);
}

// ----------------- Test LD with negative number  ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_ld_neg() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_ld(R_R5, 10));
    x16_memwrite(machine, CODESTART + 11, (uint16_t) -55);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}


// WB for LD negative
TEST_CASE("Control.WB.ld.neg", "[control.ld]") {
    x16_t* machine = setup_test_machine_ld_neg();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The register is loaded with the right value
    REQUIRE(x16_reg(machine, R_R5) == (uint16_t) -55);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_NEG);

    x16_free(machine);
}
