.PHONY: all clean

all: bin/image.o bin/process.o bin/main.o bin/pynq_utils.o
	gcc -g -o bin/main bin/image.o bin/process.o bin/main.o bin/pynq_utils.o
	bin/main

bin/image.o: src/image.h src/image.c
	gcc -g -c src/image.c -o bin/image.o

bin/process.o: src/process.h src/process.c
	gcc -g -c src/process.c -o bin/process.o

bin/pynq_utils.o: src/pynq_utils.h src/pynq_utils.c
	gcc -g -c src/pynq_utils.c -o $@

bin/main.o: src/main.c 
	gcc -g -c src/main.c -o bin/main.o

clean: Makefile
	rm -r bin
	rm data/threshold.pgm

$(info $(shell mkdir -p bin))