/*
May 15, 1998 RHG. Added ~model~ token.

August 8, 2000 RHG. Changed %p (the number of positions for Lex) to 5000.

August 10, 2000 RHG. Changed %e (the number of parse tree nodes) to 2000.

March 2002 Ulrich Telle Port to C++, using ~flex~ and ~bison~ instead of ~lex~ and ~yacc~.
The pattern for floating point numbers is adjusted so that numbers produced by
C/C++ programs are interpreted correctly.

*/
			
%p 8000
%n 1000
%e 2000
%{
#include "SecParser.tab.hpp"
#include "NestedText.h"

#define yylval xxlval
extern int yylval;

#define YY_NO_UNPUT
%}

letter         [a-zA-Z]
digit          [0-9]
ident          {letter}({letter}|{digit}|_)*  
num1	       [-+]?{digit}+\.?([eE][-+]?{digit}+)?
num2	       [-+]?{digit}*\.{digit}+([eE][-+]?{digit}+)?
number	       {num1}|{num2}
string         \"[^\"]*\"         

%%

[ \t\n]+  	;

-?[0-9]+  	{yylval = NestedText::Atom( yytext, yyleng ); return INTEGER;}
{number}	{yylval = NestedText::Atom( yytext, yyleng ); return REAL;}
TRUE  		{yylval = NestedText::Atom( yytext, yyleng ); return BOOLEAN;}
FALSE		{yylval = NestedText::Atom( yytext, yyleng ); return BOOLEAN;}
{string} 	{yylval = NestedText::Atom( yytext, yyleng ); return STRING;}
fun		{yylval = NestedText::Atom( yytext, yyleng ); return FUN;}
const		{yylval = NestedText::Atom( yytext, yyleng ); return CONST;}
tuple		{yylval = NestedText::Atom( yytext, yyleng ); return TUPLE;}
group		{yylval = NestedText::Atom( yytext, yyleng ); return GROUP;}

type		{yylval = NestedText::Atom( yytext, yyleng ); return TYPE;}
delete		{yylval = NestedText::Atom( yytext, yyleng ); return DELETE;}
create		{yylval = NestedText::Atom( yytext, yyleng ); return CREATE;}
update		{yylval = NestedText::Atom( yytext, yyleng ); return UPDATE;}
":="		{yylval = NestedText::Atom( yytext, yyleng ); return ASSIGN;}
query		{yylval = NestedText::Atom( yytext, yyleng ); return QUERY;}
model           {yylval = NestedText::Atom( yytext, yyleng ); return MODEL;}
let		{yylval = NestedText::Atom( yytext, yyleng ); return LET;}
persistent	{yylval = NestedText::Atom( yytext, yyleng ); return PERSISTENT;}
begin		{yylval = NestedText::Atom( yytext, yyleng ); return BEGIN1;}
transaction	{yylval = NestedText::Atom( yytext, yyleng ); return TRANSACTION;}
commit		{yylval = NestedText::Atom( yytext, yyleng ); return COMMIT;}
abort		{yylval = NestedText::Atom( yytext, yyleng ); return ABORT;}
database	{yylval = NestedText::Atom( yytext, yyleng ); return DATABASE;}
open		{yylval = NestedText::Atom( yytext, yyleng ); return OPEN;}
close		{yylval = NestedText::Atom( yytext, yyleng ); return CLOSE;}
save		{yylval = NestedText::Atom( yytext, yyleng ); return SAVE;}
to		{yylval = NestedText::Atom( yytext, yyleng ); return TO;}
restore		{yylval = NestedText::Atom( yytext, yyleng ); return RESTORE;}
from		{yylval = NestedText::Atom( yytext, yyleng ); return FROM;}
list		{yylval = NestedText::Atom( yytext, yyleng ); return LIST;}
databases	{yylval = NestedText::Atom( yytext, yyleng ); return DATABASES;}
constructors	{yylval = NestedText::Atom( yytext, yyleng ); return CONSTRUCTORS;}
operators	{yylval = NestedText::Atom( yytext, yyleng ); return OPERATORS;}
types		{yylval = NestedText::Atom( yytext, yyleng ); return TYPES;}
objects		{yylval = NestedText::Atom( yytext, yyleng ); return OBJECTS;}
".."		{yylval = NestedText::Atom( yytext, yyleng ); return DOUBLE;}

