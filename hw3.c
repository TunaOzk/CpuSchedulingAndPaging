#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
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
  arr->array = calloc(initialSize, sizeof(int));
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

int getProcessInfoV2(Array *arr, char** processName, FILE** fp) {
   FILE* f = *fp;
   int lineSize = 0;
   char* line = malloc(sizeof(char) * lineSize);
   char ch;
   int result = 0;
   while ((ch = fgetc(f)) != '\n') {
      if(ch == EOF) {
         result = -1;
         break;
      }
      else {
         lineSize++;
         line = realloc(line, sizeof(char) * lineSize);
         line[lineSize-1] = ch;
      }  
   }
   line = realloc(line, sizeof(char) * (lineSize+1));
   line[lineSize] = '\0';
   
   *processName = strtok(line, " ");
   char* token;
   while((token = strtok(NULL, " ")) != NULL) {
      int num = atoi(token);
      insertArray(arr, num);
   }
   insertArray(arr,0); // Address index that will be continued after time slice of the process has ended in RR scheduling.
   return result;
}

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
    insertArray(arr,0); // Address index that will be continued after time slice of the process has ended in RR scheduling.
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
   for (int i = 0; i < arr->size-1; i+=3) {
      printf("Page %d ---> Frame %d\n", arr->array[i], arr->array[i+1]);
   }
   printf("\n");
}

void setFramesFromMemory(Array* pageTable, char* fileName) {
    long int size = getFileSize(fileName);
    
    int numOfPages = (int)ceil(((double)size / PAGE_SIZE));
    if(!numOfPages)
      numOfPages = 1;
    if(freeFramesListLength() < numOfPages) {
        printf("Process %s has exceed the # of free pages. Continuing with the next Process...\n\n", fileName);
        return NULL;
    }

    for (int i = 0; i < numOfPages; i++) {
        int frameIndex = (rand() % freeFramesListLength()) - 1;
        struct node* node = deleteFromFreeFramesList(frameIndex);
        insertArray(pageTable, i);
        insertArray(pageTable, node->data);
        insertArray(pageTable, 0); // Checks whether the page is in TLB.
    }
    printPageTable(pageTable, fileName);

}

int isSchedulingFinished(Array** logicalAddresses, Array** pageTables, int numberOfProcesses) {
   for(int i = 0; i < numberOfProcesses; i++) {
      if(pageTables[i]->size == 0)
         continue;
      if(logicalAddresses[i]->array[logicalAddresses[i]->size-1] != logicalAddresses[i]->size-1)
         return 0;
   }
   return 1;
}

void printTheSchedulingResults(Array tlbHits, Array tlbMisses, Array dispatches, int numberOfProcesses,char** processNames) {
   printf("\n");
   for(int i = 0; i < numberOfProcesses; i++) {
      printf("----Process %s----\n", processNames[i]);
      printf("TLB Hits: %d\nTLB Misses: %d\nNumber of Dispatches: %d\n\n", 
         tlbHits.array[i], tlbMisses.array[i], dispatches.array[i]);
   }
}

int main(int argc, char const *argv[]) {
    srand((unsigned)time(NULL));
    char* fileName;
    int* sizeArr;
    int numberOfProcesses = 0;
    Array** pageTables = malloc(sizeof(Array*) * numberOfProcesses);
    Array** logicalAddresses = malloc(sizeof(Array*) * numberOfProcesses);
    char** processNames = malloc(sizeof(char*) * numberOfProcesses);
    initializeList(freeFramesList);
    for (int i = 0; i < NUMBER_OF_FRAMES; i++){
        insertfreeFrameList(i);
    }
    FILE* fp = fopen("tasks.txt", "r");
    //Paging simulation part
    int isFinished = 0;
    while(!isFinished) {
      numberOfProcesses++;
      logicalAddresses = realloc(logicalAddresses, sizeof(Array*) * numberOfProcesses);
      processNames = realloc(processNames, sizeof(Array*) * numberOfProcesses);
      logicalAddresses[numberOfProcesses-1] = malloc(sizeof(Array*));
      initArray(logicalAddresses[numberOfProcesses-1], 0);
      if(getProcessInfoV2(logicalAddresses[numberOfProcesses-1],&processNames[numberOfProcesses-1],&fp) == -1) {
         // numberOfProcesses--;
         isFinished = 1;
      }
      
      pageTables = realloc(pageTables, sizeof(Array*) * numberOfProcesses);
      pageTables[numberOfProcesses-1] = malloc(sizeof(Array*));
      initArray(pageTables[numberOfProcesses-1],0);
      setFramesFromMemory(pageTables[numberOfProcesses-1],processNames[numberOfProcesses-1]);
    }
    // Scheduling simulation part
    Array tlbHits;
    Array tlbMisses;
    Array dispatches;
    initArray(&tlbHits,numberOfProcesses);
    initArray(&tlbMisses,numberOfProcesses);
    initArray(&dispatches,numberOfProcesses);
    int memoryAccessCount = 0;
    int i = 0;
    while (!isSchedulingFinished(logicalAddresses, pageTables, numberOfProcesses)) {
      Array* logicalAddresArr = logicalAddresses[i];
      for(int j = logicalAddresArr->array[logicalAddresArr->size-1]; j < logicalAddresArr->size-1; j++) {
         if(pageTables[i]->size == 0) // Size of the process has exceed the number of free frames.
            break;
         int pageNo = logicalAddresArr->array[j] / PAGE_SIZE;
         if(pageTables[i]->array[3*pageNo + 2] == 0) { // TLB miss
            tlbMisses.array[i] = tlbMisses.array[i] + 1;
            memoryAccessCount++;
            pageTables[i]->array[3*pageNo + 2] = 1; // Entry has been added to TLB
            if(memoryAccessCount == 5) {
               logicalAddresArr->array[logicalAddresArr->size-1] = j+1;
               dispatches.array[i] = dispatches.array[i] + 1;
               break;
            }
         }
         else { //TLB hit
            tlbHits.array[i] = tlbHits.array[i] + 1;
            memoryAccessCount++; // for TLB access
            if(memoryAccessCount == 5) {
               logicalAddresArr->array[logicalAddresArr->size-1] = j+1;
               dispatches.array[i] = dispatches.array[i] + 1;
               break;
            }
         }
         if(j == logicalAddresArr->size-2) {
            
            dispatches.array[i] = dispatches.array[i] + 1;
            logicalAddresArr->array[logicalAddresArr->size-1] = j+1;
         }
      }
      memoryAccessCount = 0;
      i = (i + 1) % numberOfProcesses;
    }
    
    printTheSchedulingResults(tlbHits,tlbMisses,dispatches,numberOfProcesses, processNames);
    return 0;
}
