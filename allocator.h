#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define alloc(a) allocate(a, __FILENAME__, __LINE__)
#define free(a) my_free(a)

static int WORD_SIZE = sizeof(long);

void* allocate(size_t size, const char* file, int line);

void dumpMemory();

void my_free(void* ptr);

void printStats();

struct Header* requestFromOS(size_t size);

struct Header* getHeader(void* ptr);
