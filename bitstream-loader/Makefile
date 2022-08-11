all : main

CC = gcc
CFLAGS = -Wall 

main : main.o loader.o
	$(CC) $(CFLAGS) -o main main.o loader.o

main.o : main.c
	$(CC) $(CFLAGS) -c main.c

loader.o : loader.c
	$(CC) $(CFLAGS) -c loader.c

clean :
	rm -f main main.o loader.o