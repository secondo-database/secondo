#include "NLScanner.h"

NLScanner::NLScanner( NestedList* nestedList, istream* yyin = 0, ostream* yyout = 0 )
{
  switch_streams( yyin, yyout );
  nl = nestedList;
}
