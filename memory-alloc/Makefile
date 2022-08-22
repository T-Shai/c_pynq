.PHONY: all clean

all: bin/main.o bin/pynq_utils.o
	gcc -g -o bin/main bin/image.o bin/process.o bin/main.o bin/pynq_utils.o -lpthread -lcma 
	bin/main

bin/pynq_utils.o: ../common/pynq_utils.h ../common/pynq_utils.c
	gcc -g -c ../common/pynq_utils.c -o $@

bin/main.o: src/main.c 
	gcc -g -c src/main.c -o bin/main.o -I../common

clean: Makefile
	rm -r bin

$(info $(shell mkdir -p bin))