#ifndef _DATA_H_
#define _DATA_H_


typedef struct Data {
    char id[20];
    char dataName[20];
    char type[20];
    char value[50];
    struct Data* next;
    struct Data* prev;
}*DataStructure;



// data find util
DataStructure getDataStructure(DataStructure head, char* id);

// receive str to struct function
DataStructure createDataStructure(uint8_t* dataStr);
char* dataStructureToString(DataStructure data);

// List
void listAdd(DataStructure head, DataStructure addData);

void listDelete(DataStructure deleteData);


#endif
