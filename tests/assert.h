#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define assertSameInt(a, b) assert_same_int(a, b, __FILENAME__, __LINE__)
#define assertLowerEqualInt(a, b) assert_lower_equal_int(a, b, __FILENAME__, __LINE__)
#define assertLowerEqualPointer(a, b) assert_lower_equal_pointer(a, b, __FILENAME__, __LINE__)

typedef struct Test {
	int assertions;
	int errors;
	int success;	
}Test;

static struct Test testStatus = {0, 0, 0};

void assert_same_int(int expected, int actual, const char* file, int line);

void assert_lower_equal_int(int expected, int actual, const char* file, int line);

void assert_lower_equal_pointer(void* expected, void* actual, const char* file, int line);

void runTest(void (test)(), struct Test* stats);

void finishTests(struct Test* stats);
