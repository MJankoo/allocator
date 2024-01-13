#include <stdio.h>
#include "assert.h"
#include "alloc_test.h"

int main() {
	putenv("ENV_MODE=test");
	run_alloc_test();
	return 0;
}


