

BASE:=minimal

.PHONY: all
all: usingcheck



usingcheck : usingcheck.o
	gcc -o $@ $< -lfl

usingcheck.o: usingcheck.c
	gcc -c -o  $@ $< -MMD -MP -MF usingcheck.dep


usingcheck.c: usingcheck.l
	flex -o $@ $< 

# This makefile does not include makefile.env, so nothing pulls in a
# dependency file unless it is asked for here.
-include $(wildcard *.dep)

.PHONY: clean
clean:
	rm -f usingcheck  usingcheck.o usingcheck.c usingcheck.dep 


