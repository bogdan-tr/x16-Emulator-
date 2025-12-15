#include "catch.hpp"

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
}

// ------------------ ADD immediate mode, positive

// This function initializes the machine to a known state for the
// addition of a result that is positive
static x16_t* setup_test_machine_imm_pos() {
    x16_t* machine = x16_create();
    int CODESTART = 300;
    x16_memwrite(machine, CODESTART, emit_add_imm(R_R4, R_R2, 13));
    x16_set(machine, R_R2, 15);
    x16_set(machine, R_R4, 4411);
    x16_set(machine, R_PC, CODESTART);
    return machine;
}


// MEM for ADD in immediate mode
TEST_CASE("Control.MEM.add_imm.pos", "[control.add]") {
    x16_t* machine = setup_test_machine_imm_pos();

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    REQUIRE(x16_reg(machine, R_R4) == 15 + 13);
    REQUIRE(x16_cond(machine) == FL_POS);

    x16_free(machine);
}

// This function initializes the machine to a known state for the
// addition of a result that is negative
static x16_t* setup_test_machine_imm_neg() {
    x16_t* machine = x16_create();
    int CODESTART = 300;
    x16_memwrite(machine, CODESTART, emit_add_imm(R_R4, R_R2, -8));
    x16_set(machine, R_R2, 5);
    x16_set(machine, R_R4, 4411);
    x16_set(machine, R_PC, CODESTART);
    return machine;
}

// WB for ADD in immediate mode
TEST_CASE("Control.WB.add_imm.neg", "[control.add]") {
    x16_t* machine = setup_test_machine_imm_neg();

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The result should now be available in the destination register
    REQUIRE(x16_reg(machine, R_R4) == (uint16_t) (5 - 8));
    // The condition register should be set correctly
    REQUIRE(x16_cond(machine) == FL_NEG);

    x16_free(machine);
}



// ------------------ ADD register mode, positive

// This function initializes the machine to a known state for the
// addition of a result that is positive in register mode
static x16_t* setup_test_machine_reg_pos() {
    x16_t* machine = x16_create();

    int CODESTART = 300;
    x16_memwrite(machine, CODESTART, emit_add_reg(R_R4, R_R2, R_R7));
    x16_set(machine, R_R2, 15);
    x16_set(machine, R_R7, 5);
    x16_set(machine, R_R4, 4411);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

//  ADD in immediate mode
TEST_CASE("Control.WB.add_reg.pos", "[control.add]") {
    x16_t* machine = setup_test_machine_reg_pos();

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The result should now be available in the destination register
    REQUIRE(x16_reg(machine, R_R4) == 20);

    // The condition register should be set correctly
    REQUIRE(x16_cond(machine) == FL_POS);

    x16_free(machine);
}

// -------------------- ADD register mode, zero

// This function initializes the machine to a known state for the
// addition of a result that is positive
static x16_t* setup_test_machine_reg_zro() {
    x16_t* machine = x16_create();

    // ADD R4, R2, R7  with -10 in R2
    int CODESTART = 300;
    x16_memwrite(machine, CODESTART, emit_add_reg(R_R4, R_R2, R_R7));
    x16_set(machine, R_R2, 10);
    x16_set(machine, R_R7, -10);
    x16_set(machine, R_R4, 4411);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

// WB for ADD in reg mode
TEST_CASE("Control.WB.add_reg.zro", "[control.add]") {
    x16_t* machine = setup_test_machine_reg_zro();

    // Execution should be successful for one instruction
    int rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The result should now be available in the destination register
    REQUIRE(x16_reg(machine, R_R4) == 0);

    // The condition register should be set correctly
    REQUIRE(x16_cond(machine) == FL_ZRO);

    x16_free(machine);
}
