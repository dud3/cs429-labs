#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Print in good format
void printStr(char* str) {
    unsigned int cnt = 0;
    while (*str) {
        if (cnt == 72) {
            printf("\n");
            cnt = 0;
        }
        printf("%c", *str++);
        ++cnt;
    }
    printf("\n");
}

// Convert 5-bit chunks into characters
char toCharRepresentation(uint8_t data) {
    if (data < 26) {
        return 'A' + data;
    }
    return '0' + data - 26;
}

// Convert 5 bytes of data into 8 bytes of 5-bit chunks
void convertFiveBytes(uint8_t* data, char* str) {
    str[0] = toCharRepresentation((data[0] >> 3) & 0x1F);
    str[1] = toCharRepresentation(((data[0] << 2) & 0x1C) | ((data[1] >> 6) & 0x03));
    str[2] = toCharRepresentation((data[1] >> 1) & 0x1F);
    str[3] = toCharRepresentation(((data[1] << 4) & 0x10) | ((data[2] >> 4) & 0x0F));
    str[4] = toCharRepresentation(((data[2] << 1) & 0x1E) | ((data[3] >> 7) & 0x01));
    str[5] = toCharRepresentation((data[3] >> 2) & 0x1F);
    str[6] = toCharRepresentation(((data[3] << 3) & 0x18) | ((data[4] >> 5) & 0x07));
    str[7] = toCharRepresentation(data[4] & 0x1F);
}

// Encode
void encode(uint8_t* orgData, unsigned int orgDataLength) {
    if (!orgDataLength) { // If there is nothing to encode, return directly to avoid outputting a single new line
        return;
    }
    unsigned int padDataLength = (orgDataLength + 4) / 5 * 5; // Length of padded data. Align to 5 bytes
    unsigned int strLength = 8 * padDataLength / 5; // Length of padded string
    unsigned int cnt = 0;
    uint8_t* padData = (uint8_t*) calloc(padDataLength, sizeof(uint8_t)); // Padded data
    char* str = (char*) calloc(strLength + 1, sizeof(char));
    memcpy(padData, orgData, orgDataLength);
    while (cnt < padDataLength) { // Convert 5 bytes at a time
        convertFiveBytes(padData + cnt, str + (8 * cnt) / 5);
        cnt += 5;
    }
    str[(8 * orgDataLength + 4) / 5] = 0; // Truncate padded bytes in the string
    printStr(str);
    free(str);
    free(padData);
}

// Ignore invalid characters and read into buffer
unsigned int parseInput(char* str, unsigned int size, FILE* stream) {
    unsigned int len = 0;
    char c;
    while (len < size && (c = getc(stream)) != EOF) {
        if ('A' <= c && c <= 'Z') {
            *str++ = c - 'A';
            ++len;
        } else if ('0' <= c && c <= '5') {
            *str++ = c - '0' + 26;
            ++len;
        }
    }
    return len;
}

// Convert 8 bytes of 5-bit chunks into 5 bytes of data
void convertEightChars(char* str, uint8_t* data) {
    data[0] = ((str[0] << 3) & 0xF8) | ((str[1] >> 2) & 0x07);
    data[1] = ((str[1] << 6) & 0xC0) | ((str[2] << 1) & 0x3E) | ((str[3] >> 4) & 0x01);
    data[2] = ((str[3] << 4) & 0xF0) | ((str[4] >> 1) & 0x0F);
    data[3] = ((str[4] << 7) & 0x80) | ((str[5] << 2) & 0x7C) | ((str[6] >> 3) & 0x03);
    data[4] = ((str[6] << 5) & 0xE0) | (str[7] & 0x1F);
}

// Decode
void decode(char* str, unsigned int strLength) {
    unsigned int padStrLength = (5 * strLength + 7) / 8 * 8; // Length of padded string
    unsigned int cnt = 0;
    unsigned int dataLength = 5 * strLength / 8; // Actual length of data
    uint8_t* data = (uint8_t*) calloc(5 * padStrLength / 8, sizeof(uint8_t)); // Padded data
    char* padStr = (char*) calloc(padStrLength, sizeof(char));
    memcpy(padStr, str, strLength);
    while (cnt < strLength) {
        convertEightChars(padStr + cnt, data + (5 * cnt) / 8);
        cnt += 8;
    }
    fwrite(data, sizeof(uint8_t), dataLength, stdout);
    free(padStr);
    free(data);
}

#define ENCODE_BUF_SIZE 45 // A line consists of 72 characters = 360 bits of data = 45 bytes of data
#define DECODE_BUF_SIZE 72

int main(int argc, char** argv) {
    if (!(argc == 1 || (argc == 2 && !strcmp(argv[1], "-d")))) { // Print usage
        printf("Usage: %s [-d] < input > output\n", argv[0]);
        printf("  -d\tdecode, otherwise defaults to encode\n");
        printf("  Example:\n");
        printf("    Encode: %s < four.5b > four.txt\n", argv[0]);
        printf("    Decode: %s -d < four.5b > four.txt\n", argv[0]);
        exit(0);
    }
    if (argc == 1) { // Encode
        uint8_t data[ENCODE_BUF_SIZE];
        while (!feof(stdin)) {
            memset(data, 0, sizeof(data));
            encode(data, fread(data, sizeof(uint8_t), ENCODE_BUF_SIZE, stdin));
        }
    } else { // Decode
        char str[DECODE_BUF_SIZE];
        while (!feof(stdin)) {
            memset(str, 0, sizeof(str));
            decode(str, parseInput(str, DECODE_BUF_SIZE, stdin));
        }
    }
    return 0;
}

