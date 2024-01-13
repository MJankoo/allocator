#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct Header {
	uint32_t magic;
	const char* allocationFile;
	int allocationLine;
	size_t size;
	bool used;
	struct Header* next;
}Header;
