#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instruction.h"
#include "trap.h"
#include "x16.h"
#define REG 1111        // code for register
#define IMM 2222        // code for immediate
#define INST 3333       // code for instruction
#define LABEL 4444      // code for labels
#define ASSEMBLYERROR 2 // code for error in assembly
#define FILEERROR 1     // code for file error (e.g no file specified)
#define SUCCESS 0
#define LINESIZE 200
#define START 0x3000
// stores where all the labels are located, necessary for jumping
// to work
typedef struct {
  char *labelName;       // e.g label:
  uint16_t labelAddress; // label location
} labelTable;

// store collected data about the file here
typedef struct {
  uint16_t *binaryInstructions; // array of binary instructions
  int numInstructions;          // track how many instructions we have
  labelTable *table;            // table of all the labels
  int numLabels;                // number of labels
  uint16_t currentAddress;      // keep track of where we are now
} fileData;

uint16_t assembleInstructionfromMetaData(opcode_t opcode, int numTokens,
                                         int registerCount, reg_t *reg1,
                                         reg_t *reg2, reg_t *reg3,
                                         uint16_t ImmOffsetVal, bool neg,
                                         bool zero, bool pos, bool isADDwithImm,
                                         bool isANDwithImm, bool isRet,
                                         bool isJsrR, int *ErrorCode);

void printLabelTable(fileData *data);
bool detectValidLabel(char *tokenizedStr, fileData *data);
uint16_t parseValidLabel(char *tokenizedStr, fileData *data);
void determineisJsrR(char *tokenizedStr, bool *isJsrR);
void determineisRet(char *tokenizedStr, bool *isRet);
void extractNZP(char *tokenizedStr, bool *n, bool *z, bool *p);
opcode_t parseValidInstruction(char *tokenizedStr);
reg_t parseRegister(char *tokenizedStr);
uint16_t parseImmediate(char *tokenizedStr);
int identifyType(char *tokenizedStr, fileData *data);
void cleanToken(char *tokenizedStr);
bool detectRegister(char *tokenizedStr);
bool detectImmediate(char *tokenizedStr);
bool detectValidInstruction(char *tokenizedStr);
uint16_t processVal(char *line);
char *processLabel(char *line);
uint16_t processInstruction(char *line, int *ErrorCode, fileData *data);
bool detectVal(char *line);
void delSpace(char *line);
int parseLine(char *line, fileData *data, int *ErrorCode);
void delComment(char *line);
bool detectLabel(char *line);
fileData *initfileData(int labelCount, int lineCount);
int countLabels(FILE *file, int *lineCount);
void addLabelsToTable(FILE *file, fileData *data);

void usage() {
  fprintf(stderr, "Usage: ./xas file");
  exit(1);
}
int labelCount = 0;
int lineCount = 0;
int ErrorCode = 0;
int main(int argc, char **argv) {
  if (argc != 2) {
    usage();
    return 1; // NOTE: not sure if this is necessary
  }
  labelCount = 0; // count the total number of labels in file
  // grab the file
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    fprintf(stderr, "Cannot open %s\n", argv[1]);
    return FILEERROR;
  }
  // calculate number of labels;
  labelCount = countLabels(file, &lineCount);
  fseek(file, 0, SEEK_SET); // move file pointer back to beginning
  // initiallize struct that will store all the data
  fileData *data = initfileData(labelCount, lineCount);
  addLabelsToTable(file, data);
  data->currentAddress = START;
  fseek(file, 0, SEEK_SET);
  // main loop that will parse the file MAINLOOP
  char line[LINESIZE];
  while (fgets(line, sizeof(line), file) != NULL) { // iterate thought ines
    // clean up the line
    delComment(line); // delete comments
    delSpace(line);   // delete leading and trailing spaces
    if (ErrorCode == 2 || parseLine(line, data, &ErrorCode) == ASSEMBLYERROR) {
      printf("ERROR: Invalid assembly (in main)\n");
      return ASSEMBLYERROR;
    }
    //   // only increment if we processed an actual instruction/val
    if (strlen(line) > 0 && !detectLabel(line)) {
      data->currentAddress += 2; // each instruction is 2 bytes (as we have
      // a 16 bit comp)
    }
  }
  // WRITE TO OUTPUT FILE:
  int totalInstructions = data->numInstructions;
  FILE *outputFile = fopen("a.obj", "wb"); // write binary
  uint16_t origin = htons(START);
  fwrite(&origin, sizeof(uint16_t), 1, outputFile);
  for (int i = 0; i < totalInstructions; i++) {
    uint16_t instruction =
        htons(data->binaryInstructions[i]); // convert to network byte order
    fwrite(&instruction, sizeof(uint16_t), 1, outputFile);
  }
  // Free and close the file
  for (int i = 0; i < data->numLabels; i++) {
    free(data->table[i].labelName); // strdup uses malloc, so need to free
  }
  free(data->binaryInstructions);
  free(data->table);
  free(data);
  fclose(outputFile);
  return SUCCESS;
}

