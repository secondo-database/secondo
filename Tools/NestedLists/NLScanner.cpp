/*

Dec 2004, M. Spiekermann. A debug mode and the calculation of the 
input position was introduced.

*/
using namespace std;

#include "NLScanner.h"

NLScanner::NLScanner( NestedList* nestedList, std::istream* yyin, std::ostream* yyout )
{
  switch_streams( yyin, yyout );
  nl = nestedList;
  lines = 1;
  cols = 0;
}

void
NLScanner::SetDebug(const int value) {
  yy_flex_debug = value;
}
