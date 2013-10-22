#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Machine status
typedef struct {
    int link;
    int reg;
    int programCounter;
    int memory[4096];
} MachineStatus;

// Buffer for output
typedef struct {
    int size;
    int cur;
    char* buf;
} OutputBuffer;

void outputToBuffer(OutputBuffer* buf, char c) {
    if (buf->size == buf->cur + 1) {
        buf->buf = realloc(buf->buf, (buf->size <<= 1));
    }
    buf->buf[buf->cur] = c;
    ++buf->cur;
}

// Check if it is hex digit
int isHex(char c) {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

// Parse object file
int parseObjectFile(const char* filename, MachineStatus* machineStatus) {
    int epSet = 0; // EP set
    FILE* obj = fopen(filename, "r"); // Open object file
    if (!obj) {
        fprintf(stderr, "Cannot open object file \"%s\"\n", filename);
        return -1;
    }
    while (!feof(obj)) {
        char buf[1024];
        fgets(buf, sizeof(buf), obj);
        if (!strcmp(buf, "\n") || !strcmp(buf, "\r\n")) { // Skip empty line
            continue;
        }
        if (!strncmp(buf, "EP: ", 4)) { // EP: HEX
            if (epSet == 1 ||
                    !isHex(buf[4]) ||
                    !isHex(buf[5]) ||
                    !isHex(buf[6]) ||
                    (strcmp(buf + 7, "\n") && strcmp(buf + 7, "\r\n"))) {
                fprintf(stderr, "Object file error\n> %s", buf);
                fclose(obj);
                return -1;
            }
            sscanf(buf + 4, "%x", &machineStatus->programCounter);
            epSet = 1;
        } else { // HEX: HEX
            int location;
            int content;
            if (!isHex(buf[0]) ||
                    !isHex(buf[1]) ||
                    !isHex(buf[2]) ||
                    !isHex(buf[5]) ||
                    !isHex(buf[6]) ||
                    !isHex(buf[7]) ||
                    strncmp(buf + 3, ": ", 2) ||
                    (strcmp(buf + 8, "\n") && strcmp(buf + 8, "\r\n"))) {
                fprintf(stderr, "Object file error\n> %s", buf);
                fclose(obj);
                return -1;
            }
            sscanf(buf, "%x:%x", &location, &content);
            machineStatus->memory[location] = content;
        }
    }
    if (!epSet) {
        fprintf(stderr, "Object file error\n> \"No EP set\"\n");
        fclose(obj);
        return -1;
    }
    fclose(obj);
    return 0;
}

// Addressing
int getMemoryAddress(int instruction, MachineStatus* machineStatus) {
    int address = instruction & 0x7F;
    if (instruction & 0x80) { // Current page
        address |= machineStatus->programCounter & 0x0F80;
    }
    if (instruction & 0x0100) { // Indirect addressing
        address = machineStatus->memory[address];
    }
    return address;
}

// Add instruction to output
void appendInstructionStr(char* str, const char* rep) {
    int len = strlen(str);
    if (!len) {
        strcpy(str, rep);
    } else {
        str[len] = ' ';
        strcpy(str + len + 1, rep);
    }
}

int main(int argc, char** argv) {
    long long int time = 0;
    int halt = 0;
    int verbose = 0;
    MachineStatus* machineStatus;
    OutputBuffer outputBuffer;
    if (!(argc == 2 || (argc == 3 && !strcmp(argv[1], "-v")))) { // Check syntax
        fprintf(stderr, "Usage: %s [-v] object-file\n", argv[0]);
        exit(0);
    }
    if (argc == 3) { // Verbose mode
        verbose = 1;
    }
    machineStatus = (MachineStatus*) malloc(sizeof(MachineStatus)); // Initialize
    memset(machineStatus, 0, sizeof(MachineStatus));
    outputBuffer.size = 1024;
    outputBuffer.cur = 0;
    outputBuffer.buf = (char*) calloc(outputBuffer.size, sizeof(char));
    if (parseObjectFile(argv[verbose ? 2 : 1], machineStatus)) { // Parse
        free(outputBuffer.buf);
        free(machineStatus);
        exit(0);
    }
    while (!halt) {
        int oldProgramCounter = machineStatus->programCounter;
        int instruction = machineStatus->memory[machineStatus->programCounter]; // Fetch instruction
        char strInstruction[1024]; // String representation of instruction
        memset(strInstruction, 0, sizeof(strInstruction));
        if ((instruction >> 9) <= 5) { // Memory reference instruction
            int address = getMemoryAddress(instruction, machineStatus); // Effective address
            switch (instruction >> 9) {
                case 0: // AND
                    machineStatus->reg &= machineStatus->memory[address];
                    appendInstructionStr(strInstruction, "AND");
                    break;
                case 1: // TAD
                    machineStatus->reg += machineStatus->memory[address];
                    if (machineStatus->reg & 0x1000) { // Carry
                        machineStatus->link = 1 - machineStatus->link;
                        machineStatus->reg &= 0x0FFF;
                    }
                    appendInstructionStr(strInstruction, "TAD");
                    break;
                case 2: // ISZ
                    machineStatus->memory[address] = (machineStatus->memory[address] + 1) & 0x0FFF;
                    if (!machineStatus->memory[address]) {
                        machineStatus->programCounter = (machineStatus->programCounter + 1) & 0x0FFF;
                    }
                    appendInstructionStr(strInstruction, "ISZ");
                    break;
                case 3: // DCA
                    machineStatus->memory[address] = machineStatus->reg;
                    machineStatus->reg = 0;
                    appendInstructionStr(strInstruction, "DCA");
                    break;
                case 4: // JMS
                    machineStatus->memory[address] = (machineStatus->programCounter + 1) & 0x0FFF;
                    machineStatus->programCounter = address;
                    appendInstructionStr(strInstruction, "JMS");
                    break;
                case 5: // JMP
                    machineStatus->programCounter = (address - 1) & 0x0FFF;
                    appendInstructionStr(strInstruction, "JMP");
                    time -= 1;
                    break;
            }
            time += 2;
            if (instruction & 0x0100) { // Indirect addressing
                appendInstructionStr(strInstruction, "I");
                time += 1;
            }
        } else if ((instruction >> 9) == 0x07) { // Operate instruction
            if (instruction & 0x0100) { // Group 2
                if (instruction & 0x01) { // Illegal
                    halt = 1;
                    appendInstructionStr(strInstruction, "HLT");
                } else {
                    int skip = 0; // Skip next instruction
                    if (instruction & 0x40) { // SMA
                        if (machineStatus->reg & 0x0800) {
                            skip = 1;
                        }
                        appendInstructionStr(strInstruction, "SMA");
                    }
                    if (instruction & 0x20) { // SZA
                        if (!machineStatus->reg) {
                            skip = 1;
                        }
                        appendInstructionStr(strInstruction, "SZA");
                    }
                    if (instruction & 0x10) { // SNL
                        if (machineStatus->link) {
                            skip = 1;
                        }
                        appendInstructionStr(strInstruction, "SNL");
                    }
                    if (instruction & 0x08) { // RSS
                        skip = 1 - skip;
                        appendInstructionStr(strInstruction, "RSS");
                    }
                    if (instruction & 0x80) { // CLA
                        machineStatus->reg = 0;
                        appendInstructionStr(strInstruction, "CLA");
                    }
                    if (skip) {
                        machineStatus->programCounter = (machineStatus->programCounter + 1) & 0x0FFF;
                    }
                    if (instruction & 0x02) { // HLT
                        halt = 1;
                        appendInstructionStr(strInstruction, "HLT");
                    }
                    if (instruction & 0x04) { // OSR
                        appendInstructionStr(strInstruction, "OSR");
                    }
                    if (!(instruction & 0xFF)) { // NOP
                        appendInstructionStr(strInstruction, "NOP");
                    }
                }
            } else { // Group 1
                if ((instruction & 0x0C) == 0x0C) { // Illegal
                    halt = 1;
                    appendInstructionStr(strInstruction, "HLT");
                } else {
                    if (instruction & 0x80) { // CLA
                        machineStatus->reg = 0;
                        appendInstructionStr(strInstruction, "CLA");
                    }
                    if (instruction & 0x40) { // CLL
                        machineStatus->link = 0;
                        appendInstructionStr(strInstruction, "CLL");
                    }
                    if (instruction & 0x20) { // CMA
                        machineStatus->reg = ~machineStatus->reg & 0x0FFF;
                        appendInstructionStr(strInstruction, "CMA");
                    }
                    if (instruction & 0x10) { // CML
                        machineStatus->link = 1 - machineStatus->link;
                        appendInstructionStr(strInstruction, "CML");
                    }
                    if (instruction & 0x01) { // IAC
                        ++machineStatus->reg;
                        if (machineStatus->reg & 0x1000) { // Carry
                            machineStatus->link = 1 - machineStatus->link;
                            machineStatus->reg &= 0x0FFF;
                        }
                        appendInstructionStr(strInstruction, "IAC");
                    }
                    if (instruction & 0x0C) { // Rotate
                        int rotate = 1;
                        if (instruction & 0x02) { // Rotate two bits
                            rotate = 2;
                        }
                        if (instruction & 0x08) { // RAR or RTR
                            machineStatus->reg = (machineStatus->reg | (machineStatus->link << 12) | ((machineStatus->reg & 0x03) << 13)) >> rotate;
                            appendInstructionStr(strInstruction, rotate == 1 ? "RAR" : "RTR");
                        } else { // RAL or RTL
                            machineStatus->reg = (machineStatus->reg | (machineStatus->link << 12)) << rotate;
                            machineStatus->reg |= machineStatus->reg >> 13;
                            appendInstructionStr(strInstruction, rotate == 1 ? "RAL" : "RTL");
                        }
                        machineStatus->link = (machineStatus->reg & 0x1000) >> 12;
                        machineStatus->reg &= 0x0FFF;
                    }
                    if (!(instruction & 0xFD)) { // NOP
                        appendInstructionStr(strInstruction, "NOP");
                    }
                }
            }
            time += 1;
        } else { // Input-output instruction
            int device = (instruction & 0x01F8) >> 3;
            if (device == 3) {
                machineStatus->reg = getchar();
                appendInstructionStr(strInstruction, "IOT 3");
            } else if (device == 4) {
                outputToBuffer(&outputBuffer, machineStatus->reg & 0xFF);
                appendInstructionStr(strInstruction, "IOT 4");
            } else { // Illegal
                halt = 1;
                appendInstructionStr(strInstruction, "HLT");
            }
            time += 1;
        }
        if (verbose) {
            fprintf(stderr, "Time %lld: PC=0x%03X instruction = 0x%03X (%s), rA = 0x%03X, rL = %d\n", time, oldProgramCounter, instruction, strInstruction, machineStatus->reg, machineStatus->link & 0x01);
        }
        machineStatus->programCounter = (machineStatus->programCounter + 1) & 0x0FFF; // Update program counter
    }
    printf("%s", outputBuffer.buf);
    free(outputBuffer.buf);
    free(machineStatus);
    return 0;
}
