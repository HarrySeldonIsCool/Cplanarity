objects = bin/main.o
binout = bin/prot
flags = -ggdb
comp = gcc
$(binout) : $(objects)
	$(comp) $(objects) $(flags) -o $(binout)

bin/main.o : src/main.c src/buckets.h
	$(comp) src/main.c $(flags) -c -o bin/main.o
