#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define PAGE_SIZE 1024
#define TRUE 1
#define FALSE 0

struct QueueElement {
	int segmentNumber;
	int pageNumber;
};

struct Queue {
    int front, rear, size;
    unsigned capacity;
    struct QueueElement* array;
};

struct StackNode {
    int pageNumber;
    struct StackNode* next;
    struct StackNode* prev;
};

struct Stack {
	int size;
	unsigned capacity;
	struct StackNode* start;
	struct StackNode* top;
};

int parseInt(const char *s,int *i);
int ceilToDown(double d);

struct Queue* createQueue(unsigned capacity);
int queueIsFull(struct Queue* queue);
int queueIsEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, struct QueueElement item);
struct QueueElement* dequeue(struct Queue* queue);
struct QueueElement* queueFront(struct Queue* queue);
struct QueueElement* queueRear(struct Queue* queue);
int searchInQueue (struct Queue* queue, struct QueueElement item);
void retrievePage(struct Stack* stack, struct StackNode page, struct Queue* tlb, int segmentNo);

struct Stack* createStack(unsigned capacity);
int stackIsFull(struct Stack* stack);
int stackIsEmpty( struct Stack* stack);
int push(struct Stack* stack, struct StackNode item);
struct StackNode* pop(struct Stack* stack);
int searchIfFindTop(struct Stack* stack, struct StackNode item);

