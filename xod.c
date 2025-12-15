#define _GNU_SOURCE
#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "decode.h"
#include "instruction.h"

void usage() {
  fprintf(stderr, "Usage: ./xod file\n");
  exit(1);
}

int main(int argc, char** argv) {
  if (argc > 2) {
    usage();
  }

  char* filename = "a.obj";
  if (argc == 2) {
    filename = argv[1];
  }

  FILE* fp = fopen(filename, "rb");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open %s\n", filename);
    exit(2);
  }

  uint16_t origin;
  if (fread(&origin, sizeof(origin), 1, fp) != 1) {
    fprintf(stderr, "Can't read origin\n");
    exit(2);
  }
  origin = ntohs(origin);

  printf("Origin: 0x%x\n", origin);
  int location = origin;
  uint16_t instruction;
  while (fread(&instruction, sizeof(instruction), 1, fp) == 1) {
    instruction = ntohs(instruction);
    printf("0x%x: ", location);
    print_instruction(instruction);
    char* str = decode(instruction);
    printf(" : %s\n", str);
    free(str);
    location++;
  }

  fclose(fp);
}
