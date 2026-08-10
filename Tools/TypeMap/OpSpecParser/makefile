include ../../../makefile.env

BASE:=OpSpecParser

.PHONY: all
all: OpSpec



OpSpec: scan.o parse.o
	$(CPPC) -o OpSpec scan.o parse.o


# -x c++: the generated scanner and parser are C sources compiled as C++,
# which clang otherwise accepts only with a deprecation warning.
scan.o: lex.opspec.c
	$(CPPC) -c -fPIC -ggdb -x c++ -o scan.o lex.opspec.c

lex.opspec.c: $(BASE).l parse.o
	flex $(BASE).l

parse.o: $(BASE).tab.c
	$(CPPC) -fPIC -c -ggdb -x c++ -o parse.o $(BASE).tab.c


$(BASE).tab.c: $(BASE).y
	bison -v -d $(BASE).y

.PHONY: clean
clean:
	rm -f $(BASE).tab.c parse.o lex.opspec.c scan.o OpSpec $(BASE).tab.h $(BASE).output
