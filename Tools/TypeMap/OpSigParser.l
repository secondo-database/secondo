%{

#include "OpSigParser.tab.h"
#include <iostream>
#include <string>

using namespace std;



%}

%option prefix="opsig"


letter	      [a-zA-Z]
uppercase     [A-Z]    
lowercase     [a-z]
digit	      [0-9]
intindex      [1-9]
emptyspace    [ \t\n]+
otherChar     [^\(\)\{\}\"A-Za-z0-9 \:\_\,\;\.\n  ]
identifier    {letter}({letter}|{digit})*
math          {otherChar}+


%option yylineno
%option noyywrap

%%

attr/[(]	{ opsiglval.tokenchar = strdup(yytext); return (ZZATTR);
		 /* because it is also an operator */ }
attrs		{ opsiglval.tokenchar = strdup(yytext); return (ZZATTRS); }
combine		{ opsiglval.tokenchar = strdup(yytext); return (ZZCOMBINE); }
concat		{ opsiglval.tokenchar = strdup(yytext); return (ZZCONCAT); }
distinctAttrs	{ opsiglval.tokenchar = strdup(yytext); return (ZZDISTATTRS); }
minus		{ opsiglval.tokenchar = strdup(yytext); return (ZZMINUS); }
createAttr	{ opsiglval.tokenchar = strdup(yytext); return (ZZCREATEATTR); }
where		{ return (ZZWHERE); }
in		{ return (ZZIN); }
"int"|"real"|"string"|"bool"|"point"|"region"							{ opsiglval.tokenchar = strdup(yytext); return (ZZDATATYPE); }
^algebra	{ return (ZZALG); }
"::"		{ return (ZZPARAM); }
"x"		{ return (ZZCROSSPRODUCT); }
"->"		{ return (ZZFOLLOWS); }
";"		{ return (ZZSEMICOLON); }
{identifier}	{ opsiglval.tokenchar = strdup(yytext); return (ZZIDENT); }
{math}       	{ opsiglval.tokenchar = strdup(yytext); return (ZZSYMBOL); }
{intindex}	{ opsiglval.tokenchar = strdup(yytext); return (ZZINTI); }
{emptyspace}	{}
.		{/*opsiglval.tokenchar = strdup(yytext);*/ return yytext[0];}

%%

