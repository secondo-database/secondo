/*
----
This file is part of the PD system
Copyright (C) 1998 Ralf Hartmut Gueting, Fachbereich Informatik, FernUniversitaet Hagen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
----

The file ~PDLexTest.c~ contains the main function for testing lexical analysis. It prints all letters directly and prints the other tokens as integers.

*/

#include "PDTokens.h"
#include "PDNestedText.h"

main()
{

	int token;
	yylval = 0;
	
	token = yylex();
	while (token != 0) {

		if (token == LETTER) print(yylval);
		else printf("%d \n", token);

		token = yylex();
	}
}

/*
To produce a lexical analyser for testing one can issue the following commands (after including ~PDTokens.h~ in ~PDLex.l~):

----	lex PDLex.l
	cc PDLexTest.c lex.yy.c PDNestedText.o -ll 
----

The file ~a.out~ will then contain the analyser.

*/
