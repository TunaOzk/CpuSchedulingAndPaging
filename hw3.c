#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define PROCESS_NAME 50
#define BUFFER_SIZE 2048
#define NUMBER_OF_FRAMES 250
#define PAGE_SIZE 4096

#pragma region freeFramesList Implementation
struct node {
   int data;
   struct node *next;
};
struct freeFramesList {
    struct node *head;
} freeFramesList;

void initializeList() {
    freeFramesList.head = NULL;
}

void printList() {
   struct node *ptr = freeFramesList.head;
   printf("\n[ ");
	
   while(ptr != NULL) {
      printf("(%d) ",ptr->data);
      ptr = ptr->next;
   }
	
   printf(" ]");
}

void insertfreeFrameList(int data) {
   struct node *link = (struct node*) malloc(sizeof(struct node));
   link->data = data;
   link->next = freeFramesList.head;
   freeFramesList.head = link;
}

int freeFramesListLength() {
   int length = 0;
   struct node *current;
	
   for(current = freeFramesList.head; current != NULL; current = current->next) {
      length++;
   }
	
   return length;
}

struct node* deleteFromFreeFramesList(int index) {

   struct node* current = freeFramesList.head;
   struct node* previous = NULL;
   if(freeFramesList.head == NULL) {
      return NULL;
   }


   while(index > 0) {
      index--;
      if(current->next == NULL) {
         return NULL;
      } else {
         previous = current;
         current = current->next;
      }
   }

   if(current == freeFramesList.head) {
      freeFramesList.head = freeFramesList.head->next;
   } else {
      previous->next = current->next;
   }    
	
   return current;
}
#pragma endregion

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

void printPageTable(Array* arr, char *fileName) {
   printf("---PAGE TABLE OF %s---\n", fileName);
   for (int i = 0; i < arr->size-1; i+=2) {
      printf("Page %d ---> Frame %d\n", arr->array[i], arr->array[i+1]);
   }
   
}

void setFramesFromMemory(char* fileName, FILE** fp) {
    long int size = getFileSize(fileName);
    
    int numOfPages = (size / PAGE_SIZE);
    if(!numOfPages)
      numOfPages = 1;
    if(freeFramesListLength() < numOfPages) {
        printf("Process %s has exceed the # of free pages.Continuing with the next Process...\n", fileName);
        return;
    }
    Array pageTable;
    initArray(&pageTable, 1);
    for (int i = 0; i < numOfPages; i++) {
        int frameIndex = (rand() % freeFramesListLength()) - 1;
        struct node* node = deleteFromFreeFramesList(frameIndex);
        insertArray(&pageTable, i);
        insertArray(&pageTable, node->data);
    }
    printPageTable(&pageTable, fileName);

}

int main(int argc, char const *argv[])
{
    srand((unsigned)time(NULL));
    char* fileName;
    int* sizeArr;
    initializeList(freeFramesList);
    for (int i = 0; i < NUMBER_OF_FRAMES; i++){
        insertfreeFrameList(i);
    }
    FILE* fp = fopen("tasks.txt", "r");
    while(1) {
      Array logicalAddressArr;
      initArray(&logicalAddressArr, 1);
      if(getProcessInfo(&logicalAddressArr,&fileName,&fp) == -1) {
         freeArray(&logicalAddressArr);
         break;
      }
      setFramesFromMemory(fileName,&fp);
      freeArray(&logicalAddressArr);
    }
    
    // while(1) {
    //     Array logicalAddressArr;
    //     initArray(&logicalAddressArr, 1);
    //     if(getProcessInfo(&logicalAddressArr,&fileName,&fp) == -1)
    //         break;
    //     printf("File name %s\n", fileName);
    //     printf("File %s has %ld bytes\n", fileName, getFileSize(fileName));
    //     for(int i = 0; i < logicalAddressArr.size; i++)
    //         printf("%d\n", logicalAddressArr.array[i]);
    //     printf("\n");
    //     freeArray(&logicalAddressArr);
    // }
    return 0;
}
