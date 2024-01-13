#include <stdbool.h>
#include <stdio.h>
#include "assert.h"

void assert(bool value, char* errorMessage, const char* file, int line) {
	testStatus.assertions++;
	if (value) {
		testStatus.success++;
		return;
	}
	testStatus.errors++;
	printf("%s:%d:\n", file, line);
	printf("%s\n\n", errorMessage);
}

void finishTests(struct Test* stats) {
	printf("\n");
	printf("Assertions: %d\n", stats->assertions);
	printf("\033[38;5;196mErrors: \033[38;5;255m%d\n", stats->errors);
	printf("\033[38;5;70mSuccess: \033[38;5;255m%d\n", stats->success);
}

void assert_same_int(int expected, int actual, const char* file, int line) {
	char errorMessage[100];
	sprintf(errorMessage, "Failed asserting that %d is equal to %d", expected, actual);
	assert(expected == actual, errorMessage, file, line);
}

void assert_lower_equal_int(int expected, int actual, const char* file, int line) {
	char errorMessage[100];
	sprintf(errorMessage, "Failed asserting that %d is lower or equal to %d", expected, actual);
	assert(expected <= actual, errorMessage, file, line);
}

void assert_lower_equal_pointer(void* expected, void* actual, const char* file, int line) {
	char errorMessage[100];
	sprintf(errorMessage, "Failed asserting that %p is lower or equal to %p", expected, actual);
	assert(expected <= actual, errorMessage, file, line);
}

void runTest(void (test)(), struct Test* stats) {
	int status;
	int fd[2];
	pipe(fd);
	int pid = fork();
	if (pid == 0) {
		close(fd[0]);
		test();
		int data = 0;

		data += testStatus.assertions;
		data = data << 8;
		data += testStatus.errors;
		data = data << 8;
		data += testStatus.success;
		write(fd[1], &data, sizeof(int));
		exit(0);
	} else if (pid > 0) {
		do 
		{
		    waitpid(pid, &status, WUNTRACED);
		} 
		while (!WIFEXITED(status) && !WIFSIGNALED(status));
		int val = 0;
		read(fd[0], &val, sizeof(val));
		stats->success += (val & 0xFF);
		stats->errors += (val & 0xFF00) >> 8;
		stats->assertions += (val & 0xFF0000) >> 16;
	} 
}
