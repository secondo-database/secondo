ListToTable:	lex.yy.c
	gcc -o ListToTable lex.yy.c -lfl

lex.yy.c:	Lex.l
	flex Lex.l

clean: 
	rm lex.yy.c ListToTable*