int main (int argc, char *argv[]) {
	char* segment__TEXT = argv[1];
	char* segment__DATA_CONST = argv[2];
	char* segment__LINKEDIT = argv[3];
	char* stringReference = argv[4];
	
	int segmentTextSize;
	int segmentDataSize;
	int segmentLinkSize;

	uint64_t start, end;
  	uint64_t elapsed;

	long int ns;
	time_t sec;
	struct timespec specStart;
	struct timespec specEnd;

	clock_gettime(CLOCK_REALTIME, &specStart);
	sec = specStart.tv_sec;
	ns = specStart.tv_nsec;

  	start = (uint64_t) sec * 1000000 + (uint64_t) ns;

	if(segment__TEXT != NULL)
		parseInt(segment__TEXT, &segmentTextSize);
	if(segment__DATA_CONST != NULL)
		parseInt(segment__DATA_CONST, &segmentDataSize);
	if(segment__LINKEDIT != NULL)
		parseInt(segment__LINKEDIT, &segmentLinkSize);

	/*	These are required page numbers*/
	int segmentTextPages = (segmentTextSize == 0) ? 0 : (segmentTextSize / PAGE_SIZE) + 1; 
	int segmentDataPages = (segmentDataSize == 0) ? 0 : (segmentDataSize / PAGE_SIZE) + 1;
	int segmentLinkPages = (segmentLinkSize == 0) ? 0 : (segmentLinkSize / PAGE_SIZE) + 1;
	
	int totalPages = segmentTextPages + segmentDataPages + segmentLinkPages;
	
	/* 	calculating how many phsical frame is needed */
	int physicalFrameForText = (segmentTextPages % 2 == 0) ? segmentTextPages/2 : (segmentTextPages+1)/2;
	int physicalFrameForData = (segmentDataPages % 2 == 0) ? segmentDataPages/2 : (segmentDataPages+1)/2;
	int physicalFrameForLink = (segmentLinkPages % 2 == 0) ? segmentLinkPages/2 : (segmentLinkPages+1)/2;

	int totalPhysicalFrame = physicalFrameForText + physicalFrameForData + physicalFrameForLink;
	
	/*	allocating frame to each segment to proportion ai= (si/S x m) */
	int allocatedFrameForText = ceilToDown((double)segmentTextPages/totalPages * totalPhysicalFrame);
	int allocatedFrameForData = ceilToDown((double)segmentDataPages/totalPages * totalPhysicalFrame);
	int allocatedFrameForLink = ceilToDown((double)segmentLinkPages/totalPages * totalPhysicalFrame);

	/*	stack initialization for page replacement with least recently used algorithm */
	struct Stack* lruText_Stack = createStack(allocatedFrameForText);
	struct Stack* lruData_Stack = createStack(allocatedFrameForData);
	struct Stack* lruLink_Stack = createStack(allocatedFrameForLink);

	/*	queue initialization for Translation Look-aside Buffer (TLB)  */
	struct Queue* tlb = createQueue(50);
	
	/*	if segment has at least one page then add the first page of it into stack */
	struct StackNode firstPage = {.pageNumber = 0};

	if(segmentTextPages > 0) push(lruText_Stack, firstPage);
	if(segmentDataPages > 0) push(lruData_Stack, firstPage);
	if(segmentLinkPages > 0) push(lruLink_Stack, firstPage);

	/*	listing how many pages in each segment */
	printf("SEGMENT__TEXT Pages: %d\n", segmentTextPages);
	printf("SEGMENT__DATA Pages: %d\n", segmentDataPages);
	printf("SEGMENT__LINK Pages: %d\n", segmentLinkPages);
    
	FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(stringReference, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
	
	/*
	invalid segment access - 1
	invalid page access - 2		
	*/
	int errno = 0;
	int errnoCounters[3] = {0,0,0};
	
	/*
	index 0: success
	index 1 = fault
	*/
	int tlbCounter[2] = {0,0};
	int pageFaultCounter [2] = {0,0};
	
    while ((read = getline(&line, &len, fp)) != -1) {
        
		//printf("Retrieved line of length %zu:\n", read);
        //printf("%s", line);
		errno = 0;
		char* segmentNoString = strtok(line, " ");
		char* pageNoString = strtok(NULL, "\n");
		int segmentNo, pageNo;

		parseInt(segmentNoString,&segmentNo);
		parseInt(pageNoString,&pageNo);
		
		if(segmentNo < 0 || segmentNo >= 3) {
			errno = 1;
			errnoCounters[errno] += 1;
			continue;

		}
		else {
			if(segmentNo == 0 && (pageNo < 0 || pageNo >= segmentTextPages)) {
				errno = 2;
				errnoCounters[errno] += 1;
				continue;
			}
			if(segmentNo == 1 && (pageNo < 0 || pageNo >= segmentDataPages)) {
				errno = 2;
				errnoCounters[errno] += 1;
				continue;
			}
			if(segmentNo == 2 && (pageNo < 0 || pageNo >= segmentLinkPages)) {
				errno = 2;
				errnoCounters[errno] += 1;
				continue;
			}
		}

		struct QueueElement tlbElement = {.segmentNumber = segmentNo, .pageNumber = pageNo};
		int isPageInTLB = searchInQueue(tlb, tlbElement);

		struct StackNode page = {.pageNumber = pageNo};

		/*	if it is already in TLB and memory */
		if(isPageInTLB == TRUE) {
			tlbCounter[0] += 1;
			usleep(1);
			switch (segmentNo)
			{
				case 0: searchIfFindTop(lruText_Stack, page); break;
				case 1: searchIfFindTop(lruData_Stack, page); break;
				case 2: searchIfFindTop(lruLink_Stack, page); break;
			}
		}
		
		/*	if it is not in the TLB */
		else {
			tlbCounter[1] += 1;
			int isPageInMemory;
			
			/*case that for each segments */
			switch (segmentNo)
			{	
				case 0:
					isPageInMemory = searchIfFindTop(lruText_Stack, page);
					
					/*	if it is already in the memory */
					if(isPageInMemory == TRUE) {
						if(queueIsFull(tlb)) {
							struct QueueElement* item = dequeue(tlb);
						}
						enqueue(tlb, tlbElement);
						pageFaultCounter[0] += 1;
						usleep(10);
					}
					/*	if it is not in the memory */
					else {
						retrievePage(lruText_Stack, page, tlb, segmentNo);
						if(queueIsFull(tlb)) {
							struct QueueElement* item = dequeue(tlb);
						}
						enqueue(tlb, tlbElement);
						pageFaultCounter[1] += 1;
						usleep(100);
					}
				break;

				case 1:
					isPageInMemory = searchIfFindTop(lruData_Stack, page);
					
					/*	if it is already in the memory */
					if(isPageInMemory == TRUE) {
						if(queueIsFull(tlb))
							dequeue(tlb);
						enqueue(tlb, tlbElement);
						pageFaultCounter[0] += 1;
						usleep(10);
					}
					
					/*	if it is not in the memory */
					else {
						retrievePage(lruData_Stack, page, tlb, segmentNo);
						if(queueIsFull(tlb))
							dequeue(tlb);
						enqueue(tlb, tlbElement);
						pageFaultCounter[1] += 1;
						usleep(100);
					}
				break;

				case 2:
					isPageInMemory = searchIfFindTop(lruLink_Stack, page);
					
					/*	if it is already in the memory */
					if(isPageInMemory == TRUE) {
						if(queueIsFull(tlb))
							dequeue(tlb);
						enqueue(tlb, tlbElement);
						pageFaultCounter[0] += 1;
						usleep(10);
					}
					
					/*	if it is not in the memory */
					else {
						retrievePage(lruLink_Stack, page, tlb, segmentNo);
						if(queueIsFull(tlb))
							dequeue(tlb);
						enqueue(tlb, tlbElement);
						pageFaultCounter[1] += 1;
						usleep(100);
					}
				break;
			}
		}
    }

	fclose(fp);
    if (line)
        free(line);

	double tlbMissRate = ((double)tlbCounter[1]) / (tlbCounter[0] + tlbCounter[1]);
	double pageFaultRate = ((double)pageFaultCounter[1]) / (pageFaultCounter[0] + pageFaultCounter[1]);

	clock_gettime(CLOCK_REALTIME, &specEnd);
	sec = specEnd.tv_sec;
	ns = specEnd.tv_nsec;

  	end = (uint64_t) sec * 1000000 + (uint64_t) ns;
	elapsed = (end - start);

	printf("Invalid segment access: %d\nInvalid page access: %d\n", errnoCounters[1], errnoCounters[2]);
	printf("TLB miss count: %d\nTLB miss rate: %f\n", tlbCounter[1], tlbMissRate);
	printf("Page fault count: %d\nPage fault rate: %f\n", pageFaultCounter[1], pageFaultRate);
	printf("Execution time in form of nanosecond: %llu\n", elapsed);

    exit(EXIT_SUCCESS);

}

int parseInt(const char *s,int *i)
{
    char *ep;
    long l;

    l=strtol(s,&ep,0);

    if(*ep!=0)
        return 0;

    *i=(int)l;
    return 1;
}
int ceilToDown(double d) {
	if((int)d == 0)
		return 1;
	else
		return (int)d;
}

// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (struct QueueElement*)malloc(
        queue->capacity * sizeof(struct QueueElement));
    return queue;
}
 