// NOTE: go though table to count number of labels, after that use that to
// malloc space for table and init the actually assembler file

// NOTE: for comment processing for loop each line upto '\0' or '\n' or '#',
// if one of those symbols found the line ends.  (parseLine design plan) also,
// in each line look for certain flags such as val, $, %, use commas/space? as
// delimeter to split up the line and detect commands and labels
//
// NOTE: EXAMPLE: use emit convert to binary instruction (once each part is
// parsed) char line  "add %r1, %r0, $10"
// dst should equal 1? (this is of the reg type)
// src1 should equal 0?
// immediate should equal 10?
// then pass that into correct imit function to get the instruction

fileData *initfileData(int labelCount, int lineCount) {
  fileData *data = malloc(sizeof(fileData));
  labelTable *table = malloc(labelCount * sizeof(labelTable));
  data->numInstructions = 0;
  data->table = table;
  data->numLabels = 0;
  data->currentAddress = 0x3000; // start here
  data->binaryInstructions = malloc(lineCount * sizeof(uint16_t) * 2);
  return data;
}

int countLabels(FILE *file, int *lineCount) {
  char line[LINESIZE];
  int labelCount = 0;
  while (fgets(line, sizeof(line), file) != NULL) { // iterate thought ines
    if (detectLabel(line)) {
      labelCount++;
    }
    (*lineCount)++; // count total number of lines
  }
  return labelCount;
}
// function that adds labels to the table
void addLabelsToTable(FILE *file, fileData *data) {
  char line[LINESIZE];
  long originalPosition = ftell(file); // save original posiiton
  fseek(file, 0, SEEK_SET);            // move to the beginning of the file
  uint16_t tempAddress = START;
  while (fgets(line, sizeof(line), file) != NULL) { // iterate thought ines
    delComment(line);                               // clean up the line
    delSpace(line);
    // copy the line without breaking original:
    char lineCopy[LINESIZE];
    strcpy(lineCopy, line);
    if (detectLabel(lineCopy)) { // if label detected
      char *processedLabel = processLabel(lineCopy);
      data->table[data->numLabels].labelName =
          strdup(processedLabel); // add label to the table // WARN: uses
                                  // malloc, will need to free
      data->table[data->numLabels].labelAddress = tempAddress;
      data->numLabels++;
      // printf("addLabelsToTable: Total labels: %d\n", data->numLabels); //
      // DEBUG:
    }
    // Increment address only for actuall instructions or val
    if (!detectLabel(line) && strlen(line) > 0) {
      tempAddress += 2; // each instruction is 2 bytes (16 bit computer)
    }
  }
  fseek(file, originalPosition, SEEK_SET); // move back to where we started
}
// main function that parses the line, calls the helper functions below
int parseLine(char *line, fileData *data, int *ErrorCode) {
  if (strlen(line) == 0) {
    return SUCCESS; // empty lines are fine
  }
  if (detectLabel(line)) {
    // do nothing as addLabelsToTable already handled this
    //  label has no instructions
    //  line = processLabel(line); // remove ":"
    //  data->table[data->numLabels].labelName =
    //      strdup(line); // add label (making sure it is a clean string)
    //  data->table[data->numLabels].labelAddress = data->currentAddress;
    //  data->numLabels++; // keep track of number of labels
  } else if (detectVal(line)) {
    // deal with val
    uint16_t instruction = processVal(line);
    data->binaryInstructions[data->numInstructions] =
        instruction;         // add instruction to arrray
    data->numInstructions++; // increment instruction count;
  } else {                   // this is an instruction
    // furhter dealings with labels are in the processInstruction function
    uint16_t instruction =
        processInstruction(line, ErrorCode, data); // instruction can be ERROR
    if (instruction == ASSEMBLYERROR) {
      return ASSEMBLYERROR;
    }
    data->binaryInstructions[data->numInstructions] =
        instruction;         // add instruction to arrray
    data->numInstructions++; // increment instruction count;
  }
  return SUCCESS;
}
// deletes comments from the line
void delComment(char *line) {
  char *commentPointer = strchr(line, '#'); // get address pointer to comment
  if (commentPointer != NULL) {
    *commentPointer = '\0';
  }
}
void delSpace(char *line) {
  if (line == NULL) { // make sure line is valid
    return;
  }

  // trim leading space
  char *trimmedLine =
      line +
      strspn(line, " \t\n"); // strspn return lenght of substring where arg2
                             // is present, we then point to where that ends
  if (trimmedLine != line) {
    // strcpy(line, trimmedLine);
    memmove(line, trimmedLine, strlen(trimmedLine) + 1);
  }
  // trim trailing space
  size_t len = strlen(trimmedLine);
  while (len > 0 && strchr(" \t\n", trimmedLine[len - 1]) != NULL) {
    // iterate backwards, if space detect, replace it with \0
    trimmedLine[len - 1] = '\0';
    len--;
  }
}
// detect the labels e.g start:
bool detectLabel(char *line) {
  for (char *charachter = line; *charachter != '\0'; charachter++) {
    if (*charachter == ':') {
      return true;
    }
  }
  return false;
}
// detect if this line is a val
bool detectVal(char *line) {
  if (strstr(line, "val") != NULL) { // strstr search for "needle" in "haystack"
    return true;
  }
  return false;
}

