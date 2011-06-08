


.PHONY: all
all: SyntaxParser




SyntaxParser: scan.o parse.o
	g++ -o SyntaxParser scan.o parse.o -lfl


scan.o: lex.yy.c 
	g++ -c -o scan.o lex.yy.c

lex.yy.c: SyntaxParser.l parse.o 
	flex SyntaxParser.l

parse.o: SyntaxParser.tab.c
	g++ -c -o parse.o SyntaxParser.tab.c -lstd


SyntaxParser.tab.c: SyntaxParser.y
	bison -v -d SyntaxParser.y

.PHONY: clean
clean:
	rm -f SyntaxParser.tab.c parse.o lex.yy.c scan.o SyntaxParser SyntaxParser.tab.h SyntaxParser.output
