include ../../../makefile.env

BASE:=OpSigParser

.PHONY: all
all: OpSig



OpSig: scan.o parse.o
	$(CPPC) -o OpSig scan.o parse.o


scan.o: lex.opsig.c
	$(CPPC) -c -fPIC -ggdb -o scan.o lex.opsig.c

lex.opsig.c: $(BASE).l parse.o
	flex $(BASE).l

parse.o: $(BASE).tab.c
	$(CPPC) -c -fPIC -ggdb -o parse.o $(BASE).tab.c -lstd


$(BASE).tab.c: $(BASE).y
	bison -v -d $(BASE).y

.PHONY: clean
clean:
	rm -f $(BASE).tab.c parse.o lex.opsig.c scan.o OpSig $(BASE).tab.h $(BASE).output
