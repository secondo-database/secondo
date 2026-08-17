include ../../makefile.env

OPSYNTAX=$(SECONDO_BUILD_DIR)/Optimizer/opsyntaxg.pl


.PHONY: all
all: $(OPSYNTAX)
	cat Header.pl DefaultPrios.pl Middle.pl opsyntax.pl Final.pl >$<

$(OPSYNTAX): opprios.pl Header.pl Middle.pl Final.pl



opprios.pl: SyntaxParser ../../Algebras/specs
	./SyntaxParser ../../Algebras/specs


../../Algebras/specs:
	$(MAKE) -C ../../Algebras specs




SyntaxParser: scan.o parse.o
	$(CPPC) -o SyntaxParser scan.o parse.o


scan.o: lex.yy.c
	$(CPPC) -g -fPIC -x c++ -c -o scan.o lex.yy.c -MMD -MP -MF scan.dep

lex.yy.c: SyntaxParser.l parse.o
	flex SyntaxParser.l

parse.o: SyntaxParser.tab.c
	$(CPPC) -g -fPIC -x c++ -c -o parse.o SyntaxParser.tab.c -MMD -MP -MF parse.dep


SyntaxParser.tab.c: SyntaxParser.y
	bison -v -d SyntaxParser.y

# The objects here come from generated C, which $(DEP_FILES) does not
# cover; take whatever .dep files exist instead of naming them.
-include $(wildcard *.dep)

.PHONY: clean
clean:
	rm -f SyntaxParser.tab.c parse.o lex.yy.c scan.o SyntaxParser SyntaxParser.tab.h SyntaxParser.output opprios.pl opsyntax.pl $(OPSYNTAX) *.dep
