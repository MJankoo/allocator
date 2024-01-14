#!/bin/bash

EMPTY=""

if [ "$1" = "$EMPTY" ]; then
	echo "You have to provide location variable."
	exit;
fi;

gcc -c -fPIC allocator.c -o $1/allocator.o
gcc $1/allocator.o -shared -o $1/allocator.so

