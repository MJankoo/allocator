#include <unistd.h>
#include "assert.h"
#include "../allocator.h"
#include "../header.h"
#include <stdio.h>

void test_system_adds_header_size_and_alignment_to_requested_size()
{
	int requestedSize = 10;
	int expectedSize = sizeof(struct Header) + requestedSize;
	if (expectedSize % WORD_SIZE != 0) {
		expectedSize = expectedSize + WORD_SIZE - (expectedSize % WORD_SIZE);
	}
	
	char* beforeHeapPos = (char*) sbrk(0);

	alloc(requestedSize);
	char* afterHeapPos = (char*) sbrk(0);
	
	int currentSize = (int) (afterHeapPos - beforeHeapPos);
	
	assertSameInt(expectedSize, currentSize);
}

void test_system_allocates_at_least_request_size() {
	void* heapPosition = sbrk(0);
	int* intTab = alloc(sizeof(int) * 3);	
	int allocatedBytes = (int) ((char*) sbrk(0) - (char* ) heapPosition);
	assertLowerEqualInt(sizeof(int) * 3, allocatedBytes);

	heapPosition = sbrk(0);
	char* fifteenBytes = alloc(15);	
	allocatedBytes = (int) ((char*) sbrk(0) - (char* ) heapPosition);
	assertLowerEqualInt(15, allocatedBytes);
}

void test_system_mark_block_as_free() {
	int* intTab = alloc(sizeof(int) * 3);
	struct Header* intTabHeader = getHeader(intTab);	
	free(intTab);
	assertSameInt(false, intTabHeader->used);
	intTabHeader->used = 1;
}



void test_system_split_blocks() {
	int *tab = alloc(sizeof(int) * 30);
	free(tab);
	void* tabEndPointer = (void*) ((char*) tab + sizeof(int) * 30 + sizeof(struct Header));
	
	int *tab1 = alloc(WORD_SIZE);
	int *tab2 = alloc(WORD_SIZE);
	
	assertLowerEqualPointer((void*) tab, (void*) tab1);
	assertLowerEqualPointer((void*) tab2, tabEndPointer);
}

void test_system_combine_blocks() {
	int *block = alloc(10);
	int *block2 = alloc(10);
	free(block2);
	free(block);
	struct Header* blockHeader = getHeader(block);
	assertSameInt(blockHeader->size, 32 + sizeof(struct Header));
}

static int signalNumber = -1;
void signal_handler(int sig_num) {
	signalNumber = sig_num;
}

void test_system_throws_segmentation_fault() {
	signal(SIGSEGV, signal_handler);

	int *block = alloc(sizeof(int));
	int *block2 = alloc(sizeof(int) * 30);
	
	block[2] = 10;
	
	free(block2);
	assertSameInt(signalNumber, 11);
}

void run_alloc_test() {
	runTest(&test_system_split_blocks, &testStatus);
	runTest(&test_system_allocates_at_least_request_size, &testStatus);
	runTest(&test_system_mark_block_as_free, &testStatus);
	runTest(&test_system_adds_header_size_and_alignment_to_requested_size, &testStatus);
	runTest(&test_system_combine_blocks, &testStatus);
	runTest(&test_system_throws_segmentation_fault, &testStatus);
	
	finishTests(&testStatus);
}
