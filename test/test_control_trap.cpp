#include "catch.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>

extern "C" {
#include "control.h"
#include "x16.h"
#include "instruction.h"
#include "trap.h"
#include "io.h"
}

// Beginning program counter
static int CODESTART = 300;

// ----------------- Test GETC ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_getc() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_trap(TRAP_GETC));
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

TEST_CASE("Control.trap.getc", "[control.trap]") {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        REQUIRE(false);
    }
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Make the stdin read from the pipe
        if (dup2(fd[0], 0) == -1) {
            fprintf(stderr, "Dup failed\n");
            abort();
        }

        // Create machine
        x16_t* machine = setup_test_machine_getc();

        if (execute_instruction(machine) != 0) {
            fprintf(stderr, "Single cycle failed\n");
            abort();
        }
        uint16_t c = x16_reg(machine, R_R0);
        fprintf(stderr, "Child received \'%c\'\n", (char) c);
        if (c != 'x') {
            exit(1);
        }
        uint16_t cond = x16_reg(machine, R_COND);
        if (cond != FL_POS) {
            exit(2);
        }

        x16_free(machine);

        exit(0);
    } else {
        // Parent
        // Send a character to the child
        fprintf(stderr, "Parent sending \'x\' to the child\n");
        dprintf(fd[1], "%c", 'x');

        int status;
        waitpid(pid, &status, 0);
        REQUIRE(WIFEXITED(status));
        REQUIRE(WEXITSTATUS(status) == 0);
    }
}

// ----------------- Test TRAPOUT ----------------------

// This function initializes the machine to a known state
static x16_t* setup_test_machine_out() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_trap(TRAP_OUT));
    x16_memwrite(machine, CODESTART + 1, emit_add_imm(R_R0, R_R0, 5));
    x16_memwrite(machine, CODESTART + 2, emit_trap(TRAP_OUT));
    x16_set(machine, R_R0, (uint16_t) '0');
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

TEST_CASE("Control.trap.out", "[control.trap]") {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        REQUIRE(false);
    }
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Flush stdout -- Catch has stuff stuck in the pipe
        fflush(stdout);

        if (dup2(fd[1], 1) == -1) {
            fprintf(stderr, "Dup failed\n");
            abort();
        }
        close(fd[0]);      // not using the read end

        // Create machine
        x16_t* machine = setup_test_machine_out();

        // Tick the machine to execute a trap
        if (execute_instruction(machine) != 0) {
            fprintf(stderr, "Single cycle failed\n");
            abort();
        }

        x16_free(machine);

        exit(0);
    } else {
        // Parent
        // Read a character from the child
        close(fd[1]);       // not using the write end
        char c = 0;
        if (read(fd[0], &c, 1) != 1) {
            perror("Read of pipe failed for out");
            REQUIRE(false);
        }
        REQUIRE(c == '0');

        int status;
        waitpid(pid, &status, 0);
        REQUIRE(WIFEXITED(status));
        REQUIRE(WEXITSTATUS(status) == 0);
    }
}


// --------------------  Test PUTS

// This function initializes the machine to a known state
static x16_t* setup_test_machine_puts() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_trap(TRAP_PUTS));
    x16_memwrite(machine, CODESTART + 1, emit_value('H'));
    x16_memwrite(machine, CODESTART + 2, emit_value('e'));
    x16_memwrite(machine, CODESTART + 3, emit_value('l'));
    x16_memwrite(machine, CODESTART + 4, emit_value('l'));
    x16_memwrite(machine, CODESTART + 5, emit_value('o'));
    x16_memwrite(machine, CODESTART + 6, emit_value(0));
    x16_set(machine, R_R0, (uint16_t) CODESTART + 1);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

TEST_CASE("Control.trap.puts", "[control.trap]") {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        REQUIRE(false);
    }
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Flush stdout -- Catch has stuff stuck in the pipe
        fflush(stdout);

        if (dup2(fd[1], 1) == -1) {
            fprintf(stderr, "Dup failed\n");
            abort();
        }
        close(fd[0]);      // not using the read end

        // Create machine
        x16_t* machine =  setup_test_machine_puts();

        // Tick the machine to execute the trap
        fprintf(stderr, "Child putting: Hello\n");
        if (execute_instruction(machine) != 0) {
            fprintf(stderr, "Single cycle failed\n");
            exit(1);
        }

        x16_free(machine);

        exit(0);
    } else {
        // Parent
        // Read a character from the child
        close(fd[1]);       // not using the write end
        char buf[10];
        memset(buf, 0, sizeof(buf));
        if (read(fd[0], buf, sizeof(buf)) == -1) {
            perror("Read of pipe failed for puts");
            REQUIRE(false);
        }
        printf("Parent read: %s\n", buf);
        REQUIRE(strcmp(buf, "Hello") == 0);

        int status;
        waitpid(pid, &status, 0);
        REQUIRE(WIFEXITED(status));
        REQUIRE(WEXITSTATUS(status) == 0);
    }
}

