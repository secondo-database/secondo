# Filename: SecondoJava/sj/makefile
#
# Last change: Oct. 2002, M. Spiekermann

include ../makejava.inc

all: sj_lang

sj_lang:
	(cd lang; make all)


#////////////////// clean
clean:
	rm -f *.class
	(cd lang; make clean)

mrproper:
	rm -f *.class
	(cd lang; make mrproper)
