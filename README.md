# X16 Assembler and Emulator

A complete implementation of the X16 computer architecture, featuring an assembler (`xas`) and emulator (`x16`) for educational and development purposes as part of my computer systems class final project.

You can play 2048 on it!

<img width="283" height="233" alt="image" src="https://github.com/user-attachments/assets/27fa13c3-6349-4fca-9620-7b49b1730419" />

## Overview

The X16 is a 16-bit computer architecture with:

- 16-bit memory addressing (65,536 memory locations)
- 10 registers (8 general-purpose + PC + COND)
- RISC-style instruction set with 16 opcodes
- Support for assembly programming with labels and directives

## Features

- **Assembler (`xas`)**: Converts X16 assembly files (.x16s) to executable binary object files (.obj)
- **Emulator (`x16`)**: Executes compiled X16 object files with full instruction set support
- **Disassembler (`xod`)**: Converts object files back to assembly (reverse engineering)
- **Comprehensive Testing**: Full test suite covering all instructions and components

## Architecture Specifications

### Memory

- **Address Space**: 16-bit (0-65535)
- **Default Code Start**: 0x3000
- **Word Size**: 16 bits

### Registers

- **R0-R7**: General-purpose registers
- **R_PC**: Program Counter
- **R_COND**: Condition Flags (POS/ZRO/NEG)

### Instruction Set

- **Arithmetic**: ADD, AND, NOT
- **Memory**: LD, LDI, LDR, LEA, ST, STI, STR
- **Control**: BR, JMP, JSR, RET
- **System**: TRAP, HALT, GETC, OUT, PUTS, IN, PUTSP

## Installation

### Prerequisites

- GCC compiler
- Make utility

### Build Instructions

```bash
# Build all components
make

# Build emulator only
make x16

# Build assembler only
make xas

# Build disassembler only
make xod

# Build test suite
make test-build
```

## Usage

### Assembler (xas)

```bash
# Assemble assembly file to object file
./xas program.x16s

# This creates program.obj
```

### Emulator (x16)

```bash
# Run object file
./x16 program.obj

# Run with execution tracing
./x16 -l program.obj
# Creates log.txt with execution trace

# Run default file (a.obj)
./x16
```

### Disassembler (xod)

```bash
# Disassemble object file back to assembly
./xod program.obj
```

## Assembly Language Guide

### Basic Syntax

```assembly
# Comments start with #
label:          # Labels end with colon
    add %r1, %r0, $10    # Add immediate to register
    brz done             # Branch if zero
done:
    halt                 # Stop execution
```

### Instruction Formats

- **Register**: `add %r1, %r2, %r3` (R1 = R2 + R3)
- **Immediate**: `add %r1, %r2, $10` (R1 = R2 + 10)
- **Memory**: `ld %r1, label` (Load from memory)
- **Branch**: `brzp target` (Branch if positive or zero)

### Directives

- `label:` - Define a label
- `val $123` - Define a data value
- `# comment` - Comments

## File Formats

### Assembly Files (.x16s)

- Text files with X16 assembly instructions
- Support labels, comments, and directives
- Example: See `giza.x16s` for a complete program

### Object Files (.obj)

- Binary format with 16-bit values
- First 16 bits: origin address (network byte order)
- Remaining: 16-bit instructions (network byte order)

## Games

You can run 2048 on the emulator:

```bash
./x16 2048.obj
```
