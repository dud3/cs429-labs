#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    FILE* obj;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s object-file\n", argv[0]);
        exit(0);
    }
    obj = fopen(argv[1], "r");
    if (!obj) {
        fprintf(stderr, "Cannot open object file \"%s\"\n", argv[1]);
        exit(0);
    }
    fclose(obj);
    return 0;
}
