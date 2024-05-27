objects = bin/main.o
binout = bin/prot
flags = -ggdb -O3
comp = gcc
$(binout) : $(objects)
	$(comp) $(objects) $(flags) -o $(binout)

bin/main.o : src/main.c src/buckets.h src/printing.h
	$(comp) src/main.c $(flags) -c -o bin/main.o
