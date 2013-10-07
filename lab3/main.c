#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct PropertyNode {
    char* propertyName;
    char* value;
    struct PropertyNode* next;
} PropertyNode;

typedef struct ObjectNode {
    char* objectName;
    PropertyNode* property;
    struct ObjectNode* next;
} ObjectNode;

ObjectNode* objectHead = 0;

void saveIntoList(const char* object, const char* property, const char* value) {
    ObjectNode* cur = objectHead;
    while (cur && strcmp(cur->objectName, object)) { // Find insertion point
        cur = cur->next;
    }
    if (!cur) { // Object does not exist
        cur = (ObjectNode*) malloc(sizeof(ObjectNode));
        if (!cur) {
            printf("Insufficient memory");
            exit(1);
        }
        cur->objectName = (char*) calloc(strlen(object) + 1, sizeof(char));
        if (!cur->objectName) {
            printf("Insufficient memory");
            exit(1);
        }
        strcpy(cur->objectName, object);
        cur->property = (PropertyNode*) malloc(sizeof(PropertyNode));
        if (!cur->property) {
            printf("Insufficient memory");
            exit(1);
        }
        cur->property->propertyName = (char*) calloc(strlen(property) + 1, sizeof(char));
        if (!cur->property->propertyName) {
            printf("Insufficient memory");
            exit(1);
        }
        strcpy(cur->property->propertyName, property);
        cur->property->value = (char*) calloc(strlen(value) + 1, sizeof(char));
        if (!cur->property->value) {
            printf("Insufficient memory");
            exit(1);
        }
        strcpy(cur->property->value, value);
        cur->property->next = 0;
        cur->next = objectHead;
        objectHead = cur;
    } else { // Object exist
        PropertyNode* newProperty = (PropertyNode*) malloc(sizeof(PropertyNode)); // Always create a new property
        if (!newProperty) {
            printf("Insufficient memory");
            exit(1);
        }
        newProperty->propertyName = (char*) calloc(strlen(property) + 1, sizeof(char));
        if (!newProperty->propertyName) {
            printf("Insufficient memory");
            exit(1);
        }
        strcpy(newProperty->propertyName, property);
        newProperty->value = (char*) calloc(strlen(value) + 1, sizeof(char));
        if (!newProperty->value) {
            printf("Insufficient memory");
            exit(1);
        }
        strcpy(newProperty->value, value);
        newProperty->next = cur->property;
        cur->property = newProperty;
    }
}

void lookUp(const char* object, const char* property) {
    ObjectNode* cur = objectHead;
    PropertyNode* pro;
    while (cur && strcmp(cur->objectName, object)) { // Find object
        cur = cur->next;
    }
    if (!cur) { // Object does not exist
        printf("F %s: %s=unknown\n", object, property);
        return;
    }
    pro = cur->property;
    while (pro && strcmp(pro->propertyName, property)) { // Find property
        pro = pro->next;
    }
    if (!pro) { // Property does not exist
        printf("F %s: %s=unknown\n", object, property);
    } else {
        printf("F %s: %s=%s\n", object, property, pro->value);
    }
}

void stripStr(char* str) {
    char* cur = str;
    while (*cur) {
        if (*cur != ' ') {
            *str++ = *cur;
        }
        ++cur;
    }
    *str = 0;
}

#define BUF_LEN 1024

void parseFact(FILE* file) {
    char str[BUF_LEN];
    char object[BUF_LEN];
    char property[BUF_LEN];
    char value[BUF_LEN];
    char* ptr;
    if (!fgets(str, BUF_LEN, file) || str[0] != 'F') {
        return;
    }
    ptr = strtok(str + 1, ":");
    if (!ptr) {
        return;
    }
    strcpy(object, ptr);
    stripStr(object);
    ptr = strtok(0, "=");
    if (!ptr) {
        return;
    }
    strcpy(property, ptr);
    stripStr(property);
    ptr = strtok(0, "\n");
    if (!ptr) {
        return;
    }
    strcpy(value, ptr);
    stripStr(value);
    saveIntoList(object, property, value);
}

void parseQuestion(FILE* file) {
    char str[BUF_LEN];
    char object[BUF_LEN];
    char property[BUF_LEN];
    char* ptr;
    if (!fgets(str, BUF_LEN, file) || str[0] != 'Q') {
        return;
    }
    ptr = strtok(str + 1, ":");
    if (!ptr) {
        return;
    }
    strcpy(object, ptr);
    stripStr(object);
    ptr = strtok(0, "\n");
    if (!ptr) {
       return;
    }
    strcpy(property, ptr);
    stripStr(property);
    lookUp(object, property);
}

void releaseList() {
    while (objectHead) {
        ObjectNode* nextObject = objectHead->next;
        while (objectHead->property) {
            PropertyNode* nextProperty = objectHead->property->next;
            free(objectHead->property->propertyName);
            free(objectHead->property->value);
            free(objectHead->property);
            objectHead->property = nextProperty;
        }
        free(objectHead->objectName);
        free(objectHead);
        objectHead = nextObject;
    }
}

int main(int argc, char** argv) {
    FILE* fact;
    FILE* question = stdin;
    if (argc != 2 && argc != 3) { // Syntax error
        printf("Usage: %s fact-file [question-file]\n", argv[0]);
        exit(0);
    }
    fact = fopen(argv[1], "r");
    if (argc == 3) {
        question = fopen(argv[2], "r");
    }
    if (!fact) { // Fact file open error
        printf("Cannot open file %s\n", argv[1]);
        exit(0);
    }
    if (!question) { // Question file open error
        printf("Cannot open file %s\n", argv[2]);
        exit(0);
    }
    while (!feof(fact)) { // Reading fact
        parseFact(fact);
    }
    while (!feof(question)) { // Reading question
        parseQuestion(question);
    }
    releaseList();
    fclose(question);
    fclose(fact);
    return 0;
}

