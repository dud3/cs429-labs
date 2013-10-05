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
    while (cur && strcmp(cur->objectName, object)) {
        cur = cur->next;
    }
    if (!cur) {
        cur = (ObjectNode*) malloc(sizeof(ObjectNode));
        cur->objectName = (char*) calloc(strlen(object) + 1, sizeof(char));
        strcpy(cur->objectName, object);
        cur->property = (PropertyNode*) malloc(sizeof(PropertyNode));
        cur->property->propertyName = (char*) calloc(strlen(property) + 1, sizeof(char));
        strcpy(cur->property->propertyName, property);
        cur->property->value = (char*) calloc(strlen(value) + 1, sizeof(char));
        strcpy(cur->property->value, value);
        cur->property->next = 0;
        cur->next = objectHead;
        objectHead = cur;
    } else {
        PropertyNode* newProperty = (PropertyNode*) malloc(sizeof(PropertyNode));
        newProperty->propertyName = (char*) calloc(strlen(property) + 1, sizeof(char));
        strcpy(newProperty->propertyName, property);
        newProperty->value = (char*) calloc(strlen(value) + 1, sizeof(char));
        strcpy(newProperty->value, value);
        newProperty->next = cur->property;
        cur->property = newProperty;
    }
}

void lookUp(const char* object, const char* property) {
    ObjectNode* cur = objectHead;
    PropertyNode* pro;
    while (cur && strcmp(cur->objectName, object)) {
        cur = cur->next;
    }
    pro = cur->property;
    while (pro && strcmp(pro->propertyName, property)) {
        pro = pro->next;
    }
    printf("%s\n", pro->value);
}
    
int main() {
    saveIntoList("CDC6600", "number_registers", "24");
    saveIntoList("CDC6600", "opcode_bits", "6");
    saveIntoList("PDP11", "number_registers","8");
    saveIntoList("PDP11", "opcode_bits", "4,8,10");
    saveIntoList("IBM360", "number_registers", "16");
    saveIntoList("IBM360", "opcode_bits", "8");
    lookUp("PDP11", "opcode_bits");
    return 0;
}

