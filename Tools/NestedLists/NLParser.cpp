/*
December 2004, M. Spiekermann. A debug mode for the scanner and parser have been
introduced. The error message will now help much better to locate errors.

*/

using namespace std;

#include "LogMsg.h"
#include "NLParser.h"
#include "NLScanner.h"
#include <iomanip>


extern NestedList* nl;
extern CMsg cmsg;

NLParser::NLParser( NestedList* nestedList, istream* ip, ostream* op )
  : isp( ip ), osp( op ), nl( nestedList )
{
  listExpression = 0;
  lex = new NLScanner( nestedList, isp, osp );
}

NLParser::~NLParser()
{
  delete lex;
}

int
NLParser::yylex()
{
  if ( RTFlag::isActive("NLParser:Debug") ) {
    yydebug = 1;
  } else {
    yydebug = 0;
  }
  
  if ( RTFlag::isActive("NLScanner:Debug") ) {
    lex->SetDebug(1);
  } else {
    lex->SetDebug(0);
  }


  return (lex->yylex());
}

void
NLParser::yyerror( char* s )
{
  cmsg.error() << "Nested-List Parser: " << endl << "  " << s 
       << " processing character '" << lex->YYText() 
       << "' (= " 
       << setiosflags(ios::hex|ios::showbase) 
       << static_cast<unsigned short>( *(lex->YYText()) ) 
       << resetiosflags(ios::hex|ios::showbase) 
       << ") at line " << lex->lines <<" and col " << lex->cols << "!"
       << endl;
  cmsg.send();
}
