/*

*/

%{
#include "PDNestedText.h"
%}


/* regular definitions */

lbracket	("(*"|"/*")
rbracket	("*)"|"*/")
star		[*]
other		[-;,?!\`\'()/@#$%_\^{}+=|<>\n&]
open		{lbracket}{star}*(" "*[\n])+
open2		([\n]" "*)*[\n]{lbracket}{star}*(" "*[\n])+
close		{star}*{rbracket}(" "*[\n])+
epar		[\n]" "*[\n](" "*[\n])*
defline1	[\n]" "*"//"
defline2	" "*"//"
digit		[0-9]
num		({digit}{digit}|{digit})
ref		"["{num}"]"
verbatim	"----"

head1		{num}" "
head2		{num}"."{head1}
head3		{num}"."{head2}
head4		{num}"."{head3}
head5		{num}"."{head4}

enum1		"  "{digit}" "|" "{digit}{digit}" "
enum2		"  "{enum1}
bullet1		"  * "
bullet2		"  "{bullet1}
follow1		"    "
follow2		"  "{follow1}

display		"        "
figure		"                "
	
%%

^{open}			{return(OPEN);}
{open2}			{return(OPEN);}
^{close}		{return(CLOSE);}
^{close}{open}		{ }
^{verbatim}		{return(VERBATIM);}
{epar}			{yylval = atom(yytext, yyleng); return(EPAR);}
{defline1}		{yylval = atom(yytext, yyleng); return(DEFLINE);}
^{defline2}		{yylval = atom(yytext, yyleng); return(DEFLINE);}
[A-Za-z]		{yylval = atom(yytext, yyleng); return(LETTER);}
^{head1}		{yylval = atom(yytext, yyleng); return(HEAD1);}
^{head2}		{yylval = atom(yytext, yyleng); return(HEAD2);}
^{head3}		{yylval = atom(yytext, yyleng); return(HEAD3);}
^{head4}		{yylval = atom(yytext, yyleng); return(HEAD4);}
^{head5}		{yylval = atom(yytext, yyleng); return(HEAD5);}
^{enum1}		{yylval = atom(yytext, yyleng); return(ENUM1);}
^{enum2}		{yylval = atom(yytext, yyleng); return(ENUM2);}
^{bullet1}		{yylval = atom(yytext, yyleng); return(BULLET1);}
^{bullet2}		{yylval = atom(yytext, yyleng); return(BULLET2);}
^{follow1}		{yylval = atom(yytext, yyleng); return(FOLLOW1);}
^{follow2}		{yylval = atom(yytext, yyleng); return(FOLLOW2);}
^{display}		{yylval = atom(yytext, yyleng); return(DISPLAY);}
^{figure}		{yylval = atom(yytext, yyleng); return(FIGURE);}
^({ref}" "|"[] ")	{yylval = atom(yytext, yyleng); return(STARTREF);}
{ref}			{yylval = atom(yytext, yyleng); return(REF);}
[0-9]			{yylval = atom(yytext, yyleng); return(DIGIT);}
"[~]"			{yylval = atom(yytext, yyleng); return(TILDE);}
"[*]"			{yylval = atom(yytext, yyleng); return(STAR);}
"[\"]"			{yylval = atom(yytext, yyleng); return(QUOTE);}
" ~ "			{yylval = atom(yytext, yyleng); return(BLANKTILDE);}
" * "			{yylval = atom(yytext, yyleng); return(BLANKSTAR);}
" \" "			{yylval = atom(yytext, yyleng); return(BLANKQUOTE);}
{other}			{yylval = atom(yytext, yyleng); return(OTHER);}
.			{yylval = atom(yytext, yyleng); return(yytext[0]);}
"paragraph" 		{yylval = atom(yytext, yyleng); return(PARFORMAT);}
"characters" 		{yylval = atom(yytext, yyleng); return(CHARFORMAT);}

%%


/*

3.3 Testing the Lexical Analyser

One can test lexical analysis separately from the rest of the system. The files ~PDTokens.h~ and ~PDLexTest.c~ are needed. The file ~PDTokens.h~ needs to be included in the ~declarations~ section of ~Lex.l~:

----	%{
	#include "PDTokens.h"
	}%
----

This file just defines each token as an integer constant:

*/
