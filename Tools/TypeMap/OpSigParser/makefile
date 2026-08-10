include ../../../makefile.env

BASE:=OpSigParser

.PHONY: all
all: OpSig



OpSig: scan.o parse.o
	$(CPPC) -o OpSig scan.o parse.o


# -x c++: the generated scanner and parser are C sources compiled as C++,
# which clang otherwise accepts only with a deprecation warning.
scan.o: lex.opsig.c
	$(CPPC) -c -fPIC -ggdb -x c++ -o scan.o lex.opsig.c

lex.opsig.c: $(BASE).l parse.o
	flex $(BASE).l

parse.o: $(BASE).tab.c
	$(CPPC) -c -fPIC -ggdb -x c++ -o parse.o $(BASE).tab.c


$(BASE).tab.c: $(BASE).y
	bison -v -d $(BASE).y

.PHONY: clean
clean:
	rm -f $(BASE).tab.c parse.o lex.opsig.c scan.o OpSig $(BASE).tab.h $(BASE).output
