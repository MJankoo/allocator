#include "allocator.h"
#include "header.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h> 
#include "alloc_stats.h"

#define MAGIC 0xdeadbeaf

static struct AllocStats allocStats = {0, 0, 0};
static Header* firstBlock  = NULL;
static Header* previousBlock = NULL;

static int atexitSet = 0;
pthread_mutex_t lock; 

size_t alignSize(size_t size) {
	return size + (WORD_SIZE - (size % WORD_SIZE));
}

void dumpMemory() {
	if (firstBlock == NULL) {
		return;
	}
	
	struct Header* block = firstBlock;
	int blockNumber = 0;
	while(block != NULL) {
		printf("------- Block %d -------\n", blockNumber);
		printf("Pointer: %p\n", block);
		printf("Size: %d\n", (int) block->size);
		printf("Used: %s\n", block->used ? "true" : "false");
		printf("Next: %p\n", block->next);
		printf("Allocation location: %s:%d\n", block->allocationFile, block->allocationLine);
		printf("\n");
		block = block->next;
		blockNumber++;
	}
}

void printUnfreed() {
	struct Header* block = firstBlock;
	printf("Blocks that were allocated but weren't freed:\n");
	while(block != NULL) {
		if (!block->used) {
			block = block->next;
			continue;
		}
		printf("Allocation location: %s:%d\n", block->allocationFile, block->allocationLine);
		block = block->next;
	}
}

void printStats() {
	printf("Allocation counter: %d\n", allocStats.allocCounter);
	printf("Allocated bytes: %d\n", allocStats.allocatedBytes);
	printf("OS memory requests: %d\n", allocStats.sbrkCallCounter);
}

struct Header* firstFit(size_t size) {
  	struct Header* block = firstBlock;
 
  	while (block != NULL) {
    		if (block->used || block->size < size) {
      			block = block->next;
      			continue;
    		}
    		return block;
  	}
 
  	return NULL;
}

struct Header* findBlock(size_t size) {
  	struct Header* header = firstFit(size);
  	if (header == NULL) {
  		return NULL;
  	}
  	
  	if (header->magic != MAGIC) {
  		raise(SIGSEGV);
  	}
  	return header;
}

size_t align(size_t size) {
	size_t bytesToAlign = size % WORD_SIZE;
	if (bytesToAlign == 0) {
		return size;
	}
	return size + WORD_SIZE - bytesToAlign;
}

struct Header* getHeader(void* ptr) {
	return (struct Header*) ((char*) ptr - sizeof(struct Header));
}

void saveFirstBlock(struct Header* header) {
	if(firstBlock == NULL) {
		firstBlock = header;
	}
}

struct Header* requestFromOS(size_t size) {
	size += sizeof(struct Header);
	size = align(size);
	
	allocStats.sbrkCallCounter++;
	struct Header* header = (struct Header*) sbrk(0);  
	if (sbrk((intptr_t) size) == NULL) {
	    return NULL;
	}
	header->size = size - sizeof(struct Header);
	saveFirstBlock(header);
	
	return header;
}

bool canSplitBlock(struct Header* block, size_t size) {
	return block->size > align(size + sizeof(struct Header));
}

struct Header* split(struct Header* block, size_t size) {
	struct Header* next = block->next;
	block->next = (struct Header*) ((char*) block + sizeof(struct Header) + size);
	block->next->used = false;
	block->next->magic = MAGIC;
	block->next->size = block->size - sizeof(struct Header);
	block->next->next = next;
	return block;
}

struct Header* combine(struct Header* block) {
	struct Header* lastFreeBlockInRow = block;
	while(lastFreeBlockInRow->next != NULL && !lastFreeBlockInRow->next->used) {
		lastFreeBlockInRow = lastFreeBlockInRow->next;
	}
	if (lastFreeBlockInRow == block) {
		return block;
	}

	block->next = lastFreeBlockInRow->next;
	void* blockEnd = (char*) block + block->size + sizeof(struct Header);
	block->size += ((char*) lastFreeBlockInRow - (char*) blockEnd) + lastFreeBlockInRow->size + sizeof(struct Header);
	return block;
}

struct Header* getBlock(size_t size) {
	struct Header* header = findBlock(size);
	if (header == NULL) {
		header = requestFromOS(size);
		if (previousBlock != NULL) {
			previousBlock->next = header;
		}
		
		previousBlock = header;
	}
	if (canSplitBlock(header, size)) {
		split(header, size);
	}
	
	return header;
}

bool isTestMode() {
	char* mode = getenv("ENV_MODE"); // NOLINT
	if (mode == NULL) {
		return false;
	}
	return strcmp(mode, "test") == 0;
}

void* allocate(size_t size, const char* file, int line) {
	pthread_mutex_lock(&lock);

	if (!atexitSet && !isTestMode()) {
		atexitSet = atexit(printUnfreed);
	}

	allocStats.allocCounter++;
	allocStats.allocatedBytes+= (int) size;
	
	struct Header* header = getBlock(size);

	header->magic = MAGIC;
	header->used = true;
	header->allocationFile = file;
	header->allocationLine = line;
	
	pthread_mutex_unlock(&lock); 
	
	return (void*) ((char*) header + sizeof(struct Header));	
}

void my_free(void* ptr) {
	struct Header* blockHeader = getHeader(ptr);
	if (blockHeader->magic != MAGIC) {
  		//raise(SIGSEGV);
  	}
	
	blockHeader = combine(blockHeader);
	blockHeader->used = false;
}
