GralToSec:	lex.yy.c
	gcc -o GralToSec lex.yy.c -lfl 

lex.yy.c:	Lex.l
	flex Lex.l

clean:	
	rm GralToSec* lex.yy.c
