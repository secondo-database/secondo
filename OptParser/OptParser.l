%{

#include "parser.h"


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
{select} { return SELECT; }
{from} { return FROM; }
{star} {return STAR; }
{id} {  optlval.strval = yytext; return ID; }
{variable} { optlval.strval = yytext; return VARIABLE;}
. { return ERROR; }
%%
