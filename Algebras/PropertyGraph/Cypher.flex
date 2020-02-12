%{ /* -*- C++ -*- */
  #include <cerrno>
  #include <climits>
  #include <cstdlib>
  #include <string>
  #include "CypherLang.h"
  #include "CypherParser.h"

  // Work around an incompatibility in flex (at least versions
  // 2.5.31 through 2.5.33): it generates code that does
  // not conform to C89.  See Debian bug 333231
  // <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.
  #undef yywrap
  #define yywrap() 1

  // The location of the current token.
  static yy::location loc;
%}

%option outfile="CypherScanner.cpp" header-file="CypherScanner.h"
%option noyywrap nounput batch debug noinput

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
{blank}+   loc.step ();
[\n]+      loc.lines (yyleng); loc.step ();

{stringliteraldquote} {
  std::string temp(yytext);
  temp=temp.substr(1,temp.length()-2); // = new std::string(yytext + 1, yyleng - 2);
  return yy::cypher_parser::make_STRINGLITERAL(temp, loc);
}
{stringliteralsquote} {
  std::string temp(yytext);
  temp=temp.substr(1,temp.length()-2); // = new std::string(yytext + 1, yyleng - 2);
  return yy::cypher_parser::make_STRINGLITERAL(temp, loc);
}

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
"<"       return yy::cypher_parser::make_OPLESSERTHAN(loc);
">"       return yy::cypher_parser::make_OPBIGGERTHAN(loc);
"<-"       return yy::cypher_parser::make_LEFTDASH(loc);
"->"       return yy::cypher_parser::make_RIGHTDASH(loc);



{int}      {
   errno = 0;
   long n = strtol (yytext, NULL, 10);

   if (! (INT_MIN <= n && n <= INT_MAX && errno != ERANGE)) {
     driver.error (loc, "integer is out of range");
   }
    
   return yy::cypher_parser::make_NUMBER(n, loc);
}

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
