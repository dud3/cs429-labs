#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 4096

typedef struct {
    int ep;
    int link;
    int programCounter;
    int memory[4096];
} MachineStatus;

int main(int argc, char** argv) {
    FILE* obj;
    int epSet;
    MachineStatus* machineStatus;
    if (argc != 2) { // Check syntax
        fprintf(stderr, "Usage: %s object-file\n", argv[0]);
        exit(0);
    }
    obj = fopen(argv[1], "r"); // Open object file
    if (!obj) {
        fprintf(stderr, "Cannot open object file \"%s\"\n", argv[1]);
        exit(0);
    }
    epSet = 0;
    machineStatus = (MachineStatus*) malloc(sizeof(MachineStatus));
    memset(machineStatus, 0, sizeof(MachineStatus));
    while (!feof(obj)) {
        char buf[1024];
        fgets(buf, sizeof(buf), obj);
        if (!strncmp(buf, "\n", 1) || !strncmp(buf, "\r\n", 2)) { // Skip empty line
            continue;
        }
        if (!strncmp(buf, "EP: ", 4)) {
            if (epSet == 1 || sscanf(buf, "EP: %x\r\n", &machineStatus->ep) != 1) {
                fprintf(stderr, "Object file error: %s", buf);
                free(machineStatus);
                exit(0);
            }
            epSet = 1;
        }
    }
    // fscanf(obj, "EP:%x", &machineStatus->ep);
    // printf("%d\n", machineStatus->ep);
    // fscanf(obj, "%x:%x", &addr, &instruction);
    // printf("%x is %x\n", addr, instruction);


    fclose(obj);
    free(machineStatus);
    return 0;
}