// turn line with val into instruction
uint16_t processVal(char *line) {
  char *strInt = strchr(line, '$');
  if (strInt != NULL) {
    strInt++; // move strInt past $ to where the number starts
  }
  uint16_t val = (uint16_t)atoi(strInt); // cast strInt to uint16_t
  return val;
}
// turn line with label into just label (e.g remove the ":")
char *processLabel(char *line) {
  char *colon = strchr(line, ':');
  if (colon != NULL) {
    *colon = '\0';
  }
  return line;
}

// turn line with instruction into binary instruction
uint16_t
processInstruction(char *line, int *ErrorCode,
                   fileData *data) { // returns ERROR if instruction is invalid
  //  PLAN:
  //  1. split instruction into sections e.g add %r1, %r0, $10 into 4 parts
  //  2. parse each part separetly
  //  3. combine results into single hexcode (convert labels into address?)
  int numTokens = 0; // keep track of total amount of tokens (for identifying
                     // specific optype)
  int registerCount = 0; // keep track of registers
  // split instruction:
  char stringCopy[100];
  strcpy(stringCopy, line);
  char *tokenizedStr =
      strtok(stringCopy,
             " \t"); // returns pointer to next token (split by " " or " \t")
  char *instructionToken =
      NULL; // tokenizedStr was NULL at the end of the, tokenizedStr useful
            // only if it contains the instructionToken
  reg_t reg1_val, reg2_val, reg3_val;
  reg_t *reg1 = &reg1_val;
  reg_t *reg2 = &reg2_val;
  reg_t *reg3 = &reg3_val;
  opcode_t opcode;
  bool neg = false;
  bool zero = false;
  bool pos = false;
  uint16_t ImmOffsetVal = 0; // this could be any of these: offset, value, src

  // OPERATION FLAGS:
  bool isADDwithImm; // is the add operation with immediate (not a 3rd
                     // register)
  bool isANDwithImm; // is the and operation with immediate (not a 3rd
                     // register)
  bool isRET;        // is this a ret operation (not a jmp)

  bool isjsrR; // is this jsrr (not jsr)
  // loop through tokens, detect component type, then process
  while (tokenizedStr != NULL) {
    // this is a subcomponent of the instruction
    cleanToken(tokenizedStr); // clean the tokenizedStr;
    // store collected data here: (defaults necessary for extracting flags)
    int Type = identifyType(tokenizedStr, data);
    switch (Type) {
    case REG:
      // fill up corresponding register information
      registerCount++;
      switch (registerCount) {
      case 1:
        *reg1 = parseRegister(tokenizedStr);
        break;
      case 2:
        *reg2 = parseRegister(tokenizedStr);
        break;
      case 3:
        *reg3 = parseRegister(tokenizedStr);
        break;
      }
      break;
    case IMM:
      // fill up value
      ImmOffsetVal = parseImmediate(tokenizedStr);
      break;
    case INST:
      // get instruction operation code and add necessary flags
      opcode = parseValidInstruction(tokenizedStr);
      instructionToken = tokenizedStr;
      break;
    case LABEL:
      // printf("processInstruction: label found: %s\n", tokenizedStr); //
      // DEBUG: printf("processInstruction: calling parseValidLabel"); //
      // DEBUG:
      ImmOffsetVal = parseValidLabel(tokenizedStr, data);
      break;
    case ASSEMBLYERROR:

      // // DBG: start
      // printf(
      //     "Error: Invalid/Unrecognized Instruction (in
      //     processInstruction)\n");
      // printf("Instruction: %s\n", line);
      // printf("Token: %s\n", tokenizedStr);
      // // DBG: end
      *ErrorCode = ASSEMBLYERROR;
      return ASSEMBLYERROR;
    }
    tokenizedStr =
        strtok(NULL, " \t"); // strtok remember previous string, passing in NULL
                             // tells it to continue with the previous string
    numTokens++;             // increment token count
  }
  // after the parts are collected, use emit stuff and metadata (might need to
  // add certain flags) to get the final instruction
  // we should have the necessary information to use emit operation
  // fix: flag collection moved outside while loop
  tokenizedStr = instructionToken; // put instruction token back
  // FLAG COLLECTION
  if (opcode == OP_ADD) {
    //  differentiate between two types of add
    if (ImmOffsetVal != 0) { // this means this is add with immediate
      isADDwithImm = true;
    } else {
      isADDwithImm = false;
    }
  } else if (opcode == OP_AND) {
    // differentiate between two types of and
    if (ImmOffsetVal != 0) { // this means this is add with immediate
      isANDwithImm = true;
    } else {
      isANDwithImm = false;
    }
  } else if (opcode == OP_BR) {
    //  extract nzp flags
    extractNZP(tokenizedStr, &neg, &zero, &pos);
  } else if (opcode == OP_JMP) {
    //  determine if ret or jmp
    determineisRet(tokenizedStr, &isRET);
  } else if (opcode == OP_JSR) {
    //  determine if jsr or jsrr
    determineisJsrR(tokenizedStr, &isjsrR);
  } else if (opcode == OP_TRAP) {
    if (strcasecmp(tokenizedStr, "getc") == 0) {
      ImmOffsetVal = TRAP_GETC;
    }
    if (strcasecmp(tokenizedStr, "putc") == 0) {
      ImmOffsetVal = TRAP_OUT;
    }
    if (strcasecmp(tokenizedStr, "puts") == 0) {
      ImmOffsetVal = TRAP_PUTS;
    }
    if (strcasecmp(tokenizedStr, "enter") == 0) {
      ImmOffsetVal = TRAP_IN;
    }
    if (strcasecmp(tokenizedStr, "putsp") == 0) {
      ImmOffsetVal = TRAP_PUTSP;
    }
    if (strcasecmp(tokenizedStr, "halt") == 0) {
      ImmOffsetVal = TRAP_HALT;
    }
  }
  uint16_t instruction = assembleInstructionfromMetaData(
      opcode, numTokens, registerCount, reg1, reg2, reg3, ImmOffsetVal, neg,
      zero, pos, isADDwithImm, isANDwithImm, isRET, isjsrR, ErrorCode);
  return instruction; // NOTE: instruction can be ERROR!
}

