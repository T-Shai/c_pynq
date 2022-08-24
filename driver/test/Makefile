.PHONY: all clean

all: bin/main.o bin/pynq_utils.o
	gcc -o bin/main $^ -lpthread -lcma
	bin/main

bin/main.o: main.c
	gcc -c $^ -o $@  -I../../common

bin/pynq_utils.o: ../../common/pynq_utils.h ../../common/pynq_utils.c
	gcc -g -c ../../common/pynq_utils.c -o $@

$(info $(shell mkdir -p bin))