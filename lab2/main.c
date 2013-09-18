#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

char toCharRepresentation(uint8_t data) {
    if (data < 26) {
        return 'A' + data;
    }
    return '0' + data - 26;
}

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

char* encode(uint8_t* orgData, unsigned int orgDataLength) {
    unsigned int padDataLength = (orgDataLength + 4) / 5 * 5; // Length of padded data. Align to 5 byte chunks.
    uint8_t* padData = (uint8_t*) calloc(padDataLength, sizeof(uint8_t)); // Padded data.
    unsigned int strLength = 8 * padDataLength / 5; // Length of padded string.
    char* str = (char*) calloc(strLength + 1, sizeof(char));
    unsigned int cnt = 0;
    memcpy(padData, orgData, orgDataLength);
    while (cnt < padDataLength) { // Convert 5 bytes at a time.
        convertFiveBytes(padData + cnt, str + (8 * cnt) / 5);
        cnt += 5;
    }
    str[(8 * orgDataLength + 4) / 5] = 0; // Truncate padded bytes in the string.
    free(padData);
    return str;
}

unsigned int parseInput(char* str) {
    unsigned int cur = 0;
    unsigned int ptr = 0;
    while (str[ptr]) {
        if ('A' <= str[ptr] && str[ptr] <= 'Z') {
            str[cur++] = str[ptr++] - 'A';
        } else if ('0' <= str[ptr] && str[ptr] <= '5') {
            str[cur++] = str[ptr++] - '0' + 26;
        } else {
            ++ptr;
        }
    }
    return cur;
}

void convertEightChars(char* str, uint8_t* data) {
    data[0] = ((str[0] << 3) & 0xF8) | ((str[1] >> 2) & 0x07);
    data[1] = ((str[1] << 6) & 0xC0) | ((str[2] << 1) & 0x3E) | ((str[3] >> 4) & 0x01);
    data[2] = ((str[3] << 4) & 0xF0) | ((str[4] >> 1) & 0x0F);
    data[3] = ((str[4] << 7) & 0x80) | ((str[5] << 2) & 0x7C) | ((str[6] >> 3) & 0x03);
    data[4] = ((str[6] << 5) & 0xE0) | (str[7] & 0x1F);
}

uint8_t* decode(char* str, unsigned int* dataLength) {
    unsigned int strLength = parseInput(str);
    unsigned int padStrLength = (5 * strLength + 7) / 8 * 8;
    *dataLength = 5 * strLength / 8;
    char* padStr = (char*) calloc(padStrLength, sizeof(char));
    uint8_t* data = (uint8_t*) calloc(5 * padStrLength / 8, sizeof(uint8_t)); // Padded data.
    unsigned int cnt = 0;
    memcpy(padStr, str, strLength);
    while (cnt < strLength) {
        convertEightChars(padStr + cnt, data + (5 * cnt) / 8);
        cnt += 8;
    }
    free(padStr);
    return data;
}

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

void freePtr(char* ptr) {
    free(ptr);
}

int main(int argc, char** argv) {
    // uint8_t* data = (uint8_t*) calloc(1024000, sizeof(uint8_t));
    // char* str = encode(data, fread(data, sizeof(uint8_t), 1024000, stdin));
    // printStr(str);
    // freePtr(str);
    // free(data);
    char* str = (char*) calloc(1024000, sizeof(char));
    fread(str, sizeof(char), 1024000, stdin);
    unsigned int dataLength;
    uint8_t* data = decode(str, &dataLength);
    fwrite(data, sizeof(uint8_t), dataLength, stdout);
    return 0;
}