"+"		{return PLUS;}
"-"		{return MINUS;}
"*"		{return TIMES;}
"/"		{return DIVIDEDBY;}
div		{return DIV;}
mod		{return MOD;}
">"		{return GT;}
"<"		{return LT;}
"<="		{return LE;}
">="		{return GE;}
"#"		{return NE;}
"="		{return EQ;}
not		{return NOT;}
and		{return AND;}
or		{return OR;}
starts		{return STARTS;}
contains		{return CONTAINS;}
head		{return HEAD;}
max		{return MAX;}
min		{return MIN;}
avg		{return AVG;}
sum		{return SUM;}
count		{return COUNT;}
concat		{return CONCAT;}
attr		{return ATTR;}
project		{return PROJECT;}
filter		{return FILTER;}
cancel		{return CANCEL;}
rdup		{return RDUP;}
sort		{return SORT;}
extend		{return EXTEND;}
groupby		{return GROUPBY;}
gfeed		{return GFEED;}
product		{return PRODUCT;}
loopjoin		{return LOOPJOIN;}
mergesec		{return MERGESEC;}
mergediff		{return MERGEDIFF;}
sortby		{return SORTBY;}
mergejoin		{return MERGEJOIN;}
rename		{return RENAME;}
extract		{return EXTRACT;}
sd_get_x		{return GETX;}
sd_get_y		{return GETY;}
sd_p_count		{return PCOUNT;}
sd_p_diameter		{return PDIAMETER;}
sd_l_vertices		{return LVERTICES;}
sd_l_count		{return LCOUNT;}
sd_l_diameter		{return LDIAMETER;}
sd_l_length		{return LLENGTH;}
sd_r_vertices		{return RVERTICES;}
sd_r_contour		{return RCONTOUR;}
sd_l_interior		{return LINTERIOR;}
sd_r_count		{return RCOUNT;}
sd_r_diameter		{return RDIAMETER;}
sd_r_area		{return RAREA;}
sd_r_perimeter		{return RPERIMETER;}
sd_pp_equal		{return PPEQUAL;}
sd_pp_disjoint		{return PPDISJOINT;}
sd_pp_intersection		{return PPINTERSECTION;}
sd_pp_plus		{return PPPLUS;}
sd_pp_minus		{return PPMINUS;}
sd_ll_equal		{return LLEQUAL;}
sd_ll_disjoint		{return LLDISJOINT;}
sd_ll_intersects		{return LLINTERSECTS;}
sd_ll_meets		{return LLMEETS;}
sd_pl_on_border_of		{return PLONBORDEROF;}
sd_ll_border_in_common		{return LLBORDERINCOMMON;}
sd_ll_intersection		{return LLINTERSECTION;}
sd_ll_plus		{return LLPLUS;}
sd_ll_minus		{return LLMINUS;}
sd_ll_common_border		{return LLCOMMONBORDER;}
sd_rr_equal		{return RREQUAL;}
sd_rr_disjoint		{return RRDISJOINT;}
sd_pr_inside		{return PRINSIDE;}
sd_lr_inside		{return LRINSIDE;}
sd_rr_inside		{return RRINSIDE;}
sd_rr_area_disjoint		{return RRAREADISJOINT;}
sd_rr_edge_disjoint		{return RREDGEDISJOINT;}
sd_rr_edge_inside		{return RREDGEINSIDE;}
sd_rr_vertex_inside		{return RRVERTEXINSIDE;}
sd_lr_intersects		{return LRINTERSECTS;}
sd_rr_intersects		{return RRINTERSECTS;}
sd_lr_meets		{return LRMEETS;}
sd_rr_meets		{return RRMEETS;}
sd_rr_adjacent		{return RRADJACENT;}
sd_rr_encloses		{return RRENCLOSES;}
sd_pr_on_border_of		{return PRONBORDEROF;}
sd_lr_border_in_common		{return LRBORDERINCOMMON;}
sd_rr_intersection		{return RRINTERSECTION;}
sd_lr_intersection		{return LRINTERSECTION;}
sd_rr_plus		{return RRPLUS;}
sd_rr_minus		{return RRMINUS;}
sd_lr_common_border		{return LRCOMMONBORDER;}

{ident}		{yylval = NestedText::Atom( yytext, yyleng ); return IDENTIFIER;}

.		{yylval = NestedText::Atom( yytext, yyleng ); return yytext[0];}

%%
int yywrap() { return 1; }
