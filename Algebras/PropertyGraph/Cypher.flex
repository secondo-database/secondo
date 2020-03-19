/*

\framebox{\huge{ Cyper.flex }}

----
This file is part of SECONDO.

Copyright (C) 2016, 
University in Hagen, 
Faculty of Mathematocs and Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

/*

This is an input file for the flex scanner generator.

Defines the tokens for the Cypher query language.
These token will be used in the grammar rules  in the Cypher.bison file.

*/

%{ /* -*- C++ -*- */
  #include <cerrno>
  #include <climits>
  #include <cstdlib>
  #include <string>
  #include "CypherLang.h"
  #include "CypherParser.h"

  #undef yywrap
  #define yywrap() 1

  // The location of the current token.
  static yy::location loc;
%}

%option outfile="CypherScanner.cpp" header-file="CypherScanner.h"
%option noyywrap nounput batch debug noinput

/*
1  Definition part

1.1 Some token defined by regular expressions.

*/

id             [a-zA-Z][a-zA-Z_0-9]*
int            [0-9]+
blank          [ \t]
stringliteraldquote  \"([^\\\"]|\\.)*\"
stringliteralsquote '([^\\\"]|\\.)*'

%{
  // Code run each time a pattern is matched.
  #define YY_USER_ACTION  loc.columns (yyleng);
%}
%%
%{
  // Code run each time yylex is called.
  loc.step ();
%}

 
 /*
 2  Transition rules

 Skip whitespace
 */
 


{blank}+   loc.step ();
[\n]+      loc.lines (yyleng); loc.step ();

  /*

  1.1 Terminal symbols for keywords and Operators.

  (The statements are creating token that will be uses by the parser)

  */


"("        return yy::cypher_parser::make_LNPAREN(loc);
")"        return yy::cypher_parser::make_RNPAREN(loc);
"["        return yy::cypher_parser::make_LEPAREN(loc);
"]"        return yy::cypher_parser::make_REPAREN(loc);
"{"        return yy::cypher_parser::make_LPPAREN(loc);
"}"        return yy::cypher_parser::make_RPPAREN(loc);
"match"    return yy::cypher_parser::make_MATCH(loc);
"MATCH"    return yy::cypher_parser::make_MATCH(loc);
"where"    return yy::cypher_parser::make_WHERE(loc);
"WHERE"    return yy::cypher_parser::make_WHERE(loc);
"starts"   return yy::cypher_parser::make_STARTS(loc);
"with"     return yy::cypher_parser::make_WITH(loc);
"contains" return yy::cypher_parser::make_CONTAINS(loc);
"return"   return yy::cypher_parser::make_RETURN(loc);
"RETURN"   return yy::cypher_parser::make_RETURN(loc);
"and"      return yy::cypher_parser::make_AND(loc);
"as"       return yy::cypher_parser::make_AS(loc);
"AS"       return yy::cypher_parser::make_AS(loc);
","        return yy::cypher_parser::make_COMMA(loc);
"."        return yy::cypher_parser::make_DOT(loc);
"="        return yy::cypher_parser::make_OPEQUAL(loc);
":"        return yy::cypher_parser::make_COLON(loc);
"-"        return yy::cypher_parser::make_DASH(loc);
"<>"       return yy::cypher_parser::make_OPNOTEQUALTHAN(loc);
"<"        return yy::cypher_parser::make_OPLESSERTHAN(loc);
">"        return yy::cypher_parser::make_OPBIGGERTHAN(loc);
"<-"       return yy::cypher_parser::make_LEFTDASH(loc);
"->"       return yy::cypher_parser::make_RIGHTDASH(loc);


 /*
  
  2.2 String literals 
  
  strip out the string value from literal constant

 */

{stringliteraldquote} {
  std::string temp(yytext);
  temp=temp.substr(1,temp.length()-2); // = new std::string(yytext+1, yyleng-2);
  return yy::cypher_parser::make_STRINGLITERAL(temp, loc);
}
{stringliteralsquote} {
  std::string temp(yytext);
  temp=temp.substr(1,temp.length()-2); // = new std::string(yytext+1, yyleng-2);
  return yy::cypher_parser::make_STRINGLITERAL(temp, loc);
}

 /*

  2.3 Integer values

 */


{int}      {
   errno = 0;
   long n = strtol (yytext, NULL, 10);

   if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
     driver.error (loc, "integer is out of range");
   }
    
   return yy::cypher_parser::make_NUMBER(n, loc);
}

 /*

   2.4 Identifiers 

   Used i.e. for Type or Property names

 */


{id}  {
     return yy::cypher_parser::make_IDENTIFIER(yytext, loc);
}

.          driver.error (loc, "invalid character");
<<EOF>>    return yy::cypher_parser::make_END(loc);
%%



void CypherLanguage::scan_begin () {
   yy_flex_debug = trace_scanning;
   yy_scan_string (text.c_str ());
}

void CypherLanguage::scan_end () {
   yy_delete_buffer(YY_CURRENT_BUFFER);
}
