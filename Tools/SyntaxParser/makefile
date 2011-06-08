


.PHONY: all
all: mini




mini: scan.o parse.o
	g++ -o mini scan.o parse.o -lfl


scan.o: lex.yy.c 
	g++ -c -o scan.o lex.yy.c

lex.yy.c: minimal2.l parse.o 
	flex minimal2.l

parse.o: minimal2.tab.c
	g++ -c -o parse.o minimal2.tab.c -lstd


minimal2.tab.c: minimal2.y
	bison -v -d minimal2.y

.PHONY: clean
clean:
	rm -f minimal2.tab.c parse.o lex.yy.c scan.o mini minimal2.tab.h minimal2.output
