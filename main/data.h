#ifndef _DATA_H_
#define _DATA_H_


typedef struct Data {
    char id[20];
    char dataName[20];
    char type[20];
    char value[50];
    DataStructure next;
    DataStructure prev;
}*DataStructure;




DataStructure getDataStructure(DataStructure head, char* id);



// receive str to struct function
DataStructure createDataStructure(char* dataStr);

// List
void listAdd(DataStructure head, DataStructure addData);
void listDelete(DataStructure deleteData);


#endif
