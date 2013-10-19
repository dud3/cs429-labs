#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 4096

typedef struct {
    int link;
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
        if ((instruction >> 9) <= 5) {
            getMemoryAddress(instruction, machineStatus);
    }
    free(machineStatus);
    return 0;
}
