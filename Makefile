#------------------------------------------------------------------------------
# Makefile for CSE 130 Programming Assignment 0
#
# make                   makes split
# make test				 makes test
# make clean             removes all binaries
#------------------------------------------------------------------------------

CC = clang
CFLAGS = -pthread -O -Wall -Wpedantic -Werror -Wextra

all:
	make queue.o

%.o : %.c
	$(CC) $(CFLAGS) -g -c $<

queue_test: queue.o queue_test.c
	$(CC) $(FLAGS) -o queue_test queue.o queue_test.c $(LDFLAGS)

format:
	clang-format *.c

clean :
	rm -f queue_test *.o
