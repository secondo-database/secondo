/*
August 8, 2000 RHG Added print command to show result of text to list translation.

March 2002 Ulrich Telle Port to C++

*/

using namespace std;

#include <iostream>
#include <sstream>
#include <string>
#include "NestedText.h"
#include "SecParser.h"

#ifndef yyFlexLexer
#define yyFlexLexer xxFlexLexer
#include <FlexLexer.h>
#endif

extern string* yacc_outtext;
extern char*   yacc_error;
extern int xxparse();

static xxFlexLexer* lexPtr;

int xxlex()
{
  return (lexPtr->yylex());
}

SecParser::SecParser()
{
}

SecParser::~SecParser()
{
}

int
SecParser::Text2List( const string& inputString, string& outputString, string& errors)
{
  istringstream is( inputString );
  xxFlexLexer lex( &is );
  lexPtr = &lex;
  int error;
  outputString = "";
  yacc_outtext = &outputString;
  NestedText::ReleaseStorage();
  error = xxparse();
  if ( error != 0 )
  {
    errors = yacc_error;
  }
  else
  {
    errors = "";
  }
  cerr << outputString;

  return (error);
}

