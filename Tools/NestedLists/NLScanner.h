/*
December 2004, M. Spiekermann. A debug mode, and variables for the position were 
introduced.

*/

#ifndef NL_SCANNER_H
#define NL_SCANNER_H

#include "NestedList.h"

#ifndef yyFlexLexer
#define yyFlexLexer nlFlexLexer
#include <FlexLexer.h>
#endif

class NLScanner: public yyFlexLexer
{
 public:         
  enum Error
  {
    invalidInclude,
    circularInclusion,
    nestingTooDeep,
    cantRead,
  };
                
  NLScanner( NestedList* nestedList, 
             std::istream* yyin = 0, std::ostream* yyout = 0 );

  int yylex();  // overruling yyFlexLexer's yylex()

  int lines;    // position in the input
  int cols;

  // toggle debug mode
  void SetDebug(const int value);

 private:
  // no Scanner copy-initialization
  NLScanner( NLScanner const &other ); 

  // no assignment either
  NLScanner &operator=( NLScanner const &other );   

  NestedList* nl;
};

#endif

