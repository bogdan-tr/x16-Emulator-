#include "bits.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
// Get the nth bit
uint16_t getbit(uint16_t number, int n) {
  uint16_t mask = 1 << n;
  uint16_t result = (number & mask) >> n;
  return result;
}

// Get bits that are the given number of bits wide
uint16_t getbits(uint16_t number, int n, int wide) {
  uint16_t mask = ((1 << wide) - 1) << n;
  uint16_t result = (number & mask) >> n;
  return result;
}

// Set the nth bit to the given bit value and return the result
uint16_t setbit(uint16_t number, int n) {
  uint16_t mask = 1 << n;
  uint16_t result = (number | mask);
  return result;
}

// Clear the nth bit
uint16_t clearbit(uint16_t number, int n) {
  uint16_t mask = 1 << n;
  uint16_t result = number & ~mask;
  return result;
}

// Sign extend a number of the given bits to 16 bits
uint16_t sign_extend(uint16_t x, int bit_count) {
  uint16_t result;
  uint16_t msbmask = 1 << (bit_count - 1);  // most significant bit
  if (msbmask & x) {
    uint16_t mask = ((1 << (16 - (bit_count))) -
                     1);  // make bits to the left of bit_count 1s
    mask = mask << bit_count;
    result = x | mask;
  } else {
    result = x;  // if positive do nothing
  }
  return result;
}

bool is_positive(uint16_t number) { return getbit(number, 15) == 0; }

bool is_negative(uint16_t number) { return getbit(number, 15) == 1; }
