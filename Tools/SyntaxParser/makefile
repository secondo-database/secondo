


OPSYNTAX=$(SECONDO_BUILD_DIR)/Optimizer/opsyntax.pl


.PHONY: all
all: $(OPSYNTAX)


$(OPSYNTAX): opprios.pl Header.pl Middle.pl Final.pl
	cat Header.pl DefaultPrios.pl Middle.pl opsyntax.pl Final.pl >$@ 


opprios.pl: SyntaxParser ../../Algebras/specs
	./SyntaxParser ../../Algebras/specs


../../Algebras/specs:
	make -C ../../Algebras specs




SyntaxParser: scan.o parse.o
	g++ -o SyntaxParser scan.o parse.o -lfl


scan.o: lex.yy.c 
	g++ -g -c -o scan.o lex.yy.c

lex.yy.c: SyntaxParser.l parse.o 
	flex SyntaxParser.l

parse.o: SyntaxParser.tab.c
	g++ -g -c -o parse.o SyntaxParser.tab.c -lstd


SyntaxParser.tab.c: SyntaxParser.y
	bison -v -d SyntaxParser.y

.PHONY: clean
clean:
	rm -f SyntaxParser.tab.c parse.o lex.yy.c scan.o SyntaxParser SyntaxParser.tab.h SyntaxParser.output opprios.pl opsyntax.pl $(OPSYNTAX)
