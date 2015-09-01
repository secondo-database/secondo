package mmdb.streamprocessing.parser;

import java_cup10.runtime.Symbol;

%%
%implements java_cup10.runtime.Scanner
%function next_token
%type java_cup10.runtime.Symbol

SIGN		=  [+-]
LETTER		=  [A-Za-z]
DIGIT		=  [0-9]
DIGITS		=  [0-9]+
SYMCHAR		=  [^A-Za-z0-9 \t\n()\"]
EMPTYSPACE	=  [ \t\n]+
STRING		=  [^\"]*
%%

"("					{ return new Symbol(sym.OPEN); }
")"					{ return new Symbol(sym.CLOSE); }
{EMPTYSPACE}		{ /* ignore */ }
({SIGN})?{DIGITS}	{ return new Symbol(sym.INT, new Integer(yytext())); }
({SIGN})?{DIGITS}"."({DIGITS})?(E({SIGN})?{DIGITS})?
					{ return new Symbol(sym.REAL, new Float(yytext())); }
TRUE				{ return new Symbol(sym.BOOL, Boolean.TRUE); }
FALSE				{ return new Symbol(sym.BOOL, Boolean.FALSE); }
\"{STRING}\"		{ return new Symbol(sym.STRING, yytext()); }
({LETTER}({LETTER}|{DIGIT}|"_")*)|{SYMCHAR}+
					{ return new Symbol(sym.SYMBOL, yytext()); }
. 					{ System.err.println("Illegal character: \""+yytext()+"\""); }