// Remove whitespace and comma ("%r10, " -> "%r10")
void cleanToken(char *tokenizedStr) {
  delSpace(tokenizedStr); // delete leading and trailing spaces
                          // WARN: Naive implementation, assumes comma is
                          // last, should be the case though
  char *comma = strchr(tokenizedStr, ',');
  if (comma != NULL) {
    *comma = '\0';
  }
}

// detect if a cleaned tokenizedStr is a register
bool detectRegister(char *tokenizedStr) {
  if (strstr(tokenizedStr, "%r") != NULL) {
    return true;
  }
  return false;
}
// detect if a cleaned tokenizedStr is a immediate value (e.g $10)
bool detectImmediate(char *tokenizedStr) {
  if (strstr(tokenizedStr, "$") != NULL) {
    return true;
  }
  return false;
}

// returns code for the type of operation a clean tokenizedStr is
// usefull for clean switch statement
int identifyType(char *tokenizedStr, fileData *data) {
  if (detectRegister(tokenizedStr)) {
    return REG;
  }
  if (detectImmediate(tokenizedStr)) {
    return IMM;
  }
  if (detectValidInstruction(tokenizedStr)) {
    return INST;
  }
  if (detectValidLabel(tokenizedStr, data)) {
    return LABEL;
  }
  // if (strstr(tokenizedStr, "%") == NULL && strstr(tokenizedStr, "$") == NULL
  // &&
  //     !detectValidInstruction(tokenizedStr)) {
  //   return LABEL; // assume label if not other stuff
  // }
  return ASSEMBLYERROR; // should never happen
}
// detect if a cleaned tokenizedStr is a valid instruction (e.g ld)
bool detectValidInstruction(char *tokenizedStr) {
  if (strcasestr(tokenizedStr, "ld") != NULL ||
      strcasestr(tokenizedStr, "add") != NULL ||
      strcasestr(tokenizedStr, "putsp") != NULL ||
      strcasestr(tokenizedStr, "and") != NULL ||
      (strcasestr(tokenizedStr, "br") == tokenizedStr &&
       strlen(tokenizedStr) <= 5) ||
      strcasestr(tokenizedStr, "jmp") != NULL ||
      strcasestr(tokenizedStr, "jsrr") != NULL ||
      strcasestr(tokenizedStr, "jsr") != NULL ||
      strcasestr(tokenizedStr, "ldi") != NULL ||
      strcasestr(tokenizedStr, "ldr") != NULL ||
      strcasestr(tokenizedStr, "lea") != NULL ||
      strcasestr(tokenizedStr, "not") != NULL ||
      strcasestr(tokenizedStr, "ret") != NULL ||
      strcasecmp(tokenizedStr, "sti") == 0 || // identify st exactly
      strcasecmp(tokenizedStr, "str") == 0 ||
      strcasecmp(tokenizedStr, "st") == 0 ||
      strcasestr(tokenizedStr, "trap") != NULL ||
      strcasestr(tokenizedStr, "halt") != NULL ||
      strcasestr(tokenizedStr, "getc") != NULL ||
      strcasestr(tokenizedStr, "putc") != NULL ||
      strcasestr(tokenizedStr, "puts") != NULL ||
      strcasestr(tokenizedStr, "enter") != NULL) {
    return true;
  }
  return false;
}

