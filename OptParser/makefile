


GCC=gcc -ggdb

.PHONY: all
all: optparser


optparser: parser.o scanner.o
	$(GCC) -o optparser parser.o scanner.o -lstdc++ -lfl


parser.o: parser.c
	$(GCC) -x c++ -c -o parser.o parser.c

scanner.o: scanner.c
	$(GCC) -c -o scanner.o scanner.c

scanner.c: parser.h OptParser.l
	flex -o scanner.c OptParser.l

parser.h: parser.c

parser.c: OptParser.y
	bison  -d -o parser.c OptParser.y


.PHONY: clean
clean:
	rm -f parser.c parser.h scanner.c scanner.o parser.o optparser