// ----------------------- Test HALT


// This function initializes the machine to a known state
static x16_t* setup_test_machine_halt() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_add_imm(R_R1, R_R1, 5));
    x16_memwrite(machine, CODESTART + 1, emit_trap(TRAP_HALT));
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

TEST_CASE("Control.trap.halt", "[control.trap]") {
    // Create machine
    x16_t* machine =  setup_test_machine_halt();
    int rv;
    rv = execute_instruction(machine);
    REQUIRE(rv == 0);

    // The register should be updated
    REQUIRE(x16_reg(machine, R_R1) == 5);

    // The machine should halt
    rv = execute_instruction(machine);
    REQUIRE(rv == -1);

    x16_free(machine);
}

// --------------------- Test TRAP_IN

// This function initializes the machine to a known state
static x16_t* setup_test_machine_trapin() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_trap(TRAP_IN));
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

TEST_CASE("Control.trap.in", "[control.trap]") {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        REQUIRE(false);
    }
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Make the stdin read from the pipe
        if (dup2(fd[0], 0) == -1) {
            fprintf(stderr, "Dup failed\n");
            abort();
        }

        // Create machine
        x16_t* machine = setup_test_machine_trapin();
        if (execute_instruction(machine) != 0) {
            fprintf(stderr, "Single cycle failed\n");
            abort();
        }
        uint16_t c = x16_reg(machine, R_R0);
        if (c != 'k') {
            exit(1);
        }
        uint16_t cond = x16_reg(machine, R_COND);
        if (cond != FL_POS) {
            exit(1);
        }

        x16_free(machine);

        exit(0);
    } else {
        // Parent
        // Send a character to the child
        dprintf(fd[1], "%c", 'k');

        int status;
        waitpid(pid, &status, 0);
        REQUIRE(WIFEXITED(status));
        REQUIRE(WEXITSTATUS(status) == 0);
    }
}


//  --------------------  Test PUTSP

// This function initializes the machine to a known state
static x16_t* setup_test_machine_putsp() {
    x16_t* machine = x16_create();

    x16_memwrite(machine, CODESTART, emit_trap(TRAP_PUTSP));
    x16_memwrite(machine, CODESTART + 1, emit_value(0xfeca));
    x16_memwrite(machine, CODESTART + 2,emit_value(0xbeba));
    x16_memwrite(machine, CODESTART + 3, emit_value(0));
    x16_memwrite(machine, CODESTART + 6, emit_value(0));
    x16_set(machine, R_R0, (uint16_t) CODESTART + 1);
    x16_set(machine, R_PC, CODESTART);

    return machine;
}

TEST_CASE("Control.trap.putsp", "[control.trap]") {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("Pipe creation failed");
        REQUIRE(false);
    }
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Flush stdout -- Catch has stuff stuck in the pipe
        fflush(stdout);

        if (dup2(fd[1], 1) == -1) {
            fprintf(stderr, "Dup failed\n");
            abort();
        }
        close(fd[0]);      // not using the read end

        // Create machine
        x16_t* machine = setup_test_machine_putsp();

        // Tick the machine to execute the trap
        if (execute_instruction(machine) != 0) {
            fprintf(stderr, "Single cycle failed\n");
            abort();
        }

        x16_free(machine);

        exit(0);
    } else {
        // Parent
        // Read a character from the child
        close(fd[1]);       // not using the write end
        char buf[10];
        memset(buf, 0, sizeof(buf));
        if (read(fd[0], buf, sizeof(buf)) == -1) {
            perror("Read of pipe failed for puts");
            REQUIRE(false);
        }
        REQUIRE(buf[0] == (char) 0xca);
        REQUIRE(buf[1] == (char) 0xfe);
        REQUIRE(buf[2] == (char) 0xba);
        REQUIRE(buf[3] == (char) 0xbe);

        int status;
        waitpid(pid, &status, 0);
        REQUIRE(WIFEXITED(status));
        REQUIRE(WEXITSTATUS(status) == 0);
    }
}
