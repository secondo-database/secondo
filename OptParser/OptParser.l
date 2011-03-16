%{

#include "parser.h"


void lexDestroy(){
     yy_delete_buffer(YY_CURRENT_BUFFER);
    // optlex_destroy();
  }

%}


%option prefix="opt"
%option noyywrap


letter [a-zA-Z]
digit [0-9]
int [+-]?(([1-9][0-9]*)|0)
id {letter}({letter}|{digit})*
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
{id} { return ID; }
. { return ERROR; }
%%
