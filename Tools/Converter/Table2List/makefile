TableToList:	lex.yy.c
	gcc -o TableToList lex.yy.c -lfl 

lex.yy.c:	Lex.l
	flex Lex.l

clean:	
	rm TableToList* lex.yy.c
