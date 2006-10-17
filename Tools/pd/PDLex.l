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

*/

%option yylineno
%option debug

%{
#include "PDNestedText.h"

%}


/* regular definitions */

lbracket	("(*"|"/*")
rbracket	("*)"|"*/")
star		[*]
other		[-;,?!`´'()/@#$%_\^{}+=|<>\n&üÜäÄöÖßµ¤]
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
tt              "__"

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
	
%x VERB 
%%

<INITIAL>^{open}		{return(OPEN);}
<INITIAL>{open2}		{return(OPEN);}
<INITIAL>^{close}		{return(CLOSE);}
<INITIAL>^{close}{open}		{ }

<INITIAL>^{verbatim}		{ 
                                  BEGIN(VERB); 
                                  return(VERBATIM);
                                }
<VERB>.|\n			{
                                  const char* v1="\\verb!{!";
                                  const char* v2="\\verb!}!";
                                  const char* v3="\\verb!\\!";

                                  switch (yytext[0]) {
                                    case '{'  : { yylval = atom(v1,8); break; }
                                    case '}'  : { yylval = atom(v2,8); break; }
                                    case '\\' : { yylval = atom(v3,8); break; }
                                    default   : { yylval = atom(yytext, yyleng); }
                                  }
                                  return(VCHAR);
                                }
<VERB>^{verbatim}		{ 
                                  BEGIN(INITIAL); 
                                  return(ENDVERBATIM);
                                }

<INITIAL>{epar}			{yylval = atom(yytext, yyleng); return(EPAR);}
<INITIAL>{defline1}		{yylval = atom(yytext, yyleng); return(DEFLINE);}
<INITIAL>^{defline2}		{yylval = atom(yytext, yyleng); return(DEFLINE);}
<INITIAL>[A-Za-z]		{yylval = atom(yytext, yyleng); return(LETTER);}
<INITIAL>^{head1}		{yylval = atom(yytext, yyleng); return(HEAD1);}
<INITIAL>^{head2}		{yylval = atom(yytext, yyleng); return(HEAD2);}
<INITIAL>^{head3}		{yylval = atom(yytext, yyleng); return(HEAD3);}
<INITIAL>^{head4}		{yylval = atom(yytext, yyleng); return(HEAD4);}
<INITIAL>^{head5}		{yylval = atom(yytext, yyleng); return(HEAD5);}
<INITIAL>^{enum1}		{yylval = atom(yytext, yyleng); return(ENUM1);}
<INITIAL>^{enum2}		{yylval = atom(yytext, yyleng); return(ENUM2);}
<INITIAL>^{bullet1}		{yylval = atom(yytext, yyleng); return(BULLET1);}
<INITIAL>^{bullet2}		{yylval = atom(yytext, yyleng); return(BULLET2);}
<INITIAL>^{follow1}		{yylval = atom(yytext, yyleng); return(FOLLOW1);}
<INITIAL>^{follow2}		{yylval = atom(yytext, yyleng); return(FOLLOW2);}
<INITIAL>^{display}		{yylval = atom(yytext, yyleng); return(DISPLAY);}
<INITIAL>^{figure}		{yylval = atom(yytext, yyleng); return(FIGURE);}
<INITIAL>^({ref}" "|"[] ")	{yylval = atom(yytext, yyleng); return(STARTREF);}
<INITIAL>{ref}			{yylval = atom(yytext, yyleng); return(REF);}
<INITIAL>[0-9]			{yylval = atom(yytext, yyleng); return(DIGIT);}
<INITIAL>"[~]"			{yylval = atom(yytext, yyleng); return(TILDE);}
<INITIAL>"[*]"			{yylval = atom(yytext, yyleng); return(STAR);}
<INITIAL>"[__]"			{yylval = atom(yytext, yyleng); return(DUS);}
<INITIAL>"[\"]"			{yylval = atom(yytext, yyleng); return(QUOTE);}
<INITIAL>" ~ "			{yylval = atom(yytext, yyleng); return(BLANKTILDE);}
<INITIAL>" * "			{yylval = atom(yytext, yyleng); return(BLANKSTAR);}
<INITIAL>" __ "			{yylval = atom(yytext, yyleng); return(BLANKDUS);}
<INITIAL>" \" "			{yylval = atom(yytext, yyleng); return(BLANKQUOTE);}
<INITIAL>{other}		{yylval = atom(yytext, yyleng); return(OTHER);}
<INITIAL>.			{yylval = atom(yytext, yyleng); return(yytext[0]);}
<INITIAL>"paragraph" 		{yylval = atom(yytext, yyleng); return(PARFORMAT);}
<INITIAL>"characters" 		{yylval = atom(yytext, yyleng); return(CHARFORMAT);}
<INITIAL>{tt}          	        {yylval = atom(yytext, yyleng); return(TTFORMAT);}


%%


/*
<INITIAL>"'"	        	{ yylval = atom(yytext, yyleng); return(OTHER);}


3.3 Testing the Lexical Analyser

One can test lexical analysis separately from the rest of the system. The files ~PDTokens.h~ and ~PDLexTest.c~ are needed. The file ~PDTokens.h~ needs to be included in the ~declarations~ section of ~Lex.l~:

----	%{
	#include "PDTokens.h"
	}%
----

This file just defines each token as an integer constant:

*/
