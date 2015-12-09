

BASE:=minimal

.PHONY: all
all: usingcheck



usingcheck : usingcheck.o
	gcc -o $@ $< -lfl

usingcheck.o: usingcheck.c
	gcc -c -o  $@ $<


usingcheck.c: usingcheck.l
	flex -o $@ $< 

.PHONY: clean
clean:
	rm -f usingcheck  usingcheck.o usingcheck.c 


