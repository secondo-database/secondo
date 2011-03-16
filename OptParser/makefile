



include ../makefile.env


.PHONY: all
all: optparser


optparser: parser.o scanner.o OptChecker.o
	$(CC) $(CCFLAGS) -o optparser parser.o scanner.o OptChecker.o -lstdc++ -lfl


OptChecker.o: OptChecker.cpp
	$(CC) $(CCFLAGS) -c -o OptChecker.o OptChecker.cpp


parser.o: parser.c
	$(CC) $(CCFLAGS) -x c++ -c -o parser.o  parser.c

scanner.o: scanner.c
	$(CC) $(CCFLAGS) -c -o scanner.o scanner.c

scanner.c: parser.h OptParser.l
	$(LEX) -o scanner.c OptParser.l

parser.h: parser.c

parser.c: OptParser.y
	$(YACC)  -d -o parser.c OptParser.y


.PHONY: clean
clean:
	rm -f parser.c parser.h scanner.c scanner.o parser.o optparser OptChecker.o
