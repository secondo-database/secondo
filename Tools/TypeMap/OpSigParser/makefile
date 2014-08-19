

BASE:=OpSigParser

.PHONY: all
all: OpSig



OpSig: scan.o parse.o
	g++ -o OpSig scan.o parse.o -lfl


scan.o: lex.opsig.c 
	g++ -c -ggdb -o scan.o lex.opsig.c

lex.opsig.c: $(BASE).l parse.o 
	flex $(BASE).l

parse.o: $(BASE).tab.c
	g++ -c -ggdb -o parse.o $(BASE).tab.c -lstd


$(BASE).tab.c: $(BASE).y
	bison -v -d $(BASE).y

.PHONY: clean
clean:
	rm -f $(BASE).tab.c parse.o lex.opsig.c scan.o OpSig $(BASE).tab.h $(BASE).output