// takes in a clean token, that is a valid instruction and returns its
// corresponding opcode
opcode_t parseValidInstruction(char *tokenizedStr) {
  if (strcasecmp(tokenizedStr, "ld") == 0) {
    return OP_LD;
  }
  if (strcasecmp(tokenizedStr, "add") == 0) {
    return OP_ADD;
  }
  if (strcasecmp(tokenizedStr, "and") == 0) {
    return OP_AND;
  }
  if (strcasestr(tokenizedStr, "br") != NULL) {
    return OP_BR;
  }
  if (strcasecmp(tokenizedStr, "jmp") == 0 ||
      strcasecmp(tokenizedStr, "ret") == 0) {
    return OP_JMP;
  }
  if (strcasecmp(tokenizedStr, "jsr") == 0 ||
      strcasecmp(tokenizedStr, "jsrr") == 0) {
    return OP_JSR;
  }
  if (strcasecmp(tokenizedStr, "ldi") == 0) {
    return OP_LDI;
  }
  if (strcasecmp(tokenizedStr, "ldr") == 0) {
    return OP_LDR;
  }
  if (strcasecmp(tokenizedStr, "lea") == 0) {
    return OP_LEA;
  }
  if (strcasecmp(tokenizedStr, "not") == 0) {
    return OP_NOT;
  }
  if (strcasecmp(tokenizedStr, "ret") == 0) {
    return OP_RTI;
  }
  if (strcasecmp(tokenizedStr, "st") == 0) {
    return OP_ST;
  }
  if (strcasecmp(tokenizedStr, "sti") == 0) {
    return OP_STI;
  }
  if (strcasecmp(tokenizedStr, "str") == 0) {
    return OP_STR;
  }
  if (strcasecmp(tokenizedStr, "trap") == 0 ||
      strcasecmp(tokenizedStr, "putc") == 0 ||
      strcasecmp(tokenizedStr, "puts") == 0 ||
      strcasecmp(tokenizedStr, "getc") == 0 ||
      strcasecmp(tokenizedStr, "putsp") == 0 ||
      strcasecmp(tokenizedStr, "enter") == 0) {
    return OP_TRAP;
  }
  if (strcasecmp(tokenizedStr, "halt") == 0) {
    return OP_TRAP;
  }
  // should be unreachable:
  return ASSEMBLYERROR;
}

