.PHONY: all clean

all: bin/main.o bin/pynq_utils.o bin/mem_alloc.o
	gcc -g -o bin/main $^ -lpthread -lcma 
	bin/main

bin/main.o: src/main.c 
	gcc -g -c $< -o $@ -I../common

bin/pynq_utils.o: ../common/pynq_utils.c ../common/pynq_utils.h 
	gcc  -g -c $< -o $@

bin/mem_alloc.o: src/mem_alloc.c src/mem_alloc.h 
	gcc -g -c $< -o $@ -I../common

clean: Makefile
	rm -r bin

$(info $(shell mkdir -p bin))