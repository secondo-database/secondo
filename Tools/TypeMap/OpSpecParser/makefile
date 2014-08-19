

BASE:=OpSpecParser

.PHONY: all
all: OpSpec



OpSpec: scan.o parse.o
	g++ -o OpSpec scan.o parse.o -lfl


scan.o: lex.opspec.c 
	g++ -c -ggdb -o scan.o lex.opspec.c

lex.opspec.c: $(BASE).l parse.o 
	flex $(BASE).l

parse.o: $(BASE).tab.c
	g++ -c -ggdb -o parse.o $(BASE).tab.c -lstd


$(BASE).tab.c: $(BASE).y
	bison -v -d $(BASE).y

.PHONY: clean
clean:
	rm -f $(BASE).tab.c parse.o lex.opspec.c scan.o OpSpec $(BASE).tab.h $(BASE).output
