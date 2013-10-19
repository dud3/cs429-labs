#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 4096

typedef struct {
    int link;
    int reg;
    int programCounter;
    int memory[4096];
} MachineStatus;

int isHex(char c) {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

int parseObjectFile(const char* filename, MachineStatus* machineStatus) {
    int epSet = 0;
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

int main(int argc, char** argv) {
    MachineStatus* machineStatus;
    if (argc != 2) { // Check syntax
        fprintf(stderr, "Usage: %s object-file\n", argv[0]);
        exit(0);
    }
    machineStatus = (MachineStatus*) malloc(sizeof(MachineStatus));
    memset(machineStatus, 0, sizeof(MachineStatus));
    if (parseObjectFile(argv[1], machineStatus)) {
        free(machineStatus);
        exit(0);
    }
    while (1) {
        int instruction = machineStatus->memory[machineStatus->programCounter];
        if ((instruction >> 9) <= 5) { // Memory reference instruction
            int address = getMemoryAddress(instruction, machineStatus);
            // TODO Add time counter
            switch (instruction >> 9) {
                case 0: // AND
                    machineStatus->reg &= machineStatus->memory[address];
                    break;
                case 1: // TAD
                    machineStatus->reg += machineStatus->memory[address];
                    if (machineStatus->reg & 0x1000) { // Carry
                        machineStatus->link = 1 - machineStatus->link;
                        machineStatus->reg &= 0x0FFF;
                    }
                    break;
                case 2: // ISZ
                    if (++machineStatus->memory[address]) {
                        ++machineStatus->programCounter;
                        // TODO programCounter out of range?
                    }
                    break;
                case 3: // DCA
                    machineStatus->memory[address] = machineStatus->reg;
                    machineStatus->reg = 0;
                    break;
                case 4: // JMS
                    machineStatus->memory[address] = machineStatus->programCounter + 1;
                    machineStatus->programCounter = address + 1;
                    break;
                case 5: // JMP
                    machineStatus->programCounter = address;
                    break;
            }
        } else if ((instruction >> 9) == 0x07) { // Operate instruction
            if (instruction & 0x0100) { // Group 2
                int skip = 0;
                if (instruction & 0x40) { // SMA
                    if (machineStatus->reg & 0x0800) {
                        skip = 1;
                    }
                }
                if (instruction & 0x20) { // SZA
                    if (!machineStatus->reg) {
                        skip = 1;
                    }
                }
                if (instruction & 0x10) { // SNL
                    if (machineStatus->link) {
                        skip = 1;
                    }
                }
                if (instruction & 0x08) { // RSS
                    skip = 1 - skip;
                }
                if (instruction & 0x80) { // CLA
                    machineStatus->reg = 0;
                }
                if (instruction & 0x03) { // HLT
                    // TODO HLT
                }
                if (skip) {
                    ++machineStatus->programCounter;
                }
                if (instruction & 0xFB) { // NOP
                    // TODO NOP
                }
            } else { // Group 1
                if (instruction & 0x80) { // CLA
                    machineStatus->reg = 0;
                }
                if (instruction & 0x40) { // CLL
                    machineStatus->link = 0;
                }
                if (instruction & 0x20) { // CMA
                    machineStatus->reg = ~machineStatus->reg & 0x0FFF;
                }
                if (instruction & 0x10) { // CML
                    machineStatus->link = 1 - machineStatus->link;
                }
                if (instruction & 0x01) { // IAC
                    ++machineStatus->reg;
                    if (machineStatus->reg & 0x1000) { // Carry
                        machineStatus->link = 1 - machineStatus->link;
                        machineStatus->reg &= 0x0FFF;
                    }
                }
                if ((instruction & 0x0C) == 0x0C) { // Illegal
                    // TODO HLT
                }
                if (instruction & 0x0C) { // Rotate
                    int rotate = 1;
                    if (instruction & 0x02) { // Rotate two bits
                        rotate = 2;
                    }
                    if (instruction & 0x08) { // RAR or RTR
                        machineStatus->reg = (machineStatus->reg | (machineStatus->link << 12) | ((machineStatus->reg & 0x03) << 13)) >> rotate;
                    } else { // RAL or RTL
                        machineStatus->reg = (machineStatus->reg | (machineStatus->link << 12) | ((machineStatus->reg & 0x03) << 13)) << rotate;
                    }
                    machineStatus->link = (machineStatus->reg & 0x1000) >> 12;
                    machineStatus->reg &= 0x0FFF;
                }
                if (instruction & 0xFD) { // NOP
                    // TODO NOP
                }
            }
        }
    }
    free(machineStatus);
    return 0;
}
