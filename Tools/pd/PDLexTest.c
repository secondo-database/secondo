/*
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
