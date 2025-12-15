#pragma once

#include <stdint.h>

// Decode instruction and return a pointer to an internal buffer
// that shows the representation of the instruction
char* decode(uint16_t instruction);