// takes in a clean token, that is a register and returns its corresponding
// reg_t
reg_t parseRegister(char *tokenizedStr) {
  if (strcmp(tokenizedStr, "%r0") == 0) {
    return R_R0;
  }
  if (strcmp(tokenizedStr, "%r1") == 0) {
    return R_R1;
  }
  if (strcmp(tokenizedStr, "%r2") == 0) {
    return R_R2;
  }
  if (strcmp(tokenizedStr, "%r3") == 0) {
    return R_R3;
  }
  if (strcmp(tokenizedStr, "%r4") == 0) {
    return R_R4;
  }
  if (strcmp(tokenizedStr, "%r5") == 0) {
    return R_R5;
  }
  if (strcmp(tokenizedStr, "%r6") == 0) {
    return R_R6;
  }
  if (strcmp(tokenizedStr, "%r7") == 0) {
    return R_R7;
  }
  if (strcmp(tokenizedStr, "%r8") == 0) {
    return R_PC;
  }
  if (strcmp(tokenizedStr, "%r9") == 0) {
    return R_COND;
  }
  // printf("ERROR: invalid register, in function parseRegister\n"); // DBG:
  return ASSEMBLYERROR;
}

// takes in a clean token, that is an immediate value and returns its clean
// uint16_t value
uint16_t parseImmediate(char *tokenizedStr) {
  return processVal(tokenizedStr); // should work
}

// fill up the boolean values n, z, and p from tokenizedStr. input shouldd be
// br*, where * can be n,z,p or nothing
void extractNZP(char *tokenizedStr, bool *n, bool *z, bool *p) {
  if (strcasestr(tokenizedStr, "n") != NULL) {
    *n = true;
  }
  if (strcasestr(tokenizedStr, "z") != NULL) {
    *z = true;
  }
  if (strcasestr(tokenizedStr, "p") != NULL) {
    *p = true;
  }
}
// fill up boolean value for isRet
void determineisRet(char *tokenizedStr, bool *isRet) {
  if (strcasecmp(tokenizedStr, "ret") == 0) {
    *isRet = true;
  } else {
    *isRet = false;
  }
}
// fill up boolean value for isJsrR
void determineisJsrR(char *tokenizedStr, bool *isJsrR) {
  if (strcasecmp(tokenizedStr, "jsrr") == 0) {
    *isJsrR = true;
  } else {
    *isJsrR = false;
  }
}

