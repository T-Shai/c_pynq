.PHONY: all clean

all: 
	gcc -g -o bin/main $^
	bin/main

bin/main.o: src/main.c 
	gcc -g -c $< -o $@

clean: Makefile
	rm -r bin

$(info $(shell mkdir -p bin))