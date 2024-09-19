objects = bin/main.o
binout = bin/prot
flags = -O4 -march=native -ggdb
comp = gcc
$(binout) : $(objects)
	$(comp) $(objects) $(flags) -o $(binout)

bin/transcode : src/transcode.c
	$(comp) src/transcode.c $(flags) -o bin/transcode

bin/main.o : src/main.c src/buckets.h src/printing.h src/matrix.h
	$(comp) src/main.c $(flags) -c -o bin/main.o

test : test/test.c src/matrix.h
	$(comp) test/test.c -O0 -march=native -ggdb -Isrc -o testing
	./testing