// use emit functions and metadata to build instruction
uint16_t assembleInstructionfromMetaData(opcode_t opcode, int numTokens,
                                         int registerCount, reg_t *reg1,
                                         reg_t *reg2, reg_t *reg3,
                                         uint16_t ImmOffsetVal, bool neg,
                                         bool zero, bool pos, bool isADDwithImm,
                                         bool isANDwithImm, bool isRet,
                                         bool isJsrR, int *ErrorCode) {
  // reg1 = dst, the other ones should be srcs
  if (opcode == OP_ADD) { // ADD
    if (isADDwithImm) {   // add with immediate
      return emit_add_imm(*reg1, *reg2, ImmOffsetVal);
    } else { // add with sr2
      return emit_add_reg(*reg1, *reg2, *reg3);
    }
  } else if (opcode == OP_AND) { // AND
    if (isANDwithImm) {          // and with immediate
      return emit_and_imm(*reg1, *reg2, ImmOffsetVal);
    } else { // and with sr2
      return emit_and_reg(*reg1, *reg2, *reg3);
    }
  } else if (opcode == OP_BR) { // BR
    return emit_br(neg, zero, pos, ImmOffsetVal);
  } else if (opcode == OP_JMP) { // JMP
    if (isRet) {                 // RET
      return 0xC1C0;             // 1100 000 111 000000
    } else {
      return emit_jmp(*reg1);
    }
  } else if (opcode == OP_JSR) { // JSR
    if (isJsrR) {
      return emit_jsrr(*reg1);
    } else {
      return emit_jsr(ImmOffsetVal);
    }
  } else if (opcode == OP_LD) {
    return emit_ld(*reg1, ImmOffsetVal);
  } else if (opcode == OP_LDI) {
    return emit_ldi(*reg1, ImmOffsetVal);
  } else if (opcode == OP_LDR) {
    return emit_ldr(*reg1, *reg2, ImmOffsetVal);
  } else if (opcode == OP_LEA) {
    return emit_lea(*reg1, ImmOffsetVal);
  } else if (opcode == OP_NOT) {
    return emit_not(*reg1, *reg2);
  } else if (opcode == OP_ST) {
    return emit_st(*reg1, ImmOffsetVal);
  } else if (opcode == OP_STI) {
    return emit_sti(*reg1, ImmOffsetVal);
  } else if (opcode == OP_STR) {
    return emit_str(*reg1, *reg2, ImmOffsetVal);
  } else if (opcode == OP_TRAP) {
    return emit_trap(ImmOffsetVal);
  }
  *ErrorCode = ASSEMBLYERROR;
  return ASSEMBLYERROR; // should be unreachable (if code is valid) (whole
                        // program should return 2 for assembler errors)
}
// takes in a clean tokenizedStr and searches for it in data, return if found
bool detectValidLabel(char *tokenizedStr, fileData *data) {
  for (int i = 0; i < data->numLabels; i++) {
    if (strcasecmp((data->table[i]).labelName, tokenizedStr) ==
        0) { // find label in the label table
      return true;
    }
  }
  return false;
}
// pass in a valid, clean tokenizedStr with the label
// modified to return offset
uint16_t parseValidLabel(char *tokenizedStr, fileData *data) {
  uint16_t currentAddress = data->currentAddress - START;
  // printf("parseValidLabel: currentAddress: %d\n",
  //        currentAddress); // DEBUG:
  for (int i = 0; i < data->numLabels; i++) {
    if (strcasecmp((data->table[i]).labelName, tokenizedStr) ==
        0) { // find label in the label table
      uint16_t labelAddress = (data->table[i]).labelAddress - START;
      uint16_t offset = (labelAddress - (currentAddress + 2)) / 2;
      // printf("parseValidLabel: Label found: returning: %d\n",
      //        (data->table[i].labelAddress) - (currentAddress + 1)); // DEBUG:
      return (uint16_t)offset;
    }
  }
  // should be unreachable. WARN: assumes valid input
  printf("parseValidLabel: Label not found, returning ASSEMBLYERROR\n");
  return ASSEMBLYERROR;
}

void printLabelTable(fileData *data) {
  // DEBUG: function
  printf("DEBUG: Showing label table contents: \n");
  labelTable *table = data->table;
  for (int i = 0; i < data->numLabels; i++) {
    printf("Label %d: %s, Address: %d\n", i, table[i].labelName,
           table[i].labelAddress);
  }
  printf("DEBUG: End of label table contents\n");
}
