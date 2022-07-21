.PHONY: all clean

all: bin/main.o bin/pynq_utils.o bin/mem_alloc.o
	gcc -g -o bin/main $^ -lpthread -lcma 
	bin/main

bin/main.o: src/main.c 
	gcc -g -c $< -o $@

bin/pynq_utils.o: src/pynq_utils.c src/pynq_utils.h 
	gcc -g -c $< -o $@

bin/mem_alloc.o: src/mem_alloc.c src/mem_alloc.h 
	gcc -g -c $< -o $@

clean: Makefile
	rm -r bin

$(info $(shell mkdir -p bin))