// Queue is full when size becomes
// equal to the capacity
int queueIsFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int queueIsEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, struct QueueElement item)
{
    if (queueIsFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    //printf("%d enqueued to queue\n", item);
}
 
// Function to remove an item from queue.
// It changes front and size
struct QueueElement* dequeue(struct Queue* queue)
{
    struct QueueElement* item = (struct QueueElement*)malloc(sizeof(struct QueueElement));
	if (queueIsEmpty(queue)) {
		return NULL;
	}
        
    *item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
// Function to get front of queue
struct QueueElement* queueFront(struct Queue* queue)
{
	if (queueIsEmpty(queue)) {
		return NULL;
	}
    return &(queue->array[queue->front]);
}
 
// Function to get rear of queue
struct QueueElement* queueRear(struct Queue* queue)
{
	if (queueIsEmpty(queue)) {
		return NULL;
	}
    return &(queue->array[queue->rear]);
}

int searchInQueue (struct Queue* queue, struct QueueElement item) {
	if(queueIsEmpty(queue))
		return FALSE;
	else {
		for(int i = queue->front ; i <= queue->rear; i++) {
			if(item.segmentNumber == queue->array[i].segmentNumber &&
				item.pageNumber == queue->array[i].pageNumber)
				return TRUE;
		}
		return FALSE;
	}
}

struct Stack* createStack(unsigned capacity)
{
    struct Stack* stack = (struct Stack*)malloc(
        sizeof(struct Stack));
    stack->capacity = capacity;
	stack->size = 0;
	stack->start = NULL;
	stack->top = NULL;
    return stack;
}

int stackIsFull(struct Stack* stack)
{
    return (stack->size == stack->capacity);
}

int stackIsEmpty( struct Stack* stack)
{
	return (stack->size == 0);
}


int push(struct Stack* stack, struct StackNode item) 
{
    struct StackNode* n = (struct StackNode*)malloc(
        sizeof(struct StackNode));;
    n->pageNumber = item.pageNumber;
	if(stack->size < stack->capacity) {
		if (stackIsEmpty(stack)) {
			n->prev = NULL;
			n->next = NULL;
	
			stack->start = n;
			stack->top = n;
		}
		else {
			stack->top->next = n;
			n->next = NULL;
			n->prev = stack->top;
			stack->top = n;
		}
		stack->size += 1;
		return TRUE;
	}
	else
		return FALSE;
}

// Pops top element from stack
struct StackNode* pop(struct Stack* stack)
{
    struct StackNode* stackStart;
	struct StackNode* item;
	stackStart = stack->start;

    if (stackIsEmpty(stack))
        return NULL;
	else {
		item->pageNumber = stackStart->pageNumber;
		item->next = NULL;
		item->prev = NULL;
		if (stack->top == stack->start) {
			stack->top = NULL;
			stack->start = NULL;
			free(stackStart);
		}
		else {
			stack->start->next->prev = NULL;
			stack->start = stackStart->next;
			free(stackStart);
		}
		stack->size -= 1;
		return item;
	}	
}

int searchIfFindTop(struct Stack* stack, struct StackNode item) {
	if(stackIsEmpty(stack))
		return FALSE;
	else {
		struct StackNode* ptr = stack->start;
		while(ptr != NULL) {
			if(ptr->pageNumber == item.pageNumber) {
				if(stack->size == 1)
					return TRUE;
				else {
					struct StackNode* ptrNext = ptr->next;
					struct StackNode* ptrPrev = ptr->prev;
					ptr->next = NULL;
					ptr->prev = stack->top;
					stack->top->next = ptr;
					stack->top = ptr;
					ptrNext->prev = ptrPrev;
					ptrPrev->next = ptrNext;
					return TRUE;
				}
			}
			ptr = ptr->next;
		}
		return FALSE;
	}
}

void retrievePage(struct Stack* stack, struct StackNode page, struct Queue* tlb, int segmentNo) {
	int doesPageRetrieved = push(stack, page);
	if(doesPageRetrieved == FALSE) {
		struct StackNode* victimPagePtr = pop(stack);
		push(stack, page);
		struct Queue* newtlb = createQueue(50);
		while(!queueIsEmpty(tlb)) {
			struct QueueElement* item = dequeue(tlb);
			if(!((item->pageNumber == victimPagePtr->pageNumber) && 
				(item->segmentNumber == segmentNo)))
				enqueue(newtlb, *item);
		}
		tlb->array = newtlb->array;
		tlb->capacity = newtlb->capacity;
		tlb->front = newtlb->front;
		tlb->rear = newtlb->rear;
		tlb->size = newtlb->size;
	}
}