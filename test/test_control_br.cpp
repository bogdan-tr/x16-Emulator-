#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}

 // Beginning program counter
static int CODESTART = 400;

// ----------------- Test branch P taken ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_br(int offset) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(false, false, true, offset));
    x16_set(machine, R_COND, FL_POS);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.br.taken", "[control.br]") {
    x16_t* machine = setup_test_machine_br(42);

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // This is a no op for a branch
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1 + 42);

    x16_free(machine);

    x16_t* machine2 = setup_test_machine_br(-3);

    int rv2 = execute_instruction(machine2);
    REQUIRE(rv == 0);

    // This is a no op for a branch
    REQUIRE(x16_reg(machine2, R_PC) == CODESTART + 1 - 3);

    x16_free(machine2);
}

// ----------------- Test branch N not taken ----------------------

// This function initializes the machine to a known state for the
// branch that is not taken
static x16_t* setup_test_machine_brnottaken() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(true, false, false, 42));
    x16_set(machine, R_COND, FL_POS);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.br.nottaken", "[control.br]") {
    x16_t* machine = setup_test_machine_brnottaken();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // This is a no op for a branch not taken
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    x16_free(machine);
}

// ----------------- Test branch for Z not taken ----------------------

// This function initializes the machine to a known state for the
// branch that is not taken
static x16_t* setup_test_machine_brz_nottaken() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(false, true, false, 42));
    x16_set(machine, R_COND, FL_POS);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.brz.nottaken", "[control.br]") {
    x16_t* machine = setup_test_machine_brz_nottaken();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // This is a no op for a branch not taken
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    x16_free(machine);
}

// ----------------- Test branch for Z taken ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_brz_taken(int offset) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(false, true, false, offset));
    x16_set(machine, R_COND, FL_ZRO);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.brz.taken", "[control.br]") {
    x16_t* machine = setup_test_machine_brz_taken(-33);

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // This is a no op for a branch not taken
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1 -33);

    x16_free(machine);
}



// ----------------- Test branch for NZ not taken ----------------------

// This function initializes the machine to a known state for the
// branch that is not taken
static x16_t* setup_test_machine_brnz_nottaken() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(true, true, false, 42));
    x16_set(machine, R_COND, FL_POS);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.brnz.nottaken", "[control.br]") {
    x16_t* machine = setup_test_machine_brnz_nottaken();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // This is a no op for a branch not taken
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    x16_free(machine);
}

// ----------------- Test branch for NZ taken ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_brnz_taken() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(true, true, false, 42));
    x16_set(machine, R_COND, FL_NEG);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.brnz.taken", "[control.br]") {
    x16_t* machine = setup_test_machine_brnz_taken();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // This is a no op for a branch not taken
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1 + 42);

    x16_free(machine);
}

// ----------------- Test branch for ZP not taken ----------------------

// This function initializes the machine to a known state for the
// branch that is not taken
static x16_t* setup_test_machine_brzp_nottaken() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(false, true, true, 42));
    x16_set(machine, R_COND, FL_NEG);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.brzp.nottaken", "[control.br]") {
    x16_t* machine = setup_test_machine_brzp_nottaken();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // This is a no op for a branch not taken
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1);

    x16_free(machine);
}

// ----------------- Test branch for ZP taken ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_brzp_taken() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(false, true, true, 42));
    x16_set(machine, R_COND, FL_POS);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.brzp.taken", "[control.br]") {
    x16_t* machine = setup_test_machine_brzp_taken();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1 + 42);

    x16_free(machine);
}

// ----------------- Test brnzp branch ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_brnzp_taken(uint16_t cond, int offset) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(true, true, true, offset));
    x16_set(machine, R_COND, cond);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.brnzp.taken", "[control.br]") {
    x16_t* machine = setup_test_machine_brnzp_taken(FL_NEG, 42);
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1 + 42);
    x16_free(machine);

    x16_t* machine2 = setup_test_machine_brnzp_taken(FL_ZRO, -25);
    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);
    REQUIRE(x16_reg(machine2, R_PC) == CODESTART + 1 -25);
    x16_free(machine2);

    x16_t* machine3 = setup_test_machine_brnzp_taken(FL_POS, 66);
    int rv3 = execute_instruction(machine3);
    REQUIRE(rv3 == 0);
    REQUIRE(x16_reg(machine3, R_PC) == CODESTART + 1 + 66);
    x16_free(machine3);

    for (int i = -256; i <= 255; i++) {
        x16_t* machine4 = setup_test_machine_brnzp_taken(FL_POS, i);
        int rv4 = execute_instruction(machine4);
        REQUIRE(rv4 == 0);
        REQUIRE(x16_reg(machine3, R_PC) == CODESTART + 1 + i);
        x16_free(machine4);
    }
}


// ----------------- Test br branch ----------------------

// This function initializes the machine to a known state for the
// branch that is taken
static x16_t* setup_test_machine_br_uncond_taken(uint16_t cond, int offset) {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_br(false, false, false, offset));
    x16_set(machine, R_COND, cond);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for BR
TEST_CASE("Control.WB.br.uncond.taken", "[control.br]") {
    x16_t* machine = setup_test_machine_br_uncond_taken(FL_NEG, 42);
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);
    REQUIRE(x16_reg(machine, R_PC) == CODESTART + 1 + 42);
    x16_free(machine);

    x16_t* machine2 = setup_test_machine_br_uncond_taken(FL_ZRO, -25);
    int rv2 = execute_instruction(machine2);
    REQUIRE(rv2 == 0);
    REQUIRE(x16_reg(machine2, R_PC) == CODESTART + 1 -25);
    x16_free(machine2);

    // 9 bit offsets are -256 to 255
    for (int i = -256; i <= 255; i++) {
        x16_t* machine3 = setup_test_machine_br_uncond_taken(FL_POS, i);
        int rv3 = execute_instruction(machine3);
        REQUIRE(rv3 == 0);
        REQUIRE(x16_reg(machine3, R_PC) == CODESTART + 1 + i);
        x16_free(machine3);
    }
}
