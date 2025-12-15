#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}


// ------------------ AND immediate mode, positive

// This function initializes the machine to a known state for the
// AND of a result that is positive in immediate mode
static x16_t* setup_test_machine_imm_pos() {
    x16_t* machine = x16_create();

    int CODESTART = 300;
    x16_memwrite(machine, CODESTART, emit_and_imm(R_R3, R_R2, 13));
    x16_set(machine, R_R2, 15);
    x16_set(machine, R_R3, 4411);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}



// WB for AND in immediate mode
TEST_CASE("Control.WB.and_imm.pos", "[control.and]") {
    x16_t* machine = setup_test_machine_imm_pos();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The result should now be available in the destination register
    REQUIRE(x16_reg(machine, R_R3) == (15 & 13));
    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_POS);

    x16_free(machine);
}

// -------------------- ADD immediate mode, negative

// This function initializes the machine to a known state for the
// AND of a result that is negative in immediate mode
static x16_t* setup_test_machine_imm_neg() {
    x16_t* machine = x16_create();

    // AND R3, R2, -13  with -10 in R2
    int CODESTART = 300;
    x16_memwrite(machine, CODESTART, emit_and_imm(R_R3, R_R2, -13));
    x16_set(machine, R_R2, -10);
    x16_set(machine, R_R3, 4411);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}


// WB for AND in immediate mode
TEST_CASE("Control.WB.and_imm.neg", "[control.and]") {
    x16_t* machine = setup_test_machine_imm_neg();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The result should now be available in the destination register
    REQUIRE(x16_reg(machine, R_R3) == (uint16_t) (-10 & -13));

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_NEG);

    x16_free(machine);
}



// ------------------ AND register mode, positive

// This function initializes the machine to a known state for the
// AND of a result that is positive in register mode
static x16_t* setup_test_machine_reg_pos() {
    x16_t* machine = x16_create();

    int CODESTART = 300;
    x16_memwrite(machine, CODESTART, emit_and_reg(R_R3, R_R2, R_R7));
    x16_set(machine, R_R2, 15);
    x16_set(machine, R_R7, 5);
    x16_set(machine, R_R3, 4411);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for AND in register mode
TEST_CASE("Control.WB.and_reg.pos", "[control.and]") {
    x16_t* machine = setup_test_machine_reg_pos();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The result should now be available in the destination register
    REQUIRE(x16_reg(machine, R_R3) == (15 & 5));

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_POS);

    x16_free(machine);
}

// ------------------ AND register mode, zero

// This function initializes the machine to a known state for the
// AND of a result that is zero in register mode
static x16_t* setup_test_machine_reg_zro() {
    x16_t* machine = x16_create();

    int CODESTART = 300;
    x16_memwrite(machine, CODESTART, emit_and_reg(R_R3, R_R2, R_R7));
    x16_set(machine, R_R2, 1000);
    x16_set(machine, R_R7, 5);
    x16_set(machine, R_R3, 4411);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}


// WB for AND in register mode
TEST_CASE("Control.WB.and_reg.zro", "[control.and]") {
    x16_t* machine = setup_test_machine_reg_zro();

    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The result should now be available in the destination register
    REQUIRE(x16_reg(machine, R_R3) == 0);

    // The condition register should be set correctly
    REQUIRE(x16_reg(machine, R_COND) == FL_ZRO);

    x16_free(machine);
}
