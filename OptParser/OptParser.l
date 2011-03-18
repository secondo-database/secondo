%{

#include "parser.h"

/*
Some versions of flex have a memeory leak when scanning a 
string. Calling this function before and after scanning a
string will remove this memory leak.


*/
void optlexDestroy(){
     yy_delete_buffer(YY_CURRENT_BUFFER);
    // optlex_destroy();
  }

%}


%option prefix="opt"
%option noyywrap

smallLetter [a-z]
capital [A-Z]
letter [a-zA-Z]
digit [0-9]
int [+-]?(([1-9][0-9]*)|0)
id {smallLetter}({letter}|{digit})*
variable {capital}({letter}|{digit})*
select "select"
from "from"
star \*


%option yylineno
%option noyywrap



%%
[ \a\b\f\t\v\n\r]+ { // ignore space
                   }
{select} { return TOKEN_SELECT; }
{from} { return TOKEN_FROM; }
{star} {return TOKEN_STAR; }
{id} {  optlval.strval = yytext; return TOKEN_ID; }
{variable} { optlval.strval = yytext; return TOKEN_VARIABLE;}
. { return TOKEN_ERROR; }
%%
