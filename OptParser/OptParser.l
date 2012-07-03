/*************************************************************************

This is the flex Defintion of the OptParser.

****************************************************************************/



%{
#include "Types.h"
#include "parser.h"
#include <iostream> 

/*
Some versions of flex have a memeory leak when scanning a 
string. Calling this function before and after scanning a
string will remove this memory leak.


*/
extern "C" {
void optlexDestroy(){
     yy_delete_buffer(YY_CURRENT_BUFFER);
     //optlex_destroy();
  }
}

%}


%option prefix="opt"
%option noyywrap



smallLetter [a-z]
capital [A-Z]
letter [a-zA-Z]
digit [0-9]
int (([1-9][0-9]*)|0)
id {smallLetter}({letter}|{digit})*
variable {capital}({letter}|{digit})*
star \*
num1 {digit}+\.?([eE][-+]?{digit}+)?
ident   {letter}({letter}|{digit}|_)*
math    {otherChar}({otherChar}|{star})*
symbol  {math}|{ident}
otherChar [^\(\)\[\]\"\'A-Za-z0-9_\* \a\b\f\n\r\t\v]
text '[^']'

%option yylineno
%option noyywrap


%%
[ \a\b\f\t\v\n\r]+ { // ignore space
                   }
{star}   {optlval.strval = strdup(yytext); return TOKEN_STAR; }
"as"     { return TOKEN_AS; }
"all"    { return TOKEN_ALL; }
"select" { return TOKEN_SELECT; }
"from"   { return TOKEN_FROM; }
"let"    { return TOKEN_LET;}
"("      { return TOKEN_OPEN_BRACKET; }
"sql"    { return TOKEN_SQL; }
")"      { return TOKEN_CLOSE_BRACKET; }
"."      { return TOKEN_DOT; }
","      { return TOKEN_COMMA; }
"["      { return TOKEN_SQOPEN_BRACKET; }
"]"      { return TOKEN_SQCLOSE_BRACKET; }
"-"      {optlval.strval = strdup(yytext); return TOKEN_MINUS; }
"+"      {optlval.strval = strdup(yytext); return TOKEN_PLUS; }
"<"      {optlval.strval = strdup(yytext); return TOKEN_SMALL_THAN; }
"<="     {optlval.strval = strdup(yytext); return TOKEN_SMALL_THAN_EQUAL; }
"="      {optlval.strval = strdup(yytext); return TOKEN_EQUAL; }
">="     {optlval.strval = strdup(yytext); return TOKEN_GREATER_THAN_EQUAL; }
">"      {optlval.strval = strdup(yytext); return TOKEN_GREATER_THAN; }
"#"      { return TOKEN_HASH; }
"<>"     { return TOKEN_DOUBLE_BRACKET; }
"rowid"  { return TOKEN_ROWID; }
"create" { return TOKEN_CREATE; }
"columns" { return TOKEN_COLUMNS; }
"delete" { return TOKEN_DELETE; }
"distinct" { return TOKEN_DISTINCT; }
"null"   { return TOKEN_NULL; }
"table"  { return TOKEN_TABLE; }
"drop"   { return TOKEN_DROP; }
"index"  { return TOKEN_INDEX; }
"on"     { return TOKEN_ON; }
"first"  { return TOKEN_FIRST; }
"last"   { return TOKEN_LAST; }
"const"  { return TOKEN_CONST; }
"value"  { return TOKEN_VALUE; }
"groupby" { return TOKEN_GROUP_BY; }
":"      { return TOKEN_COLON; }
"indextype" { return TOKEN_INDEX_TYPE; }
"insert" { return TOKEN_INSERT; }
"into"   { return TOKEN_INTO; }
"values" { return TOKEN_VALUES; }
"asc"    { return TOKEN_ASC; }
"desc"   { return TOKEN_DESC; }
"orderby" { return TOKEN_ORDER_BY; }
"{"      { return TOKEN_CUR_OPEN_BRACKET; }
"}"      { return TOKEN_CUR_CLOSE_BRACKET; }
"nonempty" { return TOKEN_NON_EMPTY; }
"any"    { return TOKEN_ANY; }
"some"   { return TOKEN_SOME; }
"update" { return TOKEN_UPDATE; }
"set"    { return TOKEN_SET; }
"where"  { return TOKEN_WHERE; }
"'"     { return TOKEN_PUNCT; }
"in"      { return TOKEN_IN; }
"intersection"      { return TOKEN_INTERSECTION; }
"union"      { return TOKEN_UNION; }
"distance"      { return TOKEN_DISTANCE; }
"not"      { return TOKEN_NOT; }
"exists"      { return TOKEN_EXISTS; }
"or"      { return TOKEN_OR; }
"and"      { return TOKEN_AND; }
"true"|"false" {optlval.strval = strdup(yytext);  return TOKEN_BOOL; }

{int}      { return TOKEN_INT; }
{num1}      { optlval.strval = strdup(yytext); return TOKEN_REAL; }
"line"      { return TOKEN_LINE;}
"points"      { return  TOKEN_POINTS;}
"mpoint"      { return  TOKEN_MPOINT;}
"uregion"      { return TOKEN_UREGION;}
"btree"      { return TOKEN_BTREE; }
"rtree"      { return TOKEN_RTREE; }
"hash"      { return TOKEN_HASH1; }
\"[^\"]\"       { optlval.strval = strdup(yytext); return TOKEN_STRING; }
{text} { optlval.strval = strdup(yytext); return TOKEN_TEXT;}
{id}   {// we have to copy because we can't rely on yytext not changing underneath us:
        optlval.strval = strdup(yytext);
        return TOKEN_ID; 
       }
{variable} { optlval.strval = strdup(yytext); return TOKEN_VARIABLE;}
{symbol} { optlval.strval = strdup(yytext); return TOKEN_SYMBOL;}

. { return TOKEN_ERROR; }
%%
