TableToRel:	lex.yy.c
	gcc -o TableToRel lex.yy.c -lfl 

lex.yy.c:	Lex.l
	flex Lex.l

clean:	
	rm TableToRel* lex.yy.c
