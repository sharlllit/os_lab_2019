CC=gcc
CFLAGS=-I.

all: process_memory parallel_min_max_timeout parallel_sum

process_memory : 
    $(CC) -o process_memory process_memory.c $(CFLAGS)

parallel_min_max_timeout :
    $(CC) -o 1/parallel_min_max_timeout 1/find_min_max.c 1/parallel_min_max_timeout.c 1/utils.c $(CFLAGS)

parallel_sum : parallel_sum.o utils.o libsum.a
    $(CC) parallel_sum.o 1/utils.o -L static -l sum -lpthread -o parallel_sum $(CFLAGS)

parallel_sum.o :
    $(CC) -c       parallel_sum.c    -o parallel_sum.o $(CFLAGS)

utils.o :
    $(CC) -c       1/utils.c    -o 1/utils.o $(CFLAGS)

sum.o :
    $(CC) -c       sum.c    -o sum.o $(CFLAGS)

libsum.a : sum.o
    ar rcs static/libsum.a sum.o
