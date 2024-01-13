#include "allocator.h"
#include <stdio.h>
#include <unistd.h>

int main() {
	int* tab = alloc(10);
	int* tab1 = alloc(10);
	
	free(tab);

	return 0;
}
