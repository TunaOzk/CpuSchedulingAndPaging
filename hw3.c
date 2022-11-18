#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PROCESS_NAME 50
#define BUFFER_SIZE 2048

#pragma region Array Implementation
typedef struct {
  int *array;
  size_t used;
  size_t size;
} Array;

void initArray(Array *arr, size_t initialSize) {
  arr->array = malloc(initialSize * sizeof(int));
  arr->used = 0;
  arr->size = initialSize;
}

void insertArray(Array *arr, int element) {
  if (arr->used == arr->size) {
    arr->size += 1;
    arr->array = realloc(arr->array, arr->size * sizeof(int));
  }
  arr->array[arr->used++] = element;
}

void freeArray(Array *arr) {
  free(arr->array);
  arr->array = NULL;
  arr->used = arr->size = 0;
}
#pragma endregion

int getProcessInfo(Array *arr, char** processName, FILE** fp) {
    *processName = (char*)malloc(PROCESS_NAME);
    FILE* f = *fp;
    char* line = (char*)malloc(BUFFER_SIZE);

    if(fgets(line, BUFFER_SIZE, f) == NULL)
        return -1;
    *processName = strtok(line, " ");

    char* token;
    while((token = strtok(NULL, " ")) != NULL) {
        int num = atoi(token);
        insertArray(arr, num);
    }
    return 0;
}

long int getFileSize(char* fileName) {
    FILE* fp = fopen(fileName, "r");
    fseek(fp,0L,SEEK_END);
    long int size = ftell(fp);
    fclose(fp);
    return size;
}

int main(int argc, char const *argv[])
{
    char* fileName;
    int* sizeArr;
    
    FILE* fp = fopen("tasks.txt", "r");
    while(1) {
        Array logicalAddressArr;
        initArray(&logicalAddressArr, 1);
        if(getProcessInfo(&logicalAddressArr,&fileName,&fp) == -1)
            break;
        printf("File name %s\n", fileName);
        printf("File %s has %ld bytes\n", fileName, getFileSize(fileName));
        for(int i = 0; i < logicalAddressArr.size; i++)
            printf("%d\n", logicalAddressArr.array[i]);
        printf("\n");
        freeArray(&logicalAddressArr);
    }
    return 0;
}
