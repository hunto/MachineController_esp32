#include "string.h"
#include "data.h"

DataStructure getDataStructure(DataStructure head, char* id) {
    // 遍历链表
    DataStructure current;
    for (; current != head; current = current->next) {
        if (!strcmp(id, current->id)) {
            return current;
        }
    }
}


DataStructure createDataStructure(char* dataStr) {
    // str sample: {a,b,c,d}
    // new a DataStructure
    DataStructure result = (DataStructure) malloc(sizeof(struct Data));
    result->next = NULL;
    result->prev = NULL;

    // delete last }
    *(strchr(dataStr, '}')) = '\0';

    // str1 -- id
    char* item = strtoc(dataStr + 1, ',');
    if (item == NULL) {
        return NULL;
    }
    strcpy(result->id, item);

    // str2 -- dataName
    item = strtoc(NULL, ',');
    if (item == NULL) {
        return NULL;
    }
    strcpy(result->dataName, item);

    // str3 -- dataType
    item = strtoc(NULL, ',');
    if (item == NULL) {
        return NULL;
    }
    strcpy(result->dataType, item);

    // str4 -- value
    item = strtoc(NULL, ',');
    if (item == NULL) {
        return NULL;
    }
    strcpy(result->value, item);

    return result;
}

void listAdd(DataStructure head, DataStructure addData) {
    // just a single data in list
    if (head->prev == NULL) {
        head->prev = addData;
        head->next = addData;
        addData->prev = head;
        addData->next = head;
        return;
    }

    // normal situation
    head->prev->next = addData;
    addData->prev = head->prev;
    addData->next = head;
    head->prev = addData;
}

void listDelete(DataStructure deleteData) {
    deleteData->prev->next = deleteData->next;
    deleteData->next->prev = deleteData->prev;
}
