#include <iostream>
#include <sstream>
#include "NestedText.h"

#include <stdio.h>
#include <string.h>

#ifndef yyFlexLexer
#define yyFlexLexer xxFlexLexer
#include <FlexLexer.h>
#endif

int xxlval;

int main()
{
  string str = "query Staedte feed filter[.Bev > 100000] consume";
  istringstream is( str );
  xxFlexLexer lex( &is );
  int token;
  xxlval = 0;

  cout << "lex input=" << str << endl;
  token = lex.yylex();
  while (token != 0)
  {
    cout << "Token " << token << " lval=" << xxlval << " "; 
    if (xxlval != 0)
    {
      NestedText::Print( xxlval );
      xxlval = 0;
    }
    cout << endl;
    token = lex.yylex();
  }
}

