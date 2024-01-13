build:
	gcc allocator.c main.c -ggdb3 -o allocator.o
	
config:
	./install_env.sh		
	
tests:
	@gcc allocator.c tests/assert.c tests/alloc_test.c tests/main.c -o tests.sh && ./tests.sh
	@rm -rf tests.sh
	
test-coverage:
	@gcc allocator.c tests/assert.c tests/alloc_test.c tests/main.c -ftest-coverage -fprofile-arcs -o test-cov.o
	@./test-cov.o >> /dev/null
	@gcov test-cov.o-allocator.c
	@rm *.gcda *.gcno

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes ./allocator.o
	
clang-tidy:
	clang-tidy allocator.c --quiet -checks=bugprone-*,-bugprone-easily-swappable-parameters,clang-analyzer-*,cert-*,concurrency-*,misc-*,modernize-*,performance-*,readability-* --warnings-as-errors=*
	
scan-build:
	scan-build --status-bugs --keep-cc --show-description ./allocator.o
	
xanalyzer:
	clang --analyze -Xanalyzer -analyzer-output=text allocator.c			

regression:
	make valgrind
	make clang-tidy
	make scan-build
	make xanalyzer
	make tests
	make test-coverage 

clean:
	@rm *.gcda *.gcno * .gcov

.PHONY: tests
