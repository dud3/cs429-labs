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
    MachineStatus* machineStatus = (MachineStatus*) malloc(sizeof(MachineStatus));
    memset(machineStatus, 0, sizeof(MachineStatus));
    if (argc != 2) { // Check syntax
        fprintf(stderr, "Usage: %s object-file\n", argv[0]);
        exit(0);
    }
    obj = fopen(argv[1], "r"); // Open object file
    if (!obj) {
        fprintf(stderr, "Cannot open object file \"%s\"\n", argv[1]);
        exit(0);
    }
    while (!feof(obj)) {

    }
    fscanf(obj, "EP:%x", &machineStatus->ep);
    printf("%d\n", machineStatus->ep);
    // fscanf(obj, "%x:%x", &addr, &instruction);
    // printf("%x is %x\n", addr, instruction);


    fclose(obj);
    free(machineStatus);
    return 0;
}
