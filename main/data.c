#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "string.h"
#include "data.h"

DataStructure getDataStructure(DataStructure head, char* id) {
    if (head == NULL) {
      return NULL;
    }
    // 遍历链表
    DataStructure current = head;
    do {
        if (!strcmp(id, current->id)) {
            return current;
        }
        current = current->next;
    } while (current != head);
    return NULL;
}


DataStructure createDataStructure(uint8_t* str) {
    // str sample: {a,b,c,d}
    // new a DataStructure
    char * dataStr = malloc(sizeof(str));
    strcpy(dataStr, (char*)str);
    DataStructure result = (DataStructure) malloc(sizeof(struct Data));
    result->next = NULL;
    result->prev = NULL;

    // delete last }
    *(strstr(dataStr, "}")) = '\0';

    // str1 -- id
    char* item = strtok(dataStr + 1, ",");
    if (item == NULL) {
        return NULL;
    }
    strcpy(result->id, item);

    // str2 -- dataName
    item = strtok(NULL, ",");
    if (item == NULL) {
        return NULL;
    }
    strcpy(result->dataName, item);

    // str3 -- dataType
    item = strtok(NULL, ",");
    if (item == NULL) {
        return NULL;
    }
    strcpy(result->type, item);

    // str4 -- value
    item = strtok(NULL, ",");
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


char* dataStructureToString(DataStructure data) {
    char* result = malloc(sizeof(char) * 100);
    strcpy(result, "{");
    strcpy(result + strlen(result), data->id);
    strcpy(result + strlen(result), ",");
    strcpy(result + strlen(result), data->dataName);
    strcpy(result + strlen(result), ",");
    strcpy(result + strlen(result), data->type);
    strcpy(result + strlen(result), ",");
    strcpy(result + strlen(result), data->value);
    strcpy(result + strlen(result), "}");
    return result;
